# Class: Page::Setup::Login
# User login
package Page::Setup::Login;
use strict;
use warnings;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Carp;
use Page::Abstract;
use Sql::Config;
our @ISA = qw(Page::Abstract);

##
# Init.
sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};
	$self->{'sqlConfig'} = Sql::Config->new($dbh);
	return $self;
}


sub show_login {
	my ($self, $vars) = @_;
	return ($vars, "setup_login.html");
}

sub process_login {
	my $self = shift;
	my $sqlConfig = $self->{'sqlConfig'};
	
	## MEns vi tester:
		return (1, "FIRST_LOGIN");


	# check if valid, return 0 if not.

	# if valid, check if first time.
	unless ($sqlConfig->config_exists) {
		return (1, "FIRST_LOGIN");
	}
	else {
		return 1;
	}
		

	if ($sqlConfig->config_exists) {
		
		return 1;
	}
	
	#Run wizard.
	0;
}

1;
