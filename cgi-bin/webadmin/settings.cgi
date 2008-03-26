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
use Page::Settings::CollectionManager;
use Common::Generic qw(init_root_page);

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/settings:./templates/common/network', 'Page::Settings');

my %state = %{$state_ptr};
my $tpl_file;

my $pageNetwork = Page::Settings::Network->new($dbh);
my $pageCM = Page::Settings::CollectionManager->new($dbh);



# Group: User actions

if (defined($state{'submit'})) {
	my $btn = $state{'submit'};

	# User submitted network conf form.
	if (defined $btn->{'network_conf'}) {
		$pageNetwork->process_network_config($vars, $state{'netconf'}, $state{'resolv'});
		($vars, $tpl_file) = $pageNetwork->show_network_config($vars);	
	}

	elsif (defined $btn->{'reset_configuration'}) {
		# User wants to reset configuration. Confirm.
		($vars, $tpl_file) = $page->show_confirm_dialog($vars);
	}

	elsif (defined $btn->{'submit_settings'}) {
		# Update config values, show success message.
		$vars = $page->update_settings($vars, $state{'setting'});
		($vars, $tpl_file) 
			= $page->show_advanced_settings_updated($vars);
	}

	elsif (defined $btn->{'export_settings'}) {
		# User is downloading exported settings
                my $utime = time();
                print "Content-Type: text/plain\n",
                      "Content-disposition: Attachment; ",
                      "filename=bbexport-$utime.backup\n",
                      "\n";
		print $page->export_settings();
		exit 0;
	}

	elsif (defined $btn->{'import_settings'}) {
		# User is importing a file.
		($vars, $tpl_file) 
			= $page->import_settings($vars, $cgi->param("import_file"));
	}

	elsif (defined $btn->{'dist_select'}) {
		# User selected a different version from main settings
		($vars, $tpl_file) 
			= $page->select_dist_version($vars, $state{'dist'});
	}

        elsif (defined $btn->{admin_pass}) {
            # User is changing passwords
            $tpl_file = $page->update_admin_passwd($vars, $state{passwd});
        }

        elsif (defined $btn->{cm_gc}) {
            # User update GC rate.
            $tpl_file = $pageCM->update_gc($vars, $state{cm}{gc_rate});
        }
        elsif (defined $btn->{cm_schedule}) {
            $tpl_file = $pageCM->update_schedule(
                $vars, $state{cm}{use_schedule}, 
                $state{cm}{schedule_start}, $state{cm}{schedule_end});
        }


        elsif (defined $btn->{cm_suggdict}) {
            $tpl_file = $pageCM->update_suggdict($vars, $state{cm}{suggdict_run_hour});
        }
}

elsif (defined($state{'confirm_delete'})) {
        croak "Reset all settings disabled.";
    
	# User confirmed delete. Delete settings
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	
	($vars, $tpl_file) = $page->confirmed_delete_settings($vars);
	
}




# Group: Views

elsif (defined($state{'view'})) {
	my $view = $state{'view'};
	
	if ($view eq "import_export") {
		($vars, $tpl_file) = $page->show_import_export($vars);
	}

	elsif ($view eq "advanced") {
		($vars, $tpl_file) = $page->show_advanced_settings($vars);
	}

	elsif ($view eq "network") {
		($vars, $tpl_file) = $pageNetwork->show_network_config($vars);
	}

        elsif ($view eq "collection_manager") {
            $tpl_file = $pageCM->show($vars);
        }
}


unless (defined $tpl_file) {
	# Show main page.
	($vars, $tpl_file) 
		= $page->show_main_settings($vars);

}


print $cgi->header('text/html');
$template->process($tpl_file, $vars)
        or croak $template->error() . "\n";
