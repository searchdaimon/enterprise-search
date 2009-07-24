#!/usr/bin/env perl
use strict;
use warnings;
use Page::Connector::Factory qw($CONN_CRAWLER $CONN_USERSYS);
use Data::Dumper;
use JSON::XS;
use Carp;
use CGI;
use Readonly;
Readonly::Array my @TPL_FOLDERS => ('common', 'connector/crawler', 'connector/usersys', 'connector', 'usersys');

use Page::Abstract; # ._.
my %state = Page::Abstract::get_state();

Readonly::Scalar my $CONN_TYPE => $state{conntype} || $CONN_CRAWLER;
if ($CONN_TYPE != $CONN_USERSYS && $CONN_TYPE != $CONN_CRAWLER) {
	croak "Unknown connector type '$CONN_TYPE'.";
}
Readonly::Scalar my $IS_API_CALL => $state{api} ? 1 : 0;

my %api_vars;
my %tpl_vars;
my $tpl_file;

my $page = $IS_API_CALL 
	? Page::Connector::Factory->api($CONN_TYPE, $state{connid})
	: Page::Connector::Factory->form($CONN_TYPE);

if (my $show = $state{show}) {
	my $id = $state{connid};

	if ($show eq 'edit') { 
		$tpl_file = $page->show_edit(\%tpl_vars, $id);
	}
	elsif ($show eq 'del') {
		$tpl_file = $page->show_delete(\%tpl_vars, $state{connid});
	}

	else { croak "Unknown form '$show'" }
}

elsif (my $act = $state{act}) {
	if ($act eq 'upload') {
		$page->upload_source(
				$state{connid},
				$state{upload}->{file}
				);
		$tpl_vars{selected_tab} = 1; #switch to edit after upl
		$tpl_file = $page->show_edit(\%tpl_vars, $state{connid});
	}

	elsif ($act eq 'new') {
		my $conn_id = $page->new_connector();
		$tpl_file = $page->show_edit(\%tpl_vars, $conn_id);
	}
	elsif ($act eq 'clone') {
		my $conn_id = $page->clone($state{connid});
		$tpl_file = $page->show_edit(\%tpl_vars, $conn_id);
	}
	elsif ($act eq 'del') {
		croak "not a post request"
			unless $ENV{REQUEST_METHOD} eq 'POST';
		unless ($state{abort_delete}) {
			$tpl_file = $page->delete(\%tpl_vars, $state{id});
		}
		else {
			$tpl_file = $page->show_list(\%tpl_vars);
		}
	}
	else { croak "Unknown action '$act'" }

}
elsif (my $method = $state{api}) {
    my %methods = (
    	edit => sub { 
		$page->save_source(\%api_vars, $state{edit}->{source}) 
	},
	save_attr => sub {
        	$page->save_cfg_attr(\%api_vars, %{$state{save}})
	},
        save_name => sub {
		$page->save_cfg_name(\%api_vars, $state{save}->{name});
	},
	param_list => sub {
		$page->list_param(\%api_vars);
	},
    	param_add => sub {
		$page->add_param(\%api_vars, $state{param});
    	},
	param_del => sub {
        	$page->del_param(\%api_vars, $state{param}->{id});
	},
	test_apply => sub {
		my $cfgvalues = $state{share} || $state{sys}{params};
        	$page->apply_test_cfg(\%api_vars, %{$cfgvalues});
	},
	test_run => sub {
        	$page->test_run(\%api_vars, $state{method}, $state{params});
    	},
	test_output => sub {
        	$page->test_output(\%api_vars, $state{test_id});
	},
    	test_kill => sub {
        	$page->test_kill(\%api_vars, $state{test_id});
    	},
	test_cfg_html => sub {
        	# fetch HTML for test coll config options.
        	$page->cfg_html(\%api_vars, sub {
            		my ($file, $vars, $output_ref) = @_;
            		$page->process_tpl($file, $vars, ( 
                		tpl_folders => \@TPL_FOLDERS,
                		html_output => $output_ref));
        	});
	});
    my $func = $methods{$method} 
    	or croak "Unknown api call '$method'";
    &$func();
}
else { 
	# asuming Page::Connector::Form instance.
	$tpl_file = $page->show_list(\%tpl_vars);
}

if ($IS_API_CALL) {
    my $json = JSON::XS->new;
    $json->pretty(1);
    print $page->get_cgi->header("text/plain");
    print $json->encode(\%api_vars);
}
else {
	$tpl_vars{conntype} = $CONN_TYPE;
    $page->process_tpl($tpl_file, \%tpl_vars, ( tpl_folders => \@TPL_FOLDERS ));
}

1;
