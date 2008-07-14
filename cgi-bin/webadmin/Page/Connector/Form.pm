package Page::Connector::Form;
use strict;
use warnings;

use Carp;
use Readonly;
use File::Path qw(rmtree mkpath);
use Data::Dumper;
use Params::Validate qw(validate_pos OBJECT);

use Page::Connector;
our @ISA = qw(Page::Connector);

use config qw(%CONFIG);


Readonly::Array my @CHRS 
    => ('a'..'z', 'A'..'Z');

Readonly::Scalar my $TPL_LIST => 'conn_list.html';
Readonly::Scalar my $TPL_EDIT => 'conn_edit.html';
Readonly::Scalar my $TPL_DEL  => 'conn_del.html';

##
# Show list of connectors
sub show_list {
    my ($s, $tpl_vars) = @_;
    $tpl_vars->{connectors} = 
        [ $s->{sql_conn}->get({extension => 1}, '*', 'name') ];
    
    $TPL_LIST;
}

##
# Confirm del dialog.
sub show_delete {
    my ($s, $tpl_vars, $id) = @_;
    $s->{sql_conn}->is_extension($id)
        or croak "'$id' is not a connector extension";

    
    # check if has collections
    $s->del_test_collection($id);
    my $has_coll = $s->{sql_shares}->exists({ connector => $id });
    $tpl_vars->{delete_block} = 'in_use' if $has_coll;

    my %conn = %{ $s->{sql_conn}->get({ id => $id })};
    $tpl_vars->{conn} = \%conn;
    return $TPL_DEL;
}

##
# Delete connector.
sub delete {
    validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
    my ($s, $tpl_vars, $id) = @_;
    my %conn;
    eval {
        croak "Connector extension '$id' does not exist."
            unless $s->{sql_conn}->is_extension($id);
        
        $s->del_test_collection($id);
        
        %conn = %{ $s->{sql_conn}->get({id => $id}) };
        rmtree("$CONFIG{conn_base_dir}/\Q$conn{name}\E");
        $s->{sql_conn}->delete({id => $id});

    };
    if ($@) {   
        $tpl_vars->{error} = $@
    }
    else { 
        $tpl_vars->{delete_ok} = $conn{name} 
    }
    return $s->show_list($tpl_vars);
}

##
# Show edit form for connectors
sub show_edit {
    validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
    my ($s, $tpl_vars, $conn_id) = @_;

    croak "Connector extension '$conn_id' does not exist"
        unless $s->{sql_conn}->exists({id => $conn_id, extension => 1});

    my @params = $s->{sql_param}->get({ connector => $conn_id });

    my $conn_ref = $s->{sql_conn}->get({id => $conn_id});
    $conn_ref->{source} = $s->read_source($conn_ref->{name});
    $conn_ref->{input_fields} # comma seperated list. remove whitespace.
        = [ map { s/(^\s+|\s+$)//g; $_ } split(",", $conn_ref->{inputFields}) ];

    $tpl_vars->{conn} = $conn_ref;
    $tpl_vars->{params} = \@params;
    #$tpl_vars->{parents} = [ $s->{sql_conn}->list(extendable => 1) ];
    
    $TPL_EDIT;
}

sub create_new {
    my $s = shift;

    my $name = "connector";
    $name .= $CHRS[int rand @CHRS] for 1..6;

    mkpath("$CONFIG{conn_base_dir}/$name");
    open my $fh, ">", sprintf $Page::Connector::SOURCE_TPL, "\Q$name\E"
        or croak "Unable to create main.pm: ", $!;
    print {$fh} $CONFIG{connector_src_skeleton};
    close $fh;

    my $id = $s->{sql_conn}->insert({ 
        name => $name, 
        extension => 1, 
        active => 0,
        inputFields => join(", ", @Page::Connector::REQ_INPUT_FIELDS),
        }, 1);

    return $id;
}


sub upload_source {
    validate_pos(@_, 1, { regex => qr(^\d+$)}, { type => OBJECT });
    my ($s, $conn_id, $file) = @_;

    $s->{sql_conn}->is_extension($conn_id)
        or croak "'$conn_id' is not a connector extension";
        

    my %conn = %{ $s->{sql_conn}->get({id => $conn_id}) };
    croak "No data for connector '$conn_id'"
        unless %conn;

    my ($source, $buff);
    while (read $file, $buff, 1024) {
        $source .= $buff;
    }

    $s->source_to_file($conn{name}, $source);
    1;
}


##
# Private methods


##
# Read connector source file.
sub read_source {
    validate_pos(@_, 1, 1);
    my ($s, $name) = @_;

    my $path = sprintf($Page::Connector::SOURCE_TPL, "\Q$name\E");
    open my $fh, "<", $path
        or croak "Unable to read '$path': ", $!;

    my $source;
    { local $/ = undef; $source = <$fh> }
    close $fh;

    return $source;
}




1;
