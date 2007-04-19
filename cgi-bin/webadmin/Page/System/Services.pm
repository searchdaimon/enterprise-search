# Class: Page::System::Services
# Manage init services.
package Page::System::Services;
use strict;
use warnings;
use Carp;
use Data::Dumper;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::InitServices;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

sub _init {
	my ($self) = @_;
	$self->{'services'} = $CONFIG->{'init_services'};
	$self->{'init'}     = Boitho::InitServices->new($CONFIG->{'init_dir'}, $CONFIG->{'init_wrapper_path'});
	return $self;
}

##
# Display list of services, and their status.
sub show {
	my ($self, $vars) = @_;
	my $template_file = "system_services.html";

	my $services_ref = $self->{'services'};
	my $init = $self->{'init'};

	my @service_stats;

	foreach my $service (@$services_ref) {
		my ($status, $message) = $init->status($service);

		push @service_stats, {
			'name' => $service,
			'status'  => $status,
			'message' => $message,
		}
	}	

	$vars->{'service_stats'} = \@service_stats;

	return ($vars, $template_file);
}

##
# Run start/stop/restart on a service.
#
# - Adds 'succ_msg_serv' variable to vars on success.
# - Adds 'succ_msg_error' variable to vars on failure.
sub action {
	my ($self, $vars, $service, $param) = @_;
	my $init = $self->{'init'};	

	# check
	croak "Unknown parameter: $param"
		unless $self->_is_valid_param($param);

	croak "Unknown service: $service"
		unless $self->_is_valid_service($service);
	
	
	my ($success, $message) = $init->$param($service); #ex: $init->start("crawlManager")

	if ($success) {
		$vars->{'succ_msg_serv'} = $message;
	}
	else {
		$vars->{'error_msg_serv'} = $message;
	}

	return $self->show($vars);
}

sub _is_valid_param {
	my ($self, $param) = @_;

	my @valid_params = ("start", "stop", "restart", "status");	
	
	(grep { /^$param$/ } @valid_params) ? 1 : 0;
}

sub _is_valid_service {
	my ($self, $service) = @_;

	my @valid_services = @{$self->{'services'}};

	(grep { /^$service$/ } @valid_services) ? 1 : 0;
}

1;