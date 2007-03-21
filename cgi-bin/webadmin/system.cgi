#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use CGI;
use Carp;
use CGI::State;
use Template;
use Page::System;
use Data::Dumper;
use Page::System::Network;

my $cgi = CGI->new;
my $state_ptr = CGI::State->state($cgi);
my %state = %$state_ptr;
print $cgi->header('text/html');

my $vars = { };
my $template = Template->new(
	{INCLUDE_PATH => './templates:./templates/system:./templates/common',});
my $template_file;

my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();
my $page = Page::System->new($dbh);
my $pageNetwork = Page::System::Network->new($dbh);

if (defined $state{'submit'}) {
	my $button = $state{'submit'};
	
	# User submitted network conf form.
	if (defined $button->{'network_conf'}) {
		$pageNetwork->process_network_config($vars, $state{'netconf'});
		($vars, $template_file) = $pageNetwork->show_network_config($vars);	
	}
}
elsif (defined $state{'view'}) {
	my $view = $state{'view'};
	
	if ($view eq "package_upload") {
		# Show the package upload page
		
		($vars, $template_file) = $page->show_upload_page($vars);
	}
	
	if ($view eq "network") {
		($vars, $template_file) = $pageNetwork->show_network_config($vars);
	}
}
elsif (defined $state{'package_upload_button'}) {
	# User is uploading a file.
	# Let him upload it, then try to install it.
	($vars, $template_file) = $page->upload_package($vars, $state{'package_file'});
}
else {
	($vars, $template_file) = $page->show_system_diagnostics($vars);
}
 	

$template->process($template_file, $vars)
        or croak $template->error() . "\n";
