#!/usr/bin/perl
use strict;
use warnings;
use Template;
use CGI;
use Carp;
use Sql::Sql;
use Sql::Connectors;
use Sql::Shares;
use CGI::State;
use Page::Overview;
use Data::Dumper;
use Template::Stash;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);


my $vars = { };

my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();
my $overview = Page::Overview->new($dbh, $state);

my $sqlConnectors = Sql::Connectors->new($dbh);

my $template_file = 'overview.html';



if (defined($state->{'action'})) {
	my $action = $state->{'action'};
	my $id = $state->{'id'};
	
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

elsif(defined($state->{'advanced'})) {
	my @action = keys(%{$state->{'advanced'}});
	my $id = $state->{'id'};

	if ($action[0] eq 'full_recrawl') {
		# User is forcing a full recrawl from management.
		my $submit_values = 
			$state->{'advanced'}{'full_recrawl'};
		($vars, $template_file) = 
			$overview->recrawl_collection($vars, $submit_values);
	}
}

elsif (defined($state->{'edit'})) {
	# Show edit share form.
	my $collection = $state->{'edit'};
 	$template_file = $overview->edit_collection($vars, $collection);
}

elsif (defined($state->{'submit_edit'})) {
	# User submits modification for a collection;
	
	my $valid;
	($vars, $valid) = $overview->submit_edit($vars, $state->{'share'});
		
	unless ($valid) {
		# Something wrong. Show edit form again.
		my $id = $vars->{'share'}{'id'};
		$template_file = $overview->edit_collection($vars, $id);
	}
	else {
		# We're done. Back to default page.
		$template_file = $overview->list_collections($vars);
	}
}



elsif (defined($state->{'confirm_delete'})) {
	my $id = $state->{'id'};
	$vars = $overview->delete_collection_confirmed($vars, $id);
	
	$template_file = $overview->list_collections($vars);
}

elsif (defined $state->{'fetch_inner'}) {
	$template_file = $overview->list_collections($vars);
	$vars->{'show_only_inner'} = 1;
}

else {
	# Show default page (list of collections)
	$template_file
	    = $overview->list_collections($vars);
}


print $cgi->header(-type => 'text/html', -expires => '-1h');

$Template::Stash::SCALAR_OPS->{escape} = sub { "\Q$_[0]\E" };
my $template = Template->new(
	{INCLUDE_PATH => './templates:./templates/overview:./templates/common',});
$template->process($template_file, $vars)
        or croak $template->error(), "\n";
