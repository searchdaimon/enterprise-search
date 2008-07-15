package Page::Connector::API;
use strict;
use warnings;
use Readonly;
use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos CODEREF);
#use File::Path qw(mkpath);
use File::Copy qw(move);
use Fcntl ':flock';
use CGI qw(escapeHTML);
use File::Temp qw(tempfile);

use Sql::SessionData;
use Data::Collection;
use Page::Connector;
use Boitho::Infoquery;
our @ISA = qw(Page::Connector);

use config qw(%CONFIG);


Readonly::Scalar my $TPL_CFG  => "conn_configure.html";
Readonly::Scalar my $TMP_FILE_TPL => "/tmp/conn_testrun_XXXXXXXX";


sub _init {
    validate_pos(@_, 1, { regex => qr(^\d+$) });
    my ($s, $conn_id) = (shift, shift);

    $s->SUPER::_init(@_);

    $s->{conn} = $s->{sql_conn}->get({id => $conn_id});
    $s->{coll}{name} = sprintf $CONFIG{test_coll_name}, $s->{conn}{name};
    $s->{coll}{id}   = $s->init_test_collection();
}


##
# `Settings and parameters' tab

##
# Update field and active attribute
sub save_cfg_attr {
    my ($s, $api_vars, %cfg) = @_;
  
    eval {
        croak "Connector extension does not exist"
            unless $s->{sql_conn}->is_extension($s->{conn}{id});

        my %data = (
            modified => ['NOW()'],
            active   => $cfg{active},
        );

        $cfg{field} ||= {};
        my %req = map { $_ => 1 } @Page::Connector::REQ_INPUT_FIELDS;
        my @input_fields = @Page::Connector::REQ_INPUT_FIELDS;
        for my $k (keys %{$cfg{field}}) {
            next if $req{$k};
            push @input_fields, $k;
        }
        $data{inputFields} = join ", ", @input_fields;

        $s->{sql_conn}->update(\%data, { id => $s->{conn}{id} });
    };

    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Updated.";
    }
}

