package Common::Scan;
use strict;
use warnings;
use Carp;
BEGIN {
	push @INC, "Modules";
}
use Common::Generic;
use File::stat;
use Data::Dumper;
use Sql::ScanResults;
use Boitho::Scan;
use Sql::CollectionAuth;
use Sql::Connectors;
use config qw($CONFIG);

## Class for processing scans in database.

sub new {
	my $class = shift;
	my $self = {};
	my $dbh = shift;
	bless $self, $class;
	$self->_initialize($dbh);
	return $self;
}

sub _initialize($$) {
	my $self = shift;
	my $dbh = shift;
	$self->{'dbh'} = $dbh;
	$self->{'sqlResults'} = Sql::ScanResults->new($dbh);
	$self->{'scan'} = Boitho::Scan->new($CONFIG->{'infoquery'});
	$self->{'sqlAuth'} = Sql::CollectionAuth->new($dbh);
	$self->{'sqlConnectors'} = Sql::Connectors->new($dbh);
	$self->{'common'} = Common::Generic->new();
}

# Accepts either connector name, or ID as a first parameter.
# Checks if it supports scanning.
sub is_supported_connector($$) {
	my ($self, $connector) = @_;
	if ($connector =~ /\d+/) {
		my $sql = $self->{'sqlConnectors'};
		$connector = $sql->get_name($connector);
	}
	return $self->{'scan'}->is_supported_connector($connector);
}

sub scan_start {
	my $self = shift;
	my $scan_param_ptr = shift;
	my %scan_param = %$scan_param_ptr;
	
	my $scan = $self->{'scan'};
	my $sqlAuth = $self->{'sqlAuth'};
	my $sqlResults = $self->{'sqlResults'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $dbh = $self->{'dbh'};
	my $common = $self->{'common'};
	
	if (!$scan_param{'connector'}) {
		croak "Can't perform scan without knowing for what connector.";
	}
	
	# If we got connector id, rather than name
	# we'll have to grab the name.
	if ($scan_param{'connector'} =~ /\d+/) {
		$scan_param{'connector'} =
			$sqlConnectors->get_name($scan_param{'connector'});
	}

	## Add this unfinished scan to the database
	my $result_id = $sqlResults->insert_new_results(
				$scan_param{'connector'},
				$scan_param{'range'});

	
	
	my $auth_id = $common->get_auth_id(
				$dbh,
				$scan_param{'auth_id'},
				$scan_param{'username'},
				$scan_param{'password'});

	my ($user, $pass) = $sqlAuth->get_pair_by_id($auth_id);
	
	## Perform the scan
	my $result = $scan->scan(
		$scan_param{'connector'},
		$scan_param{'range'},
		$user,
		$pass,
	);

	## Update the complete scan
	my $data = {
		'result_xml' => $scan->get_xml,
		'connector' => $scan_param{'connector'},
		'done' => 1 };
	$sqlResults->update_results($result_id, $data);
	

	return $result_id;
}

sub _insert_result($\@) {
	my $self = shift;
	my $time = $self->{'scan'}->get_started;
	my $data = {
		'result_xml' => shift,
		'connector' => shift,
		'range' => shift,
		'done' => 1,
		'time' => $time,
	};
	my $sqlResults = $self->{'sqlResults'};

	$sqlResults->insert_result($data);
}


## Given a result id, fetch xml from database,
## parse it and return the scan results.
sub process($$) {
	my $self = shift;
	my $id = shift;
	
	my $scan = $self->{'scan'};
	my $sqlResults = $self->{'sqlResults'};

	return unless $sqlResults->exists($id);

	my $xml = $sqlResults->get_xml($id);
	my $result = $scan->parse_xml($xml);
	my $connector = $sqlResults->get_connector($id);

	return ($connector, $result);
}



1;
