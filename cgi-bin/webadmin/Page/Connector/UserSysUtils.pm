package Page::Connector::UserSysUtils;
use strict;
use warnings;

use Params::Validate;
use Carp;
use Data::Dumper;
use Sql::SystemMapping;
use config qw(%CONFIG);

sub new { bless { } }

sub conn_path {
	validate_pos(@_, 1, { regex => qr{^\d+$} });
	my ($s, $id) = @_;
	
	my $conn_dir = $CONFIG{sysconn_base_dir} . "/perl_id_$id";
	return wantarray ? (
		$conn_dir,
		$conn_dir . "/main.pm",
		$conn_dir . "/id"
	) : $conn_dir;
}

sub init_test_sys {
	validate_pos(@_, 1, 1, { regex => qr/^\d+$/ });
	my ($s, $crawler, $conn_id) = @_;
	
	my $sys_name = sprintf $CONFIG{test_usersys_name}, $conn_id;

	my $sys_ref = $crawler->{sql_sys}->get({ name => $sys_name });
	if ($sys_ref) {
		return Data::UserSys->new($crawler->{dbh}, $sys_ref->{id});
	}

	my $sys = Data::UserSys->create_novalidate($crawler->{dbh}, (
		name => $sys_name,
		connector => $conn_id
	));
	return $sys;

}

sub del_test_sys {
	validate_pos(@_, 1, 1, 1);
	my ($s, $testsys, $crawler) = @_;
	my $id = $testsys->get('id');

	$testsys->del();

	# Delete mapping
	my $sql_mapping = Sql::SystemMapping->new($crawler->{dbh});
	$sql_mapping->delete({ system => $id });
	$s;
}



1;
