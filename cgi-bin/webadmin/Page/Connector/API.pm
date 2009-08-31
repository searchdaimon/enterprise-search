package Page::Connector::API;
use strict;
use warnings;

use Carp;
use Params::Validate qw(validate_pos HASHREF);
use Data::Dumper;
use File::Temp qw(tempfile);
use Fcntl ':flock';
use CGI qw(escapeHTML);

use config qw(%CONFIG);

use Page::API;
use Page::Abstract;
our @ISA = qw(Page::Abstract Page::API);

Readonly::Scalar my $TMP_FILE_TPL => "/tmp/conn_testrun_XXXXXXXX";

sub _init {
	validate_pos(@_, 1, 0);
	my ($s, $utils) = @_;
	for (qw(sql_param sql_conn)) {
		croak "subclass has not defined $_"
			unless $s->{$_};
	}
	$s->{utils} = $utils;
	$s;
}

sub add_param {
    my ($s, $api_vars, %param) = @_;
    my $param_id;
    eval {
	croak "Connector extension is read only"
		if $s->{sql_conn}->is_readonly($s->{conn}{id});
        croak "No parameter provided"
            unless $param{param};
        croak "Parameter '$param{param}' exists"
            if $s->{sql_param}->exists({connector => $s->{conn}{id}, param => $param{param} });

	$param{connector} = $s->{conn}{id};
        $param_id = $s->{sql_param}->insert(\%param, 1);
    };
    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Parameter added.";
        $api_vars->{param_id} = $param_id;
    }
    1;
}

sub list_param {
	my ($s, $api_vars,@fields) = @_;
	unshift @fields, "param";
	$api_vars->{parameters} = [ $s->{sql_param}->get({ 
		connector => $s->{conn}{id} 
	}, \@fields) ];
	$api_vars->{ok} = 1;
	1;
}

sub del_param {
	my ($s, $api_vars, $where) = @_;
	eval {
		croak "Connector extension is read only"
			if $s->{sql_conn}->is_readonly($s->{conn}{id});

		croak "Parameter doesn't exist"
			unless $s->{sql_param}->exists($where);

		$s->{sql_param}->delete($where);
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Parameter deleted.";
	}
	1; 
}
sub save_cfg_attr {
	validate_pos(@_, 1, { type => HASHREF }, { type => HASHREF }, 0);
	my ($s, $api_vars, $cfg_ref, $data_ref) = @_;

	$data_ref ||= { };

	eval {
		croak "Connector extension does not exist"
			unless $s->{sql_conn}->is_extension($s->{conn}{id});
		croak "Connector extension is read only"
			if $s->{sql_conn}->is_readonly($s->{conn}{id});

		$data_ref->{modified} = ['NOW()'];
		$data_ref->{active} = $cfg_ref->{active};

		$s->{sql_conn}->update($data_ref, { id => $s->{conn}{id} });
	};

	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Updated.";
	}
	$s;
}

##
# Store connector source file
sub save_source {
	validate_pos(@_, 1, 1, 1, 1);
	my ($s, $api_vars, $content, $path) = @_;

	eval {
		croak "Connector extension '$s->{conn}{id}' is not valid"
			unless $s->{sql_conn}->exists({id => $s->{conn}{id}, extension => 1});
		croak "Connector extension is read only"
			if $s->{sql_conn}->is_readonly($s->{conn}{id});

		my %conn = % { $s->{sql_conn}->get({id => $s->{conn}{id}}) };

		open my $fh, ">", $path
			or croak "unable to open '$path' for writing", $!;
		print {$fh} $content;
	};
	unless ($s->api_error($api_vars, $@)) {
		$api_vars->{ok} = "Saved.";
	}
	1;
}


##
# Upd connector name
sub save_cfg_name {
    my ($s, $api_vars, $new_name, $extra_checks, $before_db_update) = @_;

    my $f = sub {
        croak "Connector extension does not exist"
            unless $s->{sql_conn}->is_extension($s->{conn}{id});
	croak "Connector extension is read only"
		if $s->{sql_conn}->is_readonly($s->{conn}{id});
        
        croak "Connector name is cannot be blank."
            if $new_name eq q{}; # `0' is an valid name

	&$extra_checks() if $extra_checks;

        return if $new_name eq $s->{conn}{name}; # no change.

	&$before_db_update() if $before_db_update;
               
        # Update db
        $s->{sql_conn}->update(
            { name => $new_name, 
              modified => ['NOW()'] } , 
            { id => $s->{conn}{id} });


    };
    eval { &$f() };
    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Connector name updated.";
    }
}

sub apply_test_cfg {
	croak "not implemented";
}

##
# Fetches output for given test_id
sub test_output {
    validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
    my ($s, $api_vars, $test_id) = @_;

    my $sessData = Sql::SessionData->new($s->{dbh});
    my $output_path = { $sessData->get($test_id) }->{data};

    my $done;

    eval {
	croak "Results for testrun do not exist."
    	    unless defined $output_path && -e $output_path;

        open my $fh, "<", $output_path
            or croak $!;

        # If crawler manager has a lock on the file, it's still crawling.
        $done = flock $fh, LOCK_EX | LOCK_NB;
        $api_vars->{crawl_done} = $done;
   
        $api_vars->{output} = escapeHTML(join q{}, <$fh>);
    };
    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Output fetched.";
    }

    if ($done) {
        $sessData->delete({ id => $test_id });
        unlink $output_path;
    }
}

sub new_test_output {
	my ($s, $sess_type) = @_;
	my $sessData = Sql::SessionData->new($s->{dbh});
	my (undef, $output_file) = tempfile($TMP_FILE_TPL, OPEN => 0);
	my $id = $sessData->insert(
		type => $sess_type,
		data => $output_file,
	);
	return ($id, $output_file);
}

1;
