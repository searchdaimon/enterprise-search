##
# Methods related to user performing network scan.
package Page::Scan::Scanning;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Page::Abstract;
use Sql::ScanResults;
use Sql::Connectors;
use Sql::CollectionAuth;

BEGIN {
	push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use Boitho::Scan;
use config qw(%CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_NEW_SCAN  => "scan_new.html";
use constant TPL_SCAN_HEAD => "scan_top.html";
use constant TPL_SCAN_FOOTER => "scan_bottom.html";

my $sqlConnectors;
my $sqlAuth;
my $sqlResults;
my $bScan;

sub _init {
	my $self = shift;
	$sqlConnectors = Sql::Connectors->new($self->{'dbh'});
	$sqlAuth       = Sql::CollectionAuth->new($self->{'dbh'});
	$sqlResults	   = Sql::ScanResults->new($self->{dbh});
	$bScan		   = Boitho::Scan->new($CONFIG{infoquery}, $CONFIG{genip_path});
	1;
}

##
# Show scan form
sub show {
	my ($self, $vars) = @_;

	my @authentication = $sqlAuth->get_all_auth();
	
	$vars->{'connectors'}     = $sqlConnectors->get_connectors_with_scantool();
	$vars->{'authentication'} = [$sqlAuth->get_all_auth()];
	
	return ($vars, TPL_NEW_SCAN);
}


sub run_scan {
	my ($self, $vars, $tpl_sub, $scan_ref) = @_;

        $scan_ref->{range} =~ s/^\s+|\s+$//g;
	
	# Hooks are for printing scan data as it arrives.
	$self->_add_hooks(); 

	my %auth = $self->_get_scan_auth(%{$scan_ref});

	# add scan to db
	my $result_id
            = $sqlResults->insert_new_results(
                    $scan_ref->{'connector'},
                    $scan_ref->{'range'},
                    $auth{id},
                    );


	# do (and show) the scan
	my $old_buffer_val = $|;
	$| = 1; # don't buffer stdout
        &$tpl_sub(TPL_SCAN_HEAD);
	#$template->process(TPL_SCAN_HEAD);

	my $connector = $sqlConnectors->get_name($scan_ref->{connector});
        my $xml_result;
        eval {
	    $xml_result = $bScan->scan(
				$connector,
				$scan_ref->{range},
				$auth{user},
				$auth{pass},
				$scan_ref->{use_ping_scan});
        };
        if ($@) {
            $vars->{error} = ($@ =~ /Genip exited/)
                ? "Range '$scan_ref->{range}' is invalid." 
                : "Error: $@";
            $sqlResults->delete_result($result_id);
        }
        else {
            $vars->{result_id} = $result_id;
            my $data = {
		'result_xml' => $xml_result,
		'connector' => $scan_ref->{'connector'},
		'done' => 1 };
            $sqlResults->update_results($result_id, $data);
        }
        &$tpl_sub(TPL_SCAN_FOOTER, $vars);
	$| = $old_buffer_val;

	# update db with results
        1;
}

sub _add_hooks {
	my $self = shift;

	my $start_ptr = sub {
		my ($connector, $host) = @_;
		print "Scanning $host for $connector\n";
	};
	my $done_ptr  = sub {
		my ($connector, $host, $result, $msg) = @_;
		if ($result) {
			print "Finished scanning $host\n";
		}
		else {
			print "Error when scanning $host, $msg\n";
		}
	};
	my $share_found_ptr = sub {
		my ($connector, $host, $share) = @_;
		print "Found share $share on $host, type $connector.\n";
	};

	$bScan->add_hook_scan_start($start_ptr);
	$bScan->add_hook_scan_done($done_ptr);
	$bScan->add_hook_share_found($share_found_ptr);
	1;
}

##
# Get user/pass from form.
# 
# User may have chosen auth from list. If so, we'll have to 
# get user pass from db.
sub _get_scan_auth {
    my ($self, %scan) = @_;

    my %auth = (
            id => $scan{auth_id},
            user => $scan{username},
            pass => $scan{password},
            );

    if ($auth{id} =~ /^\d+$/) {
        ($auth{user}, $auth{pass}) = $sqlAuth->get_pair_by_id($auth{id});
        return %auth;
    }

    $auth{id} = $sqlAuth->add($auth{user}, $auth{pass});
    return %auth;
}

1;
