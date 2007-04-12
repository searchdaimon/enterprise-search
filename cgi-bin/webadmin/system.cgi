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
use Page::System::Services;
use Page::System::Packages;
use Common::Generic qw(init_root_page);

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/system', 'Page::System');

my %state = %{$state_ptr};
my $template_file;

my $pageNetwork  = Page::System::Network->new($dbh);
my $pageServices = Page::System::Services->new($dbh);
my $pagePackages = Page::System::Packages->new($dbh);


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
		
		($vars, $template_file) = $pagePackages->show($vars);
	}
	
	elsif ($view eq "network") {
		($vars, $template_file) = $pageNetwork->show_network_config($vars);
	}

	elsif ($view eq "services") {
		($vars, $template_file) = $pageServices->show($vars);
	}
}
elsif (defined $state{'package_upload_button'}) {
	# User is uploading a file.

	($vars, $template_file) = $pagePackages->upload_package($vars, $state{'package_file'});
}

elsif (defined $state{'action'}) {
	my $action = $state{'action'};
	my $service = $state{'name'};

	if (grep { /^$action$/ } "start_service", "stop_service", "restart_service") {
		my %params = (
			"start_service" => "start",
			"stop_service"  => "stop",
			"restart_service" => "restart");


		($vars, $template_file) = $pageServices->action($vars, $service, $params{$action});
	}
}


else {
	($vars, $template_file) = $page->show_system_diagnostics($vars);
}
 	
print $cgi->header('text/html');
$template->process($template_file, $vars)
        or croak $template->error() . "\n";
