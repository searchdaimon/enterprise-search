# Class: Boitho::InitServices
# Class to manage boitho services.
package Boitho::InitServices;
use strict;
use warnings;
use Carp;
use constant INIT_DIR => "/etc/init.d/";
use constant WRAPPER_PATH => $ENV{'BOITHOHOME'} . "/Modules/Boitho/InitServices/initwrapper";

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    return $self;
}


##
# Start a service. Runs though suid-wrapper.
#
# Attribute:
#   service - name of service
sub start {
    my ($self, $service) = @_;
    $self->_validate_service($service);
    my ($status, $message) = $self->_exec_service_suid($service, "start");
    return ($status, $message);
}


##
# Stop a service. Runs though suid-wrapper.
#
# Attribute:
#   service - name of service
sub stop {
    my ($self, $service) = @_;
    $self->_validate_service($service);
    
    my ($status, $message) = $self->_exec_service_suid($service, "stop");
    return ($status, $message);
}


##
# Restart a service. Runs though suid-wrapper.
#
# Attribute:
#   service - name of service
sub restart {
    my ($self, $service) = @_;
    $self->_validate_service($service);

    my ($status, $message) = $self->_exec_service_suid($service, "restart");
    return ($status, $message);
}

##
 # Get status of a service. Runs as the same user as the script is executed in.
 #
 # Attributes:
 #	service - name of the service
 #
 # Returns:
 #	(status, message) - (status, message)
sub status {
    my ($self, $service) = @_;
       
    $self->_validate_service($service);

    my ($status, $message) = $self->_exec_service_suid($service, "status");
    
    return ($status, $message);
}

# Group: Private methods

##
 # Execute service with wrapper
 #
 # Attributes:
 #	service   - Service name
 #	parameter - Parameter to service. WARN: It won't be escaped.
sub _exec_service_suid {
    my ($self, $service, $parameter) = @_;
    my $exec = WRAPPER_PATH . " $service $parameter|";

    open my $wraph, $exec
	or croak "Unable to execute $service with wrapper, $?";

    my @output = <$wraph>;
    my $status = 1;

    close $wraph or $status = 0;
    return ($status, join('\n', @output));
}
##
 # Croak if service doesn't exist.
 #
 # Attributes:
 #	service - name of service
sub _validate_service {
    my ($self, $service) = @_;
    
    croak "Service not provided"
	unless $service;

    my $path = INIT_DIR . $service;
    croak "$path does not exist"
	unless -e $path;

    1;
}

1;
