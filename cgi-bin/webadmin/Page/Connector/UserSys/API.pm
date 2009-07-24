package Page::Connector::UserSys::API;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos CODEREF HASHREF);
use Readonly;

use Data::UserSys;
use Sql::System;
use Sql::SystemParam;
use Sql::SystemConnector;
use Page::API;
our @ISA = qw(Page::Connector::API Page::API);

use config qw(%CONFIG);

Readonly::Scalar my $TPL_CFG => "conn_usersys_configure.html";
Readonly::Scalar my $TEST_LIST_USERS => "list_users";
Readonly::Scalar my $TEST_LIST_GROUPS => "user_groups";
Readonly::Scalar my $TEST_AUTH_USER => "auth_user";

sub _init {
	validate_pos(@_, 1, { regex => qr(^\d+$) }, 1);
	my ($s, $conn_id) = (shift, shift);

	$s->{sql_param} = Sql::SystemParam->new($s->{dbh});
	$s->{sql_conn} = Sql::SystemConnector->new($s->{dbh});
	$s->{sql_sys} = Sql::System->new($s->{dbh});
	
	$s->SUPER::_init(@_);

	$s->{conn} = $s->{sql_conn}->get({ id => $conn_id, extension => 1 }) 
		|| croak "'$conn_id' is not a connector extension";

	$s->{testsys} = $s->{utils}->init_test_sys($s, $s->{conn}{id});
}




sub cfg_html {
	validate_pos(@_, 1, 1, { type => CODEREF });
	my ($s, $api_vars, $tpl_func) = @_;

	my %html_vars;


	my %sys = $s->{testsys}->get();
	$sys{param} = [ $s->{testsys}->get_param_all() ];

	$html_vars{sys} = \%sys;

	my $output;
	&$tpl_func($TPL_CFG, \%html_vars, \$output);

	$api_vars->{html} = $output;
}

sub save_cfg_attr {
	my ($s, $api_vars, %cfg) = @_;
	my %data = (
		modified => ['NOW()'],
		active => $cfg{active}
		);
	$s->{sql_conn}->update(\%data, { id => $s->{conn}{id} });
	$api_vars->{ok} = "Updated.";
}

sub list_param {
	my ($s, $api_vars) = @_;
	$s->SUPER::list_param($api_vars, qw(required note));
}

sub add_param {
	validate_pos(@_, 1, { type => HASHREF }, { type => HASHREF });
	my ($s, $api_vars, $param_ref) = @_;
	$s->SUPER::add_param($api_vars, (
		note => $param_ref->{note},
		required => $param_ref->{required},
		param => $param_ref->{param}
		));
}

sub del_param {
	my ($s, $api_vars, $param) = @_;
	$s->SUPER::del_param($api_vars, { 
		param => $param, 
		connector => $s->{conn}{id} 
	});
}

sub save_source {
	my ($s, $api_vars, $content) = @_;

	my (undef, $path) = $s->{utils}->conn_path($s->{conn}{id});
	$s->SUPER::save_source($api_vars, $content, $path);
}

sub apply_test_cfg {
	my ($s, $api_vars, %raw_data) = @_;

	my %data;
	my @valid_param = $s->{sql_param}->get({ connector => $s->{conn}{id} });
	for my $p_ref (@valid_param) {
		my $p = $p_ref->{param};
		$data{$p} = $raw_data{$p}
			if $raw_data{$p};
	}
	
	eval {
		$s->{testsys}->update(params => \%data);
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Configuration updated.";
	}
}

sub test_run {
	my ($s, $api_vars, $method, $param_ref) = @_;
	my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
	my ($id, $output_file) = $s->new_test_output('usersys_test');
	$api_vars->{test_id} = $id;

	my $sys_id = $s->{testsys}->get('id');

	my $res;
	if ($method eq $TEST_LIST_USERS) {
		$res = $iq->listUsers($sys_id, logfile => $output_file);
	}
	elsif ($method eq $TEST_LIST_GROUPS) {
		my $user = $param_ref->[0];
		$res = $iq->userGroups($user, $sys_id, logfile => $output_file);
	}
	elsif ($method eq $TEST_AUTH_USER) {
		my ($user, $pass) = ($param_ref->[1], $param_ref->[2]);
		croak "TODO: not implemented in infoquery";
	}
	else { croak "Unknown test method '$method'" }
	$api_vars->{output} = $res ? Dumper($res) : $iq->error;
	$api_vars->{ok} = "Done.";
	1;
}

1;
