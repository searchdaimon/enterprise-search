#!/usr/bin/env perl

package Setup;
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use CGI;
use Carp;
use CGI::State;
use Template;
use Data::Dumper;
use Page::Logs;
use Sql::Sql;
use Page::Setup;
use Page::Setup::Network;
use Page::Setup::Auth;
use Page::Setup::Login;
use Page::Setup::Integration;
use Common::FormFlow;
use Common::Generic qw(init_root_page);

# Init
my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/setup:./templates/common/network', 'Page::Setup');
my %state = %$state_ptr;
my $template_file;
my $header_printed;

my $pageNetwork = Page::Setup::Network->new($dbh);
my $pageAuth = Page::Setup::Auth->new($dbh);
my $pageLogin = Page::Setup::Login->new($dbh);
my $pageIntegration = Page::Setup::Integration->new($dbh);


# Process forms
if (my $id = $state{'form_id'}) { #we're coming from a form.
	my $flow = Common::FormFlow->new();
	
	# Add form, and processing sub
	$flow   ->add('login', \&process_login)
			->add('network_config', \&process_network)
			->add('license_valid', \&process_license)
			->add('auth_code_valid', \&process_manual_act)
			->add('integration_method', \&process_integration_method)
			->add('integration_values', \&process_integration_values);

	# Run current
	($vars, $template_file) = $flow->process($id);
}
elsif (my $view = $state{'view'}) {
	if ($view eq 'manual_activation') {
		my $pageAuth = Page::Setup::Auth->new($dbh);
		($vars, $template_file) = $pageAuth->show_activation_dialog($vars);
	}
}
# Print page
($vars, $template_file) = $pageLogin->show_login()
	unless (defined $template_file);

print $cgi->header('text/html') unless $header_printed;

$template->process($template_file, $vars)
        or croak $template->error() . "\n";





# Group: Form functions

sub process_login {
	my ($success, $status) 
		= $pageLogin->process_login();

	# login failed, show again.
	return $pageLogin->show_login()
		unless $success;

	# login ok.

	if ($status eq "FIRST_LOGIN") { #continue with wizard
		return $pageNetwork->show_network_config($vars);	
	}
	else { #skip wizard
		print CGI::redirect("overview.cgi");
		exit 0;
	}
}

sub process_network {

	
	my ($vars, $success) 
		= $pageNetwork->process_network_config($vars, $state{'netconf'}, $state{'resolv'});

	if ($success) {
		return $pageAuth->show_license_dialog($vars);	
	}
	else {
		return $pageNetwork->show_network_config($vars, $state{'netconf'}, $state{'resolv'});
	}
}


sub process_license {
	my ($vars, $success) = 
		$pageAuth->process_license($vars, $state{'license'});

	if ($success) {
		return $pageIntegration->show_integration_methods($vars);
	}
	else {
		return $pageAuth->show_license_dialog($vars, $state{'license'});
	}
}


sub process_manual_act {
	my ($vars, $success) = $pageAuth->process_auth_code($vars, $state{'auth_code'});

	if ($success) {
		return $pageIntegration->show_integration_methods($vars);
	}
	else {
		return $pageAuth->show_activation_dialog($vars, $state{'auth_code'});
	}
}

sub process_integration_method {
	my $method = $state{'auth_method'};

	if ($method eq 'shadow') {
		# No need to configure values, next step.
		print CGI::redirect("overview.cgi");
		exit 0;
	}
	else {
		return $pageIntegration
				->show_integration_values($vars, $state{'auth_method'});
	}
}

sub process_integration_values {
	my ($vars, $success) 
		= $pageIntegration->process_integration($vars, {
 						'domain'		=> $state{'domain'}, 
 						'user'			=> $state{'user'},
 						'password'		=> $state{'password'},
 						'ip'			=> $state{'ip'},
 						#'port'			=> $state{'port'},
 						'auth_method'	=> $state{'auth_method'}
				});

	if ($success) {
		print CGI::redirect("overview.cgi");
		exit 0;
	}
	else {
		return $pageIntegration
					->show_integration_values($vars, $state{'auth_method'}, 1);
	}

}







# 
# 	elsif ($form_id eq 'integration_method') {
# 		# User has selected methods. Show integration form
# 		# for filling out properties for given method.
# 		($vars, $template_file)
# 			= $page->show_integration_values($vars, $state{'auth_method'});
# 	}
# 	
# 	
# 	elsif ($form_id eq 'integration_values') {
# 		# User enter integration values. 
# 		#  1. Show form again if they're not valid.
# 		# or 2. Jump to scan for collections.
# # 	
# 		($vars, $template_file) 
# 			= $page->update_integration_config($vars,
# 						'domain'		=> $state{'domain'}, 
# 						'user'			=> $state{'user'},
# 						'password'		=> $state{'password'},
# 						'ip'			=> $state{'ip'},
# 						'port'			=> $state{'port'},
# 						'auth_method'	=> $state{'auth_method'},
# 		);
# 		
# 		#($vars, $template_file) = $page->show_start_scan($vars);
# 		
# 	}
# 		
# 		
# 	
# 	elsif ($form_id eq 'getting_started') {
# 		# User is coming from step 1. Start scanning the network.
# 		
# 		if (defined $state{'skip_button'}) {
# 			($vars, $template_file) = $page->show_complete_step($vars);
# 		}
# 		
# 		elsif (defined $state{'next_button'}) {
# 			# Starting quick scan on the local network.
# 			# Scanning process is shown.
# 			print $cgi->header('text/html');
# 			$header_printed = 1;
# 			$| = 1; # Don't buffer output.
# 			
# 			($vars, $template_file) = $page->show_scanning($vars, 'begin');
# 			$template->process($template_file)
# 				or croak $template->error() . "\n";
# 			
# 			$vars = $page->start_auto_scan($vars);
# 			
# 			($vars, $template_file) = $page->show_scanning($vars, 'end');
# 		}
# 	}
# 	
# 	elsif ($form_id eq 'scan_complete') {
# 		# User just finished scanning.
# 		($vars, $template_file) = $page->show_process_form($vars, $state{'result_id'});
# 	}
# 	
# 	elsif ($form_id eq 'process_form') {
# 		# User came from process form.
# 		# Add collections.
# 		if (defined $state{'next_button'}) {
# 			$vars = $page->add_collections($vars, $state{'share'}, $state{'checked'});
# 		}
# 		
# 		($vars, $template_file) = $page->show_complete_step($vars);
# 	}
# 	
# 	else {
# 		croak("Unknown form_id: ", $form_id);
# 	}
#}
