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
	my ($self, $vars, $template, $scan_ref) = @_;
	
	# Hooks are for printing scan data as it arrives.
	$self->_add_hooks(); 

	# add scan to db
	my $result_id
			 = $sqlResults->insert_new_results(
				$scan_ref->{'connector'},
				$scan_ref->{'range'});


	# do (and show) the scan
	my $old_buffer_val = $|;
	$| = 1; # don't buffer stdout
	$template->process(TPL_SCAN_HEAD);

	my $connector = $sqlConnectors->get_name($scan_ref->{connector});
	my %auth = $self->_get_scan_auth($scan_ref);
	my $xml_result = $bScan->scan(
				$connector,
				$scan_ref->{range},
				$auth{user},
				$auth{pass},
				$scan_ref->{use_ping_scan});
	
	$vars->{result_id} = $result_id;
	$template->process(TPL_SCAN_FOOTER, $vars);
	$| = $old_buffer_val;

	# update db with results
	my $data = {
		'result_xml' => $xml_result,
		'connector' => $scan_ref->{'connector'},
		'done' => 1 };
	$sqlResults->update_results($result_id, $data);

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
	my ($self, $scan_ref) = @_;

	unless ($scan_ref->{auth_id} =~ /^\d+$/) {
		return (user => $scan_ref->{username},
				pass => $scan_ref->{password});
	}

	my ($user, $pass) = $sqlAuth->get_pair_by_id($scan_ref->{auth_id});
	return (user => $user,
			pass => $pass);
}

1;
