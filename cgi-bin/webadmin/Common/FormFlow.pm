#Class: Common::FormFlow
#Simplify long wizards.
package Common::FormFlow;
use strict;
use warnings;
use Carp;

# Constructor: new
sub new {
	my ($class, $debug) = @_;
	my $self = {};
	bless $self, $class;

	# init
	$self->{'flow'} = [];
	$self->{'debug'} = $debug;
	return $self;
}

##
# Add new form
#
# Attributes:
#	form - Form identifier
#	method_ptr - Pointer to method to process form.
#	debug_msg - Optional message to print when form is found.
sub add {
	my ($self, $form, $method_ptr, $debug_msg) = @_;
	
	croak "Form or method_ptr not defined"
		if grep { !defined } $form, $method_ptr;

	push @{$self->{'flow'}}, [$form, $method_ptr, $debug_msg];
	return $self;
}


##
# Runs method belonging to form. Croaks if form is not found.
#
# Attributes:
#	form - Form identifier
sub process {
	my ($self, $form) = @_;
	my @flow = @{$self->{'flow'}};
	croak "No form id provided"
		unless defined $form;

	my $process_method;
	foreach my $flow_ref (@flow) {
		last if ($process_method 
					= $self->_find_method($form, $flow_ref));
	}

	croak "FormFlow: Form \"$form\" not found"
		unless $process_method;

	return &$process_method();
}

##
# Helper method to find pointer to form.
#
# Attributes:
#	looking_for - The form we're looking for.
#	method_ref - [formid, method_ref]
#
# Returns:
#	- method_ref if found
#	- undef if not.
sub _find_method {
	my ($self, $looking_for, $flow_ref) = @_;
	my ($found, $method, $debug) = @$flow_ref;

	if ($found eq $looking_for) {;
		if ($self->{'debug'} and defined $debug) {
			warn $debug;
		}
		return $method;
	}
	
	return;
}

1;