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
use Page::Settings::Network;
use Page::Settings::CrawlManager;
use Common::Generic qw(init_root_page);

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/settings:./templates/common/network', 'Page::Settings');

my %state = %{$state_ptr};
my $template_file;

my $pageNetwork = Page::Settings::Network->new($dbh);
my $pageCM = Page::Settings::CrawlManager->new($dbh);


# Group: User actions

if (defined($state{'submit'})) {
	my $button = $state{'submit'};

	# User submitted network conf form.
	if (defined $button->{'network_conf'}) {
		$pageNetwork->process_network_config($vars, $state{'netconf'}, $state{'resolv'});
		($vars, $template_file) = $pageNetwork->show_network_config($vars);	
	}

	elsif (defined $button->{'reset_configuration'}) {
		# User wants to reset configuration. Confirm.
		($vars, $template_file) = $page->show_confirm_dialog($vars);
	}

	elsif (defined $button->{'submit_settings'}) {
		# Update config values, show success message.
		$vars = $page->update_settings($vars, $state{'setting'});
		($vars, $template_file) 
			= $page->show_advanced_settings_updated($vars);
	}

	elsif (defined $button->{'export_settings'}) {
		# User is downloading exported settings
		print $cgi->header('text/plain');
		print $page->export_settings();
		exit 0;
	}

	elsif (defined $button->{'import_settings'}) {
		# User is importing a file.
		($vars, $template_file) 
			= $page->import_settings($vars, $cgi->param("import_file"));
	}

	elsif (defined $button->{'dist_select'}) {
		# User selected a different version from main settings
		($vars, $template_file) 
			= $page->select_dist_version($vars, $state{'dist'});
	}

        elsif (defined $button->{admin_pass}) {
            # User is changing passwords
            $template_file = $page->update_admin_passwd($vars, $state{passwd});
        }

        elsif (defined $button->{cm_gc}) {
            # User update GC rate.
            $template_file = $pageCM->update_gc($vars, $state{cm}{gc_rate});
        }
        elsif (defined $button->{cm_schedule}) {
            $template_file = $pageCM->update_schedule(
                $vars, $state{cm}{use_schedule}, 
                $state{cm}{schedule_start}, $state{cm}{schedule_end});
        }
}

elsif (defined($state{'confirm_delete'})) {
	# User confirmed delete. Delete settings
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	
	($vars, $template_file) = $page->confirmed_delete_settings($vars);
	
}




# Group: Views

elsif (defined($state{'view'})) {
	my $view = $state{'view'};
	
	if ($view eq "import_export") {
		($vars, $template_file) = $page->show_import_export($vars);
	}

	elsif ($view eq "advanced") {
		($vars, $template_file) = $page->show_advanced_settings($vars);
	}

	elsif ($view eq "network") {
		($vars, $template_file) = $pageNetwork->show_network_config($vars);
	}

        elsif ($view eq "crawl_manager") {
            $template_file = $pageCM->show($vars);
        }
}


unless (defined $template_file) {
	# Show main page.
	($vars, $template_file) 
		= $page->show_main_settings($vars);

}


print $cgi->header('text/html');
$template->process($template_file, $vars)
        or croak $template->error() . "\n";
