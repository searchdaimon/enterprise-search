package Page::Connector::Crawler::Form;
use strict;
use warnings;

use Carp;
use Readonly;
use File::Path qw(rmtree mkpath);
use Data::Dumper;
use Params::Validate qw(validate_pos OBJECT SCALAR ARRAYREF);

use Page::Connector::Form;
our @ISA = qw(Page::Connector::Form);

use config qw(%CONFIG);

Readonly::Array my @CHRS 
    => ('a'..'z', 'A'..'Z');

Readonly::Scalar my $TPL_LIST => 'conn_crawler_list.html';
Readonly::Scalar my $TPL_EDIT => 'conn_crawler_edit.html';
Readonly::Scalar my $TPL_DEL  => 'conn_crawler_del.html';
Readonly::Scalar my $CONN_CLONE_TPL => "Copy_%d_of_%s";

sub _init {
	my $s = shift;
	$s->{sql_conn} = Sql::Connectors->new($s->{dbh});
	$s->{sql_param} = Sql::Param->new($s->{dbh});
	$s->{sql_shares} = Sql::Shares->new($s->{dbh});
	$s->SUPER::_init(@_);
}

##
# Show list of connectors
sub show_list {
	my ($s, $tpl_vars) = @_;
	$s->SUPER::show_list($tpl_vars);
	$TPL_LIST;
}

##
# Confirm del dialog.
sub show_delete {
    my ($s, $tpl_vars, $id) = @_;
    $s->{sql_conn}->is_extension($id)
        or croak "'$id' is not a connector extension";
    not $s->{sql_conn}->is_readonly($id)
    	or croak "'$id' is read only";

    
    # check if has collections
    $s->{utils}->del_test_collection($id, $s);
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
	croak "Connector '$id' is read only"
		if $s->{sql_conn}->is_readonly($id);
        
        $s->{utils}->del_test_collection($id, $s);
        
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

    #my @params = $s->{sql_param}->get({ connector => $conn_id });

    my $conn_ref = $s->{sql_conn}->get({id => $conn_id});
    $conn_ref->{source} = $s->read_source($conn_ref->{name});
    $conn_ref->{input_fields} # comma seperated list. remove whitespace.
        = [ map { s/(^\s+|\s+$)//g; $_ } split(",", $conn_ref->{inputFields}) ];

    $tpl_vars->{conn} = $conn_ref;
    #$tpl_vars->{params} = \@params;
    #$tpl_vars->{parents} = [ $s->{sql_conn}->list(extendable => 1) ];
    
    $TPL_EDIT;
}

sub new_connector {
	my $s = shift;

	my $name = "New_connector_";
	$name .= $CHRS[int rand @CHRS] for 1..6;

	return $s->create($name, 
			$CONFIG{connector_src_skeleton}, 
			join ",", @Page::Connector::REQ_INPUT_FIELDS);
}

sub create {
	validate_pos(@_, 1, {type => SCALAR}, {type => SCALAR}, {type => SCALAR}, 0);
	my ($s, $name, $src_content, $input_fields, $param_ref) = @_;

	mkpath("$CONFIG{conn_base_dir}/$name");
	open my $fh, ">", $s->{utils}->conn_path($name)
		or croak "Unable to create main.pm: ", $!;
	print {$fh} $src_content;
	close $fh;

	my $id = $s->{sql_conn}->insert({ 
			name => $name, 
			extension => 1, 
			active => 0,
			read_only => 0,
			inputFields => $input_fields,
			}, 1);

	return $id;
}

sub clone {
	my ($s, $id) = @_;

	# Fetch other connector
	$s->{sql_conn}->is_extension($id)
		or croak "'$id' is not a connector extension";

	my $to_clone = $s->{sql_conn}->get({ id => $id })
		or croak "Connector '$id' does not exist";
	$to_clone->{extension} 
		or croak "Connector '$id' is not an extension";

	my @params = $s->{sql_param}->get(
		{ connector => $id }, 
		['param', 'example']
	);

	# New name
	my $copy_nmbr = 1;
	my $new_name;
	do {
		$new_name = sprintf($CONN_CLONE_TPL, 
			$copy_nmbr++, 
			$to_clone->{name}
		);
	} while $s->{sql_conn}->exists({ name => $new_name });

	# Create clone
	my $src_content = $s->read_source($to_clone->{name});
	my $clone_id = $s->create($new_name, 
		   $s->read_source($to_clone->{name}),
		   $to_clone->{inputFields}
	);

	# Add params
	for my $param_ref (@params) {
		$param_ref->{connector} = $clone_id;
		$s->{sql_param}->insert($param_ref);
	}

	return $clone_id;
}

sub upload_source {
	validate_pos(@_, 1, { regex => qr(^\d+$)}, { type => OBJECT });
	my ($s, $conn_id, $file) = @_;

	my $conn_ref = $s->{sql_conn}->get({ id => $conn_id })
		|| croak "No data for connector '$conn_id'";

	my $path = $s->{utils}->conn_path($conn_ref->{name});
	$s->SUPER::upload_source($conn_id, $file, $path);
}


##
# Private methods


##
# Read connector source file.
sub read_source {
    validate_pos(@_, 1, 1);
    my ($s, $name) = @_;

    my $path = $s->{utils}->conn_path($name);
    open my $fh, "<", $path
        or croak "Unable to read '$path': ", $!;

    my $source;
    { local $/ = undef; $source = <$fh> }
    close $fh;

    return $source;
}




1;
