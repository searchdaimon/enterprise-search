#!/usr/bin/perl
use strict;
use warnings;

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use CGI;
use Carp;
use Data::Dumper;
use Sql::Config;
use Page::Overview;
use Page::Overview::API;
use JSON::XS;


my $vars = { };
my %api_vars;
my $overview = Page::Overview->new();
my $api = Page::Overview::API->new();
my %state = $overview->get_state();

{ # Redirect to setup wizard if first timer.
    my $cfg = Sql::Config->new($overview->get_dbh);
    my $setup_done = $cfg->get_setting("setup_wizard_done");
    my $no_setup = $cfg->get_setting("no_setup");
    if ($no_setup) {
	# Dont want to do any setup. Skiping..
    }
    elsif (not defined $state{fetch_inner}
        and defined $setup_done # backwards compat: not defined => old installation
        and not $setup_done) {

        print "Location: setup.cgi\n\n";
        exit;
    }
}

Readonly::Scalar my $IS_API_CALL => $state{api} ? 1 : 0;


my $tpl_file = 'overview.html';


if (defined $state{action}) {
	my $action = $state{action};
	my $id = $state{id};
	my $connector = $state{connector};
	
	# User requested share to be crawled
	if ($action eq 'crawl') {
		$vars = $overview->crawl_collection($vars, $id, $connector);
		
		$tpl_file = 
			$overview->list_collections($vars);
	}
	
	elsif ($action eq 'edit') {
		# User wants to edit a collection. Show form.
		$vars->{'return_to'} = 'overview';
		$tpl_file = $overview->edit_collection($vars, $id, $connector);
	}
	
	elsif ($action eq 'delete') {
		# User wants to delete a collection. Confirm.
		$tpl_file = $overview->del_confirm($vars, id => $id);
	}
	
	elsif ($action eq 'activate') {
		# User is activating a disabled collection. Do it.
		$vars = $overview->activate_collection($vars, $id, $connector);
		$tpl_file = 
			$overview->list_collections($vars);
	}

	elsif ($action eq 'manage') {
		# User is in the advanced management tab.
		$tpl_file = $overview->manage_collection($vars, $id, $connector);
	}
	elsif ($action eq 'accesslevel') {
		# User is in the advanced management tab.
		$tpl_file = $overview->show_access_level($vars, $id, $connector);
	}
	elsif ($action eq 'console') {
		$tpl_file = $overview->show_console($vars, $id, $connector);
	}
	elsif ($action eq 'rread') {
		$tpl_file = $overview->show_rread($vars, $id, $connector, $state{lot}, $state{offset} );
	}
	elsif ($action eq 'pageinfo') {
		$tpl_file = $overview->show_pageinfo($vars, $id, $connector, $state{lot}, $state{DocID}, $state{offset}, $state{htmlSize}, $state{imagesize} );
	}
	elsif ($action eq 'htmldump') {
		$tpl_file = $overview->show_htmldump($vars, $id, $connector, $state{lot}, $state{DocID}, $state{offset}, $state{htmlSize}, $state{imagesize} );
	}
	elsif ($action eq 'documents') {
		$tpl_file = $overview->show_documents($vars, $id, $connector);
	}
	elsif ($action eq 'upload') {
		$tpl_file = $overview->show_upload($vars, $id, $connector);
	}
	elsif ($action eq 'customize') {
		$tpl_file = $overview->show_customize($vars, $id, $connector);
	}
	elsif ($action eq 'graphs') {
		$tpl_file = $overview->show_graphs($vars, $id, $connector);
	}
	elsif ($action eq 'stop_crawl') {
		$tpl_file = $overview->stop_crawl($vars, $id, $connector);
	}
	elsif ($action eq 'recrawl') {
		$tpl_file = $overview->recrawl_collection($vars, $id, $connector);
	}
	elsif ($action eq 'test_crawl') {
		$tpl_file = $overview->test_crawl_coll($vars, $id, $connector, $state{num_docs});
	}
	elsif ($action eq 'push_del') {
		$tpl_file = $overview->del_confirm($vars, 
			pushed => 1, name => $state{coll});
	}
	else { croak "Unknown action '$action'" }
}
elsif (my $method = $state{api}) {

	my $id = $state{id};

	my %methods = (
		console_output => sub {
                	#$page->save_source(\%api_vars, $state{edit}->{source})
			$api->console_output(\%api_vars, $id);
	        }
        );
    	my $func = $methods{$method}
        	or croak "Unknown api call '$method'";
    	&$func();

}
elsif (defined $state{advanced}) {
	my @action = keys %{$state{advanced}};
	croak Dumper(\@action);
	my $id = $state{id};
	my $connector = $state{connector};
}

elsif (defined $state{edit}) {
	# Show edit share form.
	my $collection = $state{edit};
	my $connector = $state{connector};
 	$tpl_file = $overview->edit_collection($vars, $collection, $connector);
}

elsif (defined $state{submit_edit}) {
	# User submits modification for a collection;
#
	
	my $valid = $overview->submit_edit($vars, $state{share});
		
	unless ($valid) {
		# Something wrong. Show edit form again.
		my $id = $vars->{share}{id};
		$tpl_file = $overview->edit_collection($vars, $id);
	}
	else {
		# We're done. Back to default page.
		$tpl_file = $overview->list_collections($vars);
	}
}



elsif (defined $state{confirm_delete}
    || defined $state{confirm_push_del}) {
	$state{confirm_delete} 
		? $overview->del_collection($vars, id => $state{id})
		: $overview->del_collection($vars, pushed => 1, coll => $state{coll});
	$tpl_file = $overview->list_collections($vars);
}


elsif (defined $state{fetch_inner}) {
	$tpl_file = $overview->list_collections($vars);
	$vars->{show_only_inner} = 1;
}

elsif (defined $state{upd_customization}) {
	$tpl_file = $overview->upd_customization($vars, $state{id}, %{$state{share}});
}

else {
	# Show default page (list of collections)
	$tpl_file
	    = $overview->list_collections($vars, $state{from_setup});
}

if ($IS_API_CALL) {
    	my $json = JSON::XS->new;
    	$json->pretty(1);
	print CGI::header(-type => 'text/plain', -expires => '-1h', -charset => "UTF-8");
    	print $json->encode(\%api_vars);	
}
else {
	print CGI::header(-type => 'text/html', -expires => '-1h', -charset => "UTF-8");
	$overview->process_tpl($tpl_file, $vars, ( 
	    ANYCASE => 0, # tpl system chokes on 'share.last', thinking last is token LAST
	    tpl_folders => 'overview', 
	    no_header => 1,
	));

}
