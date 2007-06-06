##
# Displaying and processing information from a scan.
package Page::Scan::Process;
use strict;
use warnings;

use Carp;
use Data::Dumper;

BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'};
}

use Boitho::Scan;
use Page::Abstract;
use config qw(%CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_SCAN_RESULTS => "scan_results.html";


# private vars
my $sqlResults;
my $bScan;


sub _init {
	my $self = shift;	
	
	$sqlResults = Sql::ScanResults->new($self->{dbh});
	$bScan      = Boitho::Scan->new($CONFIG{infoquery}, $CONFIG{genip_path});
	
	
	1;
}

##
# Form to process given result
sub show {
	my ($self, $vars, $scan_id, $scan_added) = @_;

	my ($connector, $results) = $self->fetch_scan($scan_id);
	if ($results and $connector) {
 			$vars->{'results'} = $results;
 			$vars->{'connector'} = $connector;
 			$vars->{'id'} = $scan_id;
	}

	
	$vars->{'succ_scan_added'} = $scan_added; # Share added by clicking "Add as collection".

	return ($vars, TPL_SCAN_RESULTS);
}

##
# Fetch and parse results in database.
#
# Arguments:
#	id - scan result id
#
# Returns:
#	connector - What connector result is for. 
#	result - Scan results.
sub fetch_scan {
	my ($self, $scan_id) = @_;

	return unless $sqlResults->exists($scan_id);

	my $xml = $sqlResults->get_xml($scan_id);
	my $result = $bScan->parse_xml($xml);
	my $connector = $sqlResults->get_connector($scan_id);
	return ($connector, $result);
}

##
# Delete scan results
sub delete_result {
	my ($self, $vars, $scan_id) = @_;
	$sqlResults->delete_result($scan_id);
	$vars->{'success_delete_result'} = 1;
	return $vars;
}


1;
