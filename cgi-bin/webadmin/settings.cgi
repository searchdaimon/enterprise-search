#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;
use Sql::Sql;
use CGI::State;
use Template;
use Data::Dumper;
use Page::Settings;

my $cgi = CGI->new;
my $state_ptr = CGI::State->state($cgi);
my %state = %$state_ptr;


my $vars;
my $template = Template->new({INCLUDE_PATH => './templates:./templates/settings:./templates/common',});

my $sql = Sql::Sql->new;
my $dbh = $sql->get_connection();
my $page = Page::Settings->new($dbh);

my $template_file = 'settings_main.html';


if (defined($state{'reset_configuration'})) {
	# User wants to delete. Confirm it.
	($vars, $template_file) = $page->show_confirm_dialog($vars);
}
elsif (defined($state{'confirm_delete'})) {
	# User confirmed delete. Delete settings
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	
	($vars, $template_file) = $page->confirmed_delete_settings($vars);
	
}

elsif (defined($state{'submit_settings'})) {
	# Update config values, show success message.
	$vars = $page->update_settings($vars, $state{'setting'});
	($vars, $template_file) 
		= $page->show_advanced_settings_updated($vars);
}

elsif (defined($state{'export_settings'})) {
	# User is downloading exported settings
	print $cgi->header('text/plain');
	print $page->export_settings();
	exit 0;

}

elsif (defined($state{'view'})) {
	my $view = $state{'view'};
	
	if ($view eq "import_export") {
		($vars, $template_file) = $page->show_import_export($vars);
	}

	elsif ($view eq "advanced") {
		($vars, $template_file) = $page->show_advanced_settings($vars);
	}
}

elsif (defined($state{'import_button'})) {
	# User is importing a file.
	($vars, $template_file) = $page->import_settings($vars, $cgi->param("import_file"));
}

elsif (defined($state{'dist_select'})) {
	# User selected a different version from main settings
	($vars, $template_file) 
		= $page->select_dist_version($vars, $state{'dist'});
}

else { 
	# Show main page.
	($vars, $template_file) 
		= $page->show_main_settings($vars);
}


print $cgi->header('text/html');
$template->process($template_file, $vars)
        or croak $template->error() . "\n";
