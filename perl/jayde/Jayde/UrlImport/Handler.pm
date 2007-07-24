##
# SAX Handler class for parsing Jayde user url xml dump.
# based on Jayde::UserImport::Handler.
# dvj, 2007
package Jayde::UrlImport::Handler;
use strict;
use warnings;
use Readonly;
use Switch;
use Data::Dumper;

# If jayde export has strings in quotation,
# this flag will have them removed.
Readonly::Scalar my $STRIP_QUOTATION => 1;

# states
my $parent_elm;
my $current_elm;
my $url_count = 0;
my $document_name;

# hooks
my $record_hook;

my %crnt_record;


##
# Default constructor
#
# Arguments:
#   document - Document name (used for log purpose)
#   record_hok - Function called when a record is read.
sub new { 
    my ($class, $document, $my_record_hook) = @_;
    $document_name = $document;

    $record_hook = $my_record_hook;
    return bless {}, $class;
}

sub start_element {
    my ($self, $element) = @_;
    
    if (defined $current_elm) {
        # If previous element is still defined, then
        # it hasn't been closed yet. Thus this element is it's child.
        $parent_elm = $current_elm;
    }

    $current_elm = $element;
    1;
}    


sub characters {
    my ($self, $characters) = @_;

    # SAX calls this method on every character data, not just
    # nested inside elements.
    return unless defined $current_elm;

    my $elm_name = $current_elm->{Name};
    my $chr_data = $characters->{Data};

    if ($STRIP_QUOTATION) {
        $chr_data =~ s/^"//;
        $chr_data =~ s/"$//;
    }
    
    $crnt_record{$elm_name} = $chr_data;

    1;
}

sub end_element {
    my ($self, $element) = @_;

    if ($element->{Name} eq 'UserUrl') {
        &{$record_hook}($document_name, %crnt_record);
        %crnt_record = ();
        ++$url_count;
    }

    $current_elm = undef;
    1;
}

sub start_document {
    my $self = shift;
    1;
}

sub end_document {
    my $self = shift;
    1;
}

1;
