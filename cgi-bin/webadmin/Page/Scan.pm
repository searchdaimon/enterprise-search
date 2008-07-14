package Page::Scan;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Sql::ScanResults;

use Page::Abstract;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

use constant TPL_SCAN_OVERVIEW => "scan_main.html";
my $sqlResults;

sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};
	$sqlResults = Sql::ScanResults->new($dbh);
}

sub show { show_overview(@_) }

sub show_overview {
	my ($self, $vars) = @_;
	my $results_ref = $sqlResults->get_list_and_connector_name();
	
	my @results = sort by_scan_date @{$results_ref};
	$vars->{results} = \@results;
	return TPL_SCAN_OVERVIEW;
}

sub by_scan_date {
	$b->{time} cmp $a->{time};
}
1;
