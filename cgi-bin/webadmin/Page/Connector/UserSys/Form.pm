package Page::Connector::UserSys::Form;
use strict;
use warnings;

use Page::Connector::Form;
use Sql::System;
use Sql::SystemConnector;
our @ISA = qw(Page::Connector::Form);

use File::Path qw(rmtree mkpath);
use Carp;
use Readonly;
use Data::Dumper;
use Params::Validate qw(validate_pos OBJECT);
use config qw(%CONFIG);

Readonly::Scalar my $TPL_LIST => "conn_usersys_list.html";
Readonly::Scalar my $TPL_DEL  => "conn_usersys_del.html";
Readonly::Scalar my $TPL_EDIT => "conn_usersys_edit.html";

sub _init {
	my $s = shift;
	$s->{sql_conn} = Sql::SystemConnector->new($s->{dbh});
	$s->{sql_sys} = Sql::System->new($s->{dbh});
	$s->{sql_param} = Sql::SystemParam->new($s->{dbh});
	$s->SUPER::_init(@_);
}

sub show_list {
	my ($s, $tpl_vars) = @_;
	$s->SUPER::show_list($tpl_vars);
	$TPL_LIST;
}

sub show_edit {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
	my ($s, $tpl_vars, $id) = @_;
	
	croak "extension '$id' does not exist."
		unless $s->{sql_conn}->exists({ id => $id, extension => 1 });
	my %conn = %{ $s->{sql_conn}->get({ id => $id }) };
	
	$conn{source} = $s->_read_source($id);
	$tpl_vars->{conn} = \%conn;

	my $testsys = $s->{utils}->init_test_sys($s, $id);
	$tpl_vars->{testsys}{id} = $testsys->get('id');

	return $TPL_EDIT;
}

sub new_connector {
	my $s = shift;
	my $prefix = "New connector ";
	my $i = 1;
	my $name = $prefix . $i;
	while ($s->{sql_conn}->exists({ name => $name })) {
		$name = $prefix . ++$i
	}
	
	return $s->create($name, $CONFIG{connector_usersys_skeleton} );
}

sub clone {
	my ($s, $id) = @_;
	croak "connector does not exist"
		unless $s->{sql_conn}->exists({ id => $id });
	my %conn = %{ $s->{sql_conn}->get({ id => $id }) };

	croak "'$id' is not a extension"
		unless $conn{extension};
	

	# new name
	my $i = 0;
	my $new_name;
	do {
		$new_name = sprintf "Copy %d of '$conn{name}'", ++$i;
	} while $s->{sql_conn}->exists({ name => $new_name });

	my $content = $s->_read_source($id);
	my @params = $s->{sql_param}->get({ connector => $id }, ['param', 'note', 'required']);
	
	return $s->create($new_name, $content, \@params);
}

sub _read_source {
	validate_pos(@_, 1, { regex => qr/^\d+$/ });
	my ($s, $id) = @_;
	my (undef, $path) = $s->{utils}->conn_path($id);
	open my $fh, "<", $path
		or croak "unable to open '$path' ", $!;
	return join q{}, <$fh>;
}

sub create {
	my ($s, $name, $content, $params_ref) = @_;

	my $id = $s->{sql_conn}->insert({
		name => $name,
		extension => 1,
		active => 0,
		read_only => 0
	}, 1);
	eval {
		my ($dir_path, $file_path, $id_path) = $s->{utils}->conn_path($id);
		croak "Connector already exists in path '$dir_path'"
			if -e $dir_path;
		mkpath($dir_path);


		open my $fh, ">", $file_path
			or croak "Unable to create main.pm: ", $!;
		print {$fh} $content;
		close $fh;

		open my $id_fh, ">", $id_path
			or croak "Unable to create id file: ", $!;
		print {$id_fh} $id;
		close $id_fh;

		if ($params_ref) { #cloning
			for my $p (@{$params_ref}) {
				$p->{connector} = $id;
				$s->{sql_param}->insert($p);
			}
		}
	};
	if ($@) { # clean up
		my $err = $@;
		$s->{sql_conn}->delete({ id => $id });
		croak $err;
	}

	return $id;
}

sub delete {
	validate_pos(@_, 1, 1, { regex => qr/^\d+$/ });
	my ($s, $tpl_vars, $id) = @_;

	
	croak "connector '$id' doesn't exist"
		unless $s->{sql_conn}->exists({ id => $id });
	my %conn = %{ $s->{sql_conn}->get({ id => $id }) };

	
	my $testsys = $s->{utils}->init_test_sys($s, $id); # TODO? Don't create.. just to delete it again.
	$s->{utils}->del_test_sys($testsys, $s);

	eval {
		croak "Connector is not an extension"
			unless $conn{extension};
		croak "Connector is read only"
			if $conn{read_only};
		croak "Connector is in use"
			if $s->_list_sys_using($id);
		rmtree(scalar $s->{utils}->conn_path($id));
		$s->{sql_conn}->delete({ id => $id });
	};
	if ($@) {
		$tpl_vars->{error} = $@;
		return $s->show_delete($tpl_vars, $id);
	}
	$tpl_vars->{delete_ok} = $conn{name};
	return $s->show_list($tpl_vars);
}

sub _list_sys_using { 
	my ($s, $id) = @_;
	my @coll = $s->{sql_sys}->get({ connector => $id }, 'name');
	return unless @coll;
	return map { $_->{name} } @coll;
}

sub show_delete {
	my ($s, $tpl_vars, $id) = @_;
	croak "'$id' does not exist"
		unless $s->{sql_conn}->exists({ id => $id });
	my %conn = %{ $s->{sql_conn}->get({ id => $id }) };

	croak "$id is not a extension"
		unless $conn{extension};
	croak "$id is read only"
		if $conn{read_only};
	
	my $testsys = $s->{utils}->init_test_sys($s, $id); # TODO? Don't create.. just to delete it again.
	$s->{utils}->del_test_sys($testsys, $s);
	
	if (my @sys = $s->_list_sys_using($id)) {
		$tpl_vars->{error} = "User system extension is in use by collections " . join(",", @sys);
	}

	$tpl_vars->{conn} = \%conn;
	return $TPL_DEL;
}

sub upload_source {
	validate_pos(@_, 1, { regex => qr(^\d+$)}, { type => OBJECT });
	my ($s, $conn_id, $file) = @_;
	my (undef, $path) = $s->{utils}->conn_path($conn_id);
	$s->SUPER::upload_source($conn_id, $file, $path);
}

1;
