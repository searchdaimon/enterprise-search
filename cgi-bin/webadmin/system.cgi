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

use Page::System::Services;
use Page::System::Packages;
use Common::Generic qw(init_root_page);

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/system:./templates/common/network', 'Page::System');

my %state = %{$state_ptr};
my $template_file;


my $pageServices = Page::System::Services->new($dbh);
my $pagePackages = Page::System::Packages->new($dbh);


if (defined $state{'submit'}) { #POST actions
	my $button = $state{'submit'};	
	

	# User wants a list of available updates..
	if (defined $button->{'software_check_updates'}) {
	    ($vars, $template_file) = $pagePackages->show($vars, "SHOW_AVAILABLE");
	}

	# User wants to update software on the black box.
	if (defined $button->{'software_install_available'}) {
		($vars, $template_file) = $pagePackages->update_packages($vars);
	}

	# User wants to install manually uploaded packages
	if (defined $button->{'software_install_uploaded'}) {
		($vars, $template_file) = $pagePackages->install_uploaded($vars);
	}

	# User wants to see siez for all lots.
	if (defined $button->{'show_lot_size'}) {
		($vars, $template_file) = $page->show_system_diagnostics($vars, {'show_lot_usage' => 1});
	}
}

elsif (defined $state{'view'}) {
	my $view = $state{'view'};
	
	if ($view eq "updates") {
		# Show the package upload page
		
		($vars, $template_file) = $pagePackages->show($vars);
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

	elsif ($action eq "pkg_remove") {
		# User wants to remove a uploaded package.
		
		my $pkg = $state{'pkg'};
		($vars, $template_file)  #todo chekc if rpm
			= $pagePackages->remove_uploaded_package($vars, $pkg);
	}
}


else {
	($vars, $template_file) = $page->show_system_diagnostics($vars);
}
 	
print $cgi->header('text/html');
$template->process($template_file, $vars)
        or croak $template->error() . "\n";
