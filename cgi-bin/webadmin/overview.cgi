#!/usr/bin/perl
use strict;
use warnings;
use CGI;
use Carp;
use Data::Dumper;
use Sql::Config;
use Page::Overview;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}


my $vars = { };
my $overview = Page::Overview->new();
my %state = $overview->get_state();

{ # Redirect to setup wizard if first timer.
    my $cfg = Sql::Config->new($overview->get_dbh);
    my $setup_done = $cfg->get_setting("setup_wizard_done");
    if (not defined $state{fetch_inner}
        and defined $setup_done # backwards compat: not defined => old installation
        and not $setup_done) {

        print "Location: setup.cgi\n\n";
        exit;
    }
}


my $tpl_file = 'overview.html';


if (defined $state{action}) {
	my $action = $state{action};
	my $id = $state{id};
	
	# User requested share to be crawled
	if ($action eq 'crawl') {
		$vars = $overview->crawl_collection($vars, $id);
		
		$tpl_file = 
			$overview->list_collections($vars);
	}
	
	elsif ($action eq 'edit') {
		# User wants to edit a collection. Show form.
		$vars->{'return_to'} = 'overview';
		$tpl_file = $overview->edit_collection($vars, $id);
	}
	
	elsif ($action eq 'delete') {
		# User wants to delete a collection. Confirm.
		$tpl_file = $overview->del_confirm($vars, id => $id);
	}
	
	elsif ($action eq 'activate') {
		# User is activating a disabled collection. Do it.
		$vars = $overview->activate_collection($vars, $id);
		$tpl_file = 
			$overview->list_collections($vars);
	}

	elsif ($action eq 'manage') {
		# User is in the advanced management tab.
		$tpl_file = $overview->manage_collection($vars, $id);
	}
	elsif ($action eq 'customize') {
		$tpl_file = $overview->show_customize($vars, $id);
	}
	elsif ($action eq 'graphs') {
		$tpl_file = $overview->show_graphs($vars, $id);
	}
	elsif ($action eq 'stop_crawl') {
		$tpl_file = $overview->stop_crawl($vars, $id);
	}
	elsif ($action eq 'recrawl') {
		$tpl_file = $overview->recrawl_collection($vars, $id);
	}
	elsif ($action eq 'test_crawl') {
		$tpl_file = $overview->test_crawl_coll($vars, $id, $state{num_docs});
	}
	elsif ($action eq 'push_del') {
		$tpl_file = $overview->del_confirm($vars, 
			pushed => 1, name => $state{coll});
	}
	else { croak "Unknown action '$action'" }
}

elsif (defined $state{advanced}) {
	my @action = keys %{$state{advanced}};
	croak Dumper(\@action);
	my $id = $state{id};
}

elsif (defined $state{edit}) {
	# Show edit share form.
	my $collection = $state{edit};
 	$tpl_file = $overview->edit_collection($vars, $collection);
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


print CGI::header(-type => 'text/html', -expires => '-1h', -charset => "UTF-8");
$overview->process_tpl($tpl_file, $vars, ( 
    ANYCASE => 0, # tpl system chokes on 'share.last', thinking last is token LAST
    tpl_folders => 'overview', 
    no_header => 1,
));

