package Page::Connector::Crawler::API;
use strict;
use warnings;
use Readonly;
use Carp;
use Data::Dumper;
use Params::Validate qw(validate_pos CODEREF HASHREF);
#use File::Path qw(mkpath);
use File::Copy qw(move);

use Sql::SessionData;
use Sql::System;
use Sql::Connectors;
use Sql::Shares;
use Sql::Param;

use Data::Collection;
use Boitho::Infoquery;
use Page::API;
use Page::Connector::API;
our @ISA = qw(Page::Connector::API Page::API);

use config qw(%CONFIG);

Readonly::Scalar my $TPL_CFG  => "conn_crawler_configure.html";


sub _init {
	my ($s, $conn_id) = (shift, shift);

	$s->{sql_conn} = Sql::Connectors->new($s->{dbh});
	$s->{sql_shares} = Sql::Shares->new($s->{dbh});
	$s->{sql_param} = Sql::Param->new($s->{dbh});

	$s->{conn} = $s->{sql_conn}->get({id => $conn_id, extension => 1})
		or croak "'$conn_id' is not a connector extension";
	$s->{coll}{name} = sprintf $CONFIG{test_coll_name}, $s->{conn}{name};
	$s->{coll}{id}   = $s->init_test_collection();

	$s->SUPER::_init(@_);
}


##
# `Settings and parameters' tab

##
# Update field and active attribute
sub save_cfg_attr {
	my ($s, $api_vars, %cfg) = @_;

	$cfg{field} ||= {};
	my %input_fields = map { $_ => 1 } (keys %{$cfg{field}}, $s->{utils}->req_input_fields);

	$s->SUPER::save_cfg_attr($api_vars, \%cfg, {
		inputFields => join ", ", keys %input_fields
	});
}


sub save_cfg_name {
	my ($s, $api_vars, $new_name) = @_;

	my $connector_checks = sub { 
		unless ($new_name =~ /^[a-zA-Z0-9_]+$/) { # TODO: Use same check as Add collection
			croak "Invalid connector name. Valid characters",
				" are letters, numbers and '_'. (no space)";
		}
	};

	$s->SUPER::save_cfg_name($api_vars, $new_name, $connector_checks, sub {
		# Delete test collection.   
		$s->{utils}->del_test_collection($s->{conn}{id}, $s);

		# Move source file.
		croak "Collection name taken"
			if $s->{sql_conn}->exists({name => $new_name});

		my $new_path = "$CONFIG{conn_base_dir}/\Q$new_name\E";
		my $old_path = "$CONFIG{conn_base_dir}/\Q$s->{conn}{name}\E";
		croak "Directory/file named $new_name already exists."
			if -e $new_path;

		move($old_path, $new_path)
			or croak "Unable to move '$old_path' to '$new_path': ", $!;
	});

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

	my ($id, $output_file)  = $s->new_test_output('connector_test');
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

##
# Fetches the id of the test collection.
# Creates collection if needed.
sub init_test_collection {
    my $s = shift;
    my $test_coll = $s->{sql_shares}->get({ collection_name => $s->{coll}{name} }, 'id');
    return $test_coll->{id} 
        if $test_coll;

	# Fetch primary system
	my $sqlSys = Sql::System->new($s->{dbh});
	my $prim_sys = $sqlSys->primary_id()
		|| croak "No primary system found.";
    
    # Collection does not exist, create.
    my $coll = Data::Collection->new($s->{dbh}, { 
        collection_name => $s->{coll}{name},
        connector => $s->{conn}{id},
        active => 0,
	'system' => $prim_sys,
    });
    $coll->create();
    return { $coll->get_attr() }->{id};
}

sub list_param {
	my ($s, $api_vars) = @_;
	$s->SUPER::list_param($api_vars, qw(example id));
}

sub add_param {
	validate_pos(@_, 1, { type => HASHREF }, { type => HASHREF });
	my ($s, $api_vars, $param_ref) = @_;
	$s->SUPER::add_param($api_vars, (
		param => $param_ref->{param},
		example => $param_ref->{example}
		));
}

sub del_param {
	my ($s, $api_vars, $param_id) = @_;
	$s->SUPER::del_param($api_vars, { id => $param_id });
}

sub save_source {
	my ($s, $api_vars, $content) = @_;
	my $path = $s->{utils}->conn_path($s->{conn}{name});
	$s->SUPER::save_source($api_vars, $content, $path);
}
1;
