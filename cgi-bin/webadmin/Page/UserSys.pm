package Page::UserSys;
use strict;
use warnings;

BEGIN { unshift @INC, "$ENV{BOITHOHOME}/Modules" }
use Page::Abstract;
use Sql::System;
use Sql::SystemMapping;
use Boitho::Infoquery;
use Params::Validate qw(validate_pos);
use Carp;
our @ISA = qw(Page::Abstract);

use config qw(%CONFIG);

sub _init {
	my $s = shift;
	$s->{sql_sys} = Sql::System->new($s->{dbh});
	$s->{sql_mapping} = Sql::SystemMapping->new($s->{dbh});
}

sub list_users {
	validate_pos(@_, 1, { regex => qr(^\d+$) });
	my ($s, $system_id) = @_;
	
	# DEBUG:
	if ($system_id == 1) {
		open my $fh, "/home/dagurval/websearch/example_list" or die $1;
		my @users = <$fh>;
		chomp for @users;
		my %users = map { $_ => 1 } grep { $_ } @users;
		return keys %users;
	}
	else {
		open my $fh, "/home/dagurval/websearch/example_list" or die $1;
		my @users = <$fh>;
		chomp for @users;
		my %users = map { $_ => 1 } grep { $_ } @users;
		return keys %users;

	}
	#my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
	#return @{$iq->listUsers($system_id)}
}

1;
