#Class: Common::FormFlow
#Simplify long wizards.
package Common::FormFlow;
use strict;
use warnings;
use Carp;
use constant FLOW_START_FORM => 'FLOW_START_FORM';
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(FLOW_START_FORM);

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
        croak "No form id provided"
            unless defined $form;
	
	my @flow = @{$self->{'flow'}};

	my $process_method;
        my $i = 0;
        foreach my $flow_ref (@flow) {
            if ($process_method 
                    = $self->_find_method($form, $flow_ref)) {
                $self->{next_form} = $flow[$i + 1]->[0];
                last;
            }
            $i++;
        }

	croak qq{FormFlow: Form "$form" not found}
            unless $process_method;

	return &$process_method();
}

##
# Process next form in list. 
# Useful when user skips a form.
sub process_next {
    my $s = shift;
    return $s->process($s->{next_form});
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