##
# Upd connector name
sub save_cfg_name {
    my ($s, $api_vars, $new_name) = @_;

    my $f = sub {
        croak "Connector extension does not exist"
            unless $s->{sql_conn}->is_extension($s->{conn}{id});
        
        croak "Connector name is cannot be blank."
            if $new_name eq q{}; # `0' is an valid name

        unless ($new_name =~ /^[a-zA-Z0-9_]+$/) {
            croak "Invalid connector name. Valid characters",
                  " are letters, numbers and '_'. (no space)";
        }

        return if $new_name eq $s->{conn}{name}; # no change.

        # Delete test collection.   
        $s->del_test_collection($s->{conn}{id});
        

        # Move source file.
        croak "Collection name taken"
            if $s->{sql_conn}->exists({name => $new_name});

        my $new_path = "$CONFIG{conn_base_dir}/\Q$new_name\E";
        my $old_path = "$CONFIG{conn_base_dir}/\Q$s->{conn}{name}\E";
        croak "Directory/file named $new_name already exists."
            if -e $new_path;

        move($old_path, $new_path)
            or croak "Unable to move '$old_path' to '$new_path': ", $!;

        
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

sub add_param {
    my ($s, $api_vars, $param, $example) = @_;
    my $param_id;
    eval {
        croak "No parameter provided"
            unless $param;
        croak "Parameter '$param' exists"
            if $s->{sql_param}->exists({connector => $s->{conn}{id}, param => $param });

        $param_id = $s->{sql_param}->insert({
            connector => $s->{conn}{id},
            param => $param,
            example => $example,
        }, 1);
    };
    unless ($s->api_error($@)) {
        $api_vars->{ok} = "Parameter added.";
        $api_vars->{param_id} = $param_id;
    }
    1;
}

sub del_param {
    my ($s, $api_vars, $param_id) = @_;
    eval {
        croak "Parameter doesn't exist"
            unless $s->{sql_param}->exists({ id => $param_id });

        $s->{sql_param}->delete({ id => $param_id });
    };
    unless ($s->api_error($@)) {
        $api_vars->{ok} = "Parameter deleted.";
    }
    1; 
}


##
# `Edit source' tab

##
# Store connector source file
sub save_source {
    my ($s, $api_vars, $content) = @_;
    
    eval {
        croak "Connector extension '$s->{conn}{id}' is not valid"
            unless $s->{sql_conn}->exists({id => $s->{conn}{id}, extension => 1});

        my %conn = % { $s->{sql_conn}->get({id => $s->{conn}{id}}) };

        $s->source_to_file($conn{name}, $content);
    };
    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Saved.";
    }
    1;
}


##
# `Configure test collection' tab

sub apply_test_cfg {
    my ($s, $api_vars, %raw_coll_data) = @_;
    
    my %coll_data;
    $coll_data{id} = $s->{coll}{id};

    for my $attr (keys %raw_coll_data) {
        if ($COLLECTION_ATTR{$attr}) {
            $coll_data{$attr} = $raw_coll_data{$attr};
        }
    }
    
    my $coll = Data::Collection->new(
        $s->{dbh},
        \%coll_data);

    unless ($coll_data{auth_id}) {
        $coll_data{auth_id} = $coll->set_auth(
            $raw_coll_data{username}, 
            $raw_coll_data{password});
    }

    $coll->update();
    $api_vars->{ok} = "Configuration updated.";
}

##
# Fetches form HTML for configuring connector.
sub cfg_html {
    validate_pos(@_, 1, 1, { type => CODEREF });
    my ($s, $api_vars, $tpl_func) = @_;

    my %html_vars;
    # Get data for tpl
    my $coll = Data::Collection->new($s->{dbh}, { 
        connector => $s->{conn}{id},
        id => $s->{coll}{id},
    });

    my %form_data = $coll->form_data();
    while (my ($k, $v) = each %form_data) {
        $html_vars{$k} = $v 
    }

    my %coll_data = $coll->coll_data();
    $html_vars{share} = \%coll_data;

    my $output;
    &$tpl_func($TPL_CFG, \%html_vars, \$output);

    $api_vars->{html} = $output;
}




##
# `Run test' tab

##
# Starts run using infoquery
sub test_run {
    my ($s, $api_vars) = @_;
    my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
    my $sessData = Sql::SessionData->new($s->{dbh});

    my (undef, $output_file) = tempfile($TMP_FILE_TPL, OPEN => 0);

    my $id = $sessData->insert(
        type => "connector_test", 
        data => $output_file
    );
    $api_vars->{test_id} = $id;

    my $ok = $iq->recrawlCollection(
        $s->{coll}{name}, logfile => $output_file);
    
    if (!$ok) {
        $s->api_error($api_vars, "Error during crawl: " . $iq->error());
    }
    else {
        $api_vars->{ok}    = "Test crawl running.";
    }
    1;
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


##
# Kills test run for given test_id
sub test_kill {
    validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
    my ($s, $api_vars, $test_id) = @_;
    
    my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
    my $sessData = Sql::SessionData->new($s->{dbh});

    my $output_path = { $sessData->get($test_id) }->{data};

    eval {
        my $pid = $s->{sql_shares}->get(
			{ id => $s->{coll}{id} }, 'crawl_pid')->{crawl_pid};

		croak "Crawl is not running."
			unless defined $pid;

        $iq->killCrawl($pid)
            or croak $iq->error();
    };
    unless ($s->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Crawl killed.";
    }
}


##
# Private methods

sub api_error {
    my ($s, $api_vars, $err) = @_;
    if ($err) {
        carp $err;
        $err =~ s/at .* line \d+$//g;
        $api_vars->{error} = $err;
        return 1;
    }
    return;
}


##
# Fetches the id of the test collection.
# Creates collection if needed.
sub init_test_collection {
    my $s = shift;
    my $test_coll = $s->{sql_shares}->get({ collection_name => $s->{coll}{name} }, 'id');
    return $test_coll->{id} 
        if $test_coll;
    
    # Collection does not exist, create.
    my $coll = Data::Collection->new($s->{dbh}, { 
        collection_name => $s->{coll}{name},
        connector => $s->{conn}{id},
        active => 0,
    });
    $coll->create();
    return { $coll->get_attr() }->{id};
}


