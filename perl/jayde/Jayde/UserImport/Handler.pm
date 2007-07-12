##
# SAX Handler class for parsing Jayde userdb xml dump.
# dvj, 2007
package Jayde::UserImport::Handler;
use strict;
use warnings;
use Readonly;
use Switch;
use Data::Dumper;

Readonly::Scalar my $LOG_DEBUG => "debug";
Readonly::Scalar my $LOG_INFO  => "info";
Readonly::Scalar my $LOG_WARN  => "warn";

# If jayde export has strings in quotation,
# this flag will have them removed.
Readonly::Scalar my $STRIP_QUOTATION => 1;

# states
my $parent_elm;
my $current_elm;
my $user_count = 0;
my $document_name;

# hooks
my $log;
my $record_hook;

my %crnt_record;


##
# Default constructor
#
# Arguments:
#   document - Document name (used for log purpose)
#   record_hok - Function called when a record is read.
#   log_hook - Function for logging (optional)
sub new { 
    my ($class, $document, $my_record_hook, $log_hook) = @_;
    $document_name = $document;

    $log = $log_hook;
    $record_hook = $my_record_hook;
    return bless {}, $class;
}

sub start_element {
    my ($self, $element) = @_;
    #&{$log}($LOG_DEBUG, "Start element ", $element->{Name}) if $log;
    
    if ($element->{Name} eq 'UserAccount') {
        # read attribute id.
        $crnt_record{id} = $element->{Attributes}{id};
    }

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

    #&{$log}($LOG_DEBUG, $elm_name, "\t,", $chr_data, ",");

    if ($STRIP_QUOTATION) {
        $chr_data =~ s/^"//;
        $chr_data =~ s/"$//;
    }
    
    if ($elm_name eq "array-element") {
            my $p_name = $parent_elm->{Name};

            if (defined $crnt_record{$p_name} 
                    and ref $crnt_record{$p_name} ne 'ARRAY') {
                # set parent to array, in case it 
                # contained bogus character data.
                $crnt_record{$p_name} = [];
            }
            push @{ $crnt_record{$p_name} }, $chr_data;
    }
    else {
        $crnt_record{$elm_name} = $chr_data;
    }

    1;
}

sub end_element {
    my ($self, $element) = @_;

    if ($element->{Name} eq 'UserAccount') {
        &{$log}($LOG_DEBUG, "Done parsing a record") if $log;
        &{$record_hook}($document_name, %crnt_record);
        %crnt_record = ();
        ++$user_count;
    }

    $current_elm = undef;
    1;
}

sub start_document {
    my $self = shift;
    &{$log}($LOG_INFO, "Now parsing $document_name") if $log;
    1;
}

sub end_document {
    my $self = shift;
    &{$log}($LOG_INFO, "Done parsing $document_name.",
                    " Read $user_count users.") if $log;
    1;
}

1;
