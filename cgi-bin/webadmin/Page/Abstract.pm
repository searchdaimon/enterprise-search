# Class: Page::Abstract
# Function for Page classes. Should be inhereted, not used directly.
package Page::Abstract;
use strict;
use warnings;
use Carp;

# Constructor: new
#
# Attributes:
#	dbh - Database handler
#	@ - Anything, sent to the childclass' _init function.
sub new {
    my $class = shift;
    my $dbh = shift;
    
    my $self = {};
    bless $self, $class;

    $self->{'dbh'} = $dbh;
    if ($self->can("_init")) { 
		# initialize if supported
		$self->_init(@_);
	}
	return $self;
}

1;
