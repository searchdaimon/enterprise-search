#!/usr/bin/perl
use strict;
use warnings;
use Template;
use CGI;
use Carp;
use Page::Login;
use Sql::Config;
use Sql::Sql;


my $cgi = CGI->new();

my $vars = { };
my $template_file = "login.html";
my $template = Template->new({
	INCLUDE_PATH => './templates/login:./templates/common',
});
my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();
#my $sqlConfig = Sql::Config->new($dbh);
#my $login = Page::Login->new($c, $sqlConfig);

my $state_ptr = CGI::State->state($cgi);
my %state = %$state_ptr;
print Dumper($state_ptr);





unless ($state{'form_id'}) {
	# Nothing has been submitted. Show login form.
	
}
else {
	my $form_id = $state{'form_id'};
	
	if ($form_id eq 'login') {
		unless $sqlConfig->config_exists {
			# First login. Start first-time-wizard.
			
			#TODO: Implement.
		}
		else {
			$cgi->redirect("overview.cgi");
			print '<a href="overview.cgi">overview</a>';
		}
	}
	elsif ($form_id eq 'integration') {
		# User is selecting what system to integrate with.
		my $auth_method = $state{'auth_method'};
		my $auth_config = $state{'auth_config'};
		
		unless ($auth_config or ($auth_method eq 'shadow')) {
			# Missing [what?]. Show form again.
		}
		
		else {
			# Update config ($login->update_config($auth))
			# Next step in wizard.
		}
		
		
	}

# elsif ($last_form eq 'auth') {
# 	unless ($auth_config or ($auth_method eq 'shadow')) {
# 		$vars->{'show_dap_config'} = $auth_method;
# 		$vars->{'show_security_form'} = 1;
# 	}

# 	else { 
# 		my $auth;
# 		if ($auth_method eq 'shadow') {
# 			$auth = 'shadow';
# 		}
# 		else { $auth = $auth_config; }
# 		$login->update_config($auth);
# 		print "Location: overview.cgi\n\n";
# 		print "Configuration updated..<br />";
# 		print "You're logged in. Continue to <a href=\"overview.cgi\">overview</a>.";
# 	} 
# }
print $c->header("text/html");
$template->process($template_file, $vars)
        or croak $template->error() . "\n";

# print "parameters: <br />";
# foreach($c->param) { print "$_ -> " . $c->param($_) . "<br />"; }
