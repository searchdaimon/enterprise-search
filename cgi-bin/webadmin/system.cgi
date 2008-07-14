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
use Switch;

use Page::System::Services;
use Page::System::Crashes;

my $vars = { };
my $page = Page::System->new();
my %state = $page->get_state();
my $template_file;


my $pageServices = Page::System::Services->new($page->get_dbh);
my $pageCrashes  = Page::System::Crashes->new($page->get_dbh);


if (defined $state{'submit'}) { #POST actions
	my $button = $state{'submit'};	
	

	# User wants to see siez for all lots.
	if (defined $button->{'show_lot_size'}) {
		($vars, $template_file) = $page->show_system_diagnostics($vars, {'show_lot_usage' => 1});
	}

	# USe wants to send a crash report.
	if (defined $button->{'crash_send_report'}) {
		($vars, $template_file) = $pageCrashes->send_report($vars, $state{'core_file'}, 
			$state{'core_time'}, $state{'report'})
	}
}

elsif (defined $state{'view'}) {
	my $view = $state{'view'};
	
	switch ($view) {
		case "services" {
			($vars, $template_file) = $pageServices->show($vars);
		}

		case "crashes" {
			($vars, $template_file) = $pageCrashes->show($vars);
		}

		case "crash_report" {
			($vars, $template_file) = $pageCrashes->show_report($vars, $state{'core'});
		}
                default { croak "unknown view '$view'" }
	}
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

$page->process_tpl($template_file, $vars, 
    tpl_folders => ['system', 'common/network']);
