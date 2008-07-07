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


my $template_file = 'overview.html';


if (defined $state{action}) {
	my $action = $state{action};
	my $id = $state{id};
	
	# User requested share to be crawled
	if ($action eq 'crawl') {
		$vars = $overview->crawl_collection($vars, $id);
		
		$template_file = 
			$overview->list_collections($vars);
	}
	
	elsif ($action eq 'edit') {
		# User wants to edit a collection. Show form.
		$vars->{'return_to'} = 'overview';
		$template_file = $overview->edit_collection($vars, $id);
	}
	
	elsif ($action eq 'delete') {
		# User wants to delete a collection. Confirm.
		$template_file = $overview->delete_collection($vars, $id);
	}
	
	elsif ($action eq 'activate') {
		# User is activating a disabled collection. Do it.
		$vars = $overview->activate_collection($vars, $id);
		$template_file = 
			$overview->list_collections($vars);
	}

	elsif ($action eq 'manage') {
		# User is in the advanced management tab.
		($vars, $template_file) = $overview->manage_collection($vars, $id);
	}
}

elsif (defined $state{advanced}) {
	my @action = keys %{$state{advanced}};
	my $id = $state{id};

	if ($action[0] eq 'full_recrawl') {
		# User is forcing a full recrawl from management.
		my $submit_values = 
			$state{advanced}{full_recrawl};
		($vars, $template_file) = 
			$overview->recrawl_collection($vars, $submit_values);
	}
}

elsif (defined $state{edit}) {
	# Show edit share form.
	my $collection = $state{edit};
 	$template_file = $overview->edit_collection($vars, $collection);
}

elsif (defined $state{submit_edit}) {
	# User submits modification for a collection;

       my $valid;
       ($vars, $valid) = $overview->submit_edit($vars, $state{share});

	
		
	unless ($valid) {
		# Something wrong. Show edit form again.
		my $id = $vars->{share}{id};
		$template_file = $overview->edit_collection($vars, $id);
	}
	else {
		# We're done. Back to default page.
		$template_file = $overview->list_collections($vars);
	}
}



elsif (defined $state{confirm_delete}) {
	my $id = $state{id};
	$vars = $overview->delete_collection_confirmed($vars, $id);
	
	$template_file = $overview->list_collections($vars);
}

elsif (defined $state{fetch_inner}) {
	$template_file = $overview->list_collections($vars);
	$vars->{show_only_inner} = 1;
}

else {
	# Show default page (list of collections)
	$template_file
	    = $overview->list_collections($vars);
}


print CGI::header(-type => 'text/html', -expires => '-1h', -charset => "UTF-8");
$overview->process_tpl($template_file, $vars, ( 
    ANYCASE => 0, # tpl system chokes on 'share.last', thinking last is token LAST
    tpl_folders => 'overview', 
    no_header => 1,
));

