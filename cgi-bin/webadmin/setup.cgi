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
#use Page::Setup::Auth;
use Page::Setup::Login;
use Page::Setup::Integration;
use Common::FormFlow qw(FLOW_START_FORM);
use Digest::MD5 qw(md5_hex);

# Init
my $vars = { };
my $page = Page::Setup->new();
my %state = $page->get_state();
my $dbh = $page->get_dbh();
my $tpl_file;
#warn Dumper(\%state);

my $pageNet     = Page::Setup::Network->new($dbh);
my $pageAuth        = undef; # Page::Setup::Auth->new($dbh); fjerner til bi begynner med lisens
my $pageLogin       = Page::Setup::Login->new($dbh);
my $pageIntegr = Page::Setup::Integration->new($dbh);
    
my $flow = Common::FormFlow->new();
my $tpl_folders = ['setup', 'common/network', 'usersys'];

if (defined $state{view}) {
    # Non-wizard pages.
    my $view = $state{view};
    if ($view eq 'manual_activation') {
        ($vars, $tpl_file) 
            = $pageAuth->show_activation_dialog($vars);
    }
    elsif ($view eq 'network_restart') {
        $tpl_file = $pageNet->show_post_restart($vars, $state{id});
    }
    elsif ($view eq 'network_cfg') {
        $tpl_file = process_network();
    }
}
else {
    # Show wizard.
    my $form_submitted = $state{form_id} ||= FLOW_START_FORM;


    # The setup flow.
    # Add takes 'id for form submitted' => 'what function to run'
    $flow  
        #->add($FLOW_START_FORM, \&process_login) 
        ->add(FLOW_START_FORM,  \&process_network)
        ->add('network_config', \&process_network)
        ->add('network_restarted', sub { $pageIntegr->show($vars) })
        #->add('license_valid', \&process_license)
        #->add('manual_act', \&process_manual_act)
        ->add('integration_method', \&process_integration_method)
        ->add('integration_values', \&process_integration_values);
    $tpl_file = $flow->process($form_submitted);
}

$page->process_tpl($tpl_file, $vars, 
    tpl_folders => $tpl_folders);


# Group: Form functions

#sub process_login {
#	my ($success, $status) 
#		= $pageLogin->process_login();
#
#	# login failed, show again.
#	return $pageLogin->show_login()
#		unless $success;
#
#	# login ok.
#
#	if ($status eq "FIRST_LOGIN") { #continue with wizard
#		return $pageNet->show_network_config($vars);	
#	}
#	else { #skip wizard
#		print CGI::redirect("overview.cgi");
#		exit 0;
#	}
#}
sub process_network {
    my ($netconf, $resolv) = ($state{netconf}, $state{resolv});

    return $flow->process_next()
        if $state{skip};

    if (defined $netconf and defined $resolv) {
        my ($tpl_file, $restart_id) 
            = $pageNet->show_restart($vars, $netconf);

        # Page needs to be shown before network
        # is restarted.
        $page->process_tpl($tpl_file, $vars, tpl_folders => $tpl_folders);

        $pageNet->run_updates($restart_id,
            $netconf, $resolv);
        exit;
    }

    return $pageNet->show_network_config(
        $vars, $netconf, $resolv);
}


sub process_license {
	my ($vars, $success) = 
		$pageAuth->process_license($vars, $state{'license'});

	if ($success) {
		return $pageIntegr->show_integration_methods($vars);
	}
	else {
		return $pageAuth->show_license_dialog($vars, $state{'license'});
	}
}


sub process_manual_act {
	my ($vars, $success) = $pageAuth->process_signature($vars, $state{'license'}, 
											$state{'hardware'}, $state{'signature'});

	if ($success) {
		return $pageIntegr->show_integration_methods($vars);
	}
	else {
		return $pageAuth->show_activation_dialog($vars, $state{'license'}, $state{'signature'});
	}
}

sub process_integration_method {
	return $pageIntegr->show_integration_values(
		$vars, 
		$state{auth_connector}, 
		$state{sys_name}
	);
}

sub process_integration_values {
	my $tpl_file = $pageIntegr->process_integration($vars, $state{sys});
	return $tpl_file if $tpl_file;
 	# $tpl_file is undef if all is OK.

	my $cfg = Sql::Config->new($dbh);
	#setter at wixarden er gjort.
	$cfg->insert_setting("setup_wizard_done", 1);
	#oppdaterer key verdien, slik at vi får en unik nøkker. Dette må gjøres her, da vi på vmware ikke instalerer databasen på ny for hver gang. 
	$cfg->update_setting("key", md5_hex(rand()) );

	print CGI::redirect("overview.cgi?from_setup=1");
	exit 0;
}

