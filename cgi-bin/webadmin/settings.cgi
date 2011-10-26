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

my $page = Page::Settings->new;
my $tpl_folders = ['common/network', 'settings'];
my %state = $page->get_state();
my $tpl_file;
my $vars = {};

my $pageNet = Page::Settings::Network->new($page->get_dbh);
my $pageCM = Page::Settings::CollectionManager->new($page->get_dbh);

my $using_api = 0;
my %api_vars;
#carp Dumper(\%state);
if ($state{api}) {
	my $api = $state{api};
	$using_api = 1;

	if ($api eq "check_license") {
		$page->api_check_license(\%api_vars, $state{license});
	}
	else { croak "Unknown api call '$api'" }
}
elsif (defined($state{'submit'})) {
	my $btn = $state{'submit'};

	if (defined $btn->{'network_conf'}) {
            my ($tpl_file, $restart_id) 
                = $pageNet->show_restart($vars, $state{netconf});
   
            $page->process_tpl($tpl_file, $vars, tpl_folders => $tpl_folders);
            $pageNet->run_updates($restart_id, $state{netconf}, $state{resolv});
            exit;
        }

	elsif (defined $btn->{'reset_configuration'}) {
		# User wants to reset configuration. Confirm.
		$tpl_file = $page->show_confirm_dialog($vars);
	}

	elsif (defined $btn->{'submit_settings'}) {
		# Update config values, show success message.
		$vars = $page->update_settings($vars, $state{'setting'});
		$tpl_file = $page->show_advanced_settings_updated($vars);
	}

	elsif (defined $btn->{'export_settings'}) {
		# User is downloading exported settings
                my $utime = time();
		my $export_file = $page->export_settings();
		open my $fh, "<", $export_file
			or croak "export file open: ", $!;

                print "Content-Type: application/x-gzip\n",
                      "Content-disposition: Attachment; ",
                      "filename=bbexport-$utime.backup\n",
                      "\n";
		print while <$fh>;
		close $fh;

		exit 0;
	}

	elsif (defined $btn->{'import_settings'}) {
		# User is importing a file.
		$tpl_file = $page->import_settings($vars, $state{import_file});
	}

	elsif (defined $btn->{'dist_select'}) {
		# User selected a different version from main settings
		$tpl_file 
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
	elsif (defined $btn->{update_license}) {
		$tpl_file = $page->update_license($vars, $state{license_key});
	}

	else { croak "Unknown submit action" }
}

elsif (defined($state{'confirm_delete'})) {
        croak "Reset all settings disabled.";
    
	# User confirmed delete. Delete settings
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	
	$tpl_file = $page->confirmed_delete_settings($vars);
	
}




# Group: Views

elsif (defined $state{'view'}) {
	my $view = $state{'view'};
	
	if ($view eq "import_export") {
		$tpl_file = $page->show_import_export($vars);
	}

	elsif ($view eq "advanced") {
		$tpl_file = $page->show_advanced_settings($vars);
	}

        elsif ($view eq "network_restart") {
            $tpl_file = $pageNet->show_post_restart($vars, $state{restart});
        }

	elsif ($view eq "network") {
		$tpl_file = $pageNet->show_network_config($vars);
	}

        elsif ($view eq "collection_manager") {
            $tpl_file = $pageCM->show($vars);
        }
}


if (!$using_api && !$tpl_file) {
	$tpl_file  = $page->show_main_settings($vars);

}
if ($using_api) {
    	my $json = JSON::XS->new;
    	$json->pretty(1);
	print $page->get_cgi()->header(-type => 'text/plain', -charset => "UTF-8");

	print $json->encode(\%api_vars);
}
else {
	$page->process_tpl($tpl_file, $vars, 
    		tpl_folders => $tpl_folders);
}
