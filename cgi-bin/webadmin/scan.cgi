#!/usr/bin/perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Template;
use CGI;
use Carp;
use CGI::State;
use Data::Dumper;
use Common::Generic qw(init_root_page);
use Page::Scan;
use Page::Scan::Scanning;
use Page::Scan::Process;
use Switch;

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/scan:./templates/common', 'Page::Scan');
my %state = %$state_ptr;
my $template_file;
my $common = Common::Generic->new();

my $pageScanning = Page::Scan::Scanning->new($dbh);
my $pageProcess  = Page::Scan::Process->new($dbh);

#carp Dumper($state_ptr);


if (defined $state{'action'}) {
	my $action = $state{'action'};

	switch ($action) {
		case "new" {
			# starting new scan
			($vars, $template_file) = $pageScanning->show($vars);
		}

		case "process" {
			# process results from a scan
			my $scan_id = $state{'id'};
			($vars, $template_file) = $pageProcess->show($vars, $scan_id, $state{scan_added});
		}

		else {
			croak "unknown action $action";
		}
	}
}


if (defined $state{submit}) {
	my $button = $state{submit};

	# user is deleting scan result
	if ($button->{delete_result}) {
		my $scan_id = $common->request($button->{delete_result});
		$vars = $pageProcess->delete_result($vars, $scan_id);
		($vars, $template_file) = $page->show($vars);
	}

	# user is running a scan.
	elsif ($button->{start_scan}) {
		# This process differs from normal page views.
		# Scan result has to be shown during scan, so we'll be printing html as we go.	

		print $cgi->header("text/html");
		$pageScanning->run_scan($vars, $template, $state{scan});

		exit();
	}
}
#my $scan = Common::Scan->new($dbh);
#my $common = Common::Generic->new($dbh);

# 
# 
# if (defined($state->{'delete_result'})) {
# 	# User is pressed some delete result button
# 	my $id = $common->request($state->{'delete_result'});
# 	my $sqlResults = Sql::ScanResults->new($dbh);
# 	$sqlResults->delete_result($id);
# 	$vars->{'success_delete_result'} = 1;
# }
# 
# if (defined($state->{'start_scan'})) {
# 	$template->process('scan_top.html');
# 	my $result_id = $scan->scan_start($state->{'scan'});
# 	$vars->{'result_id'} = $result_id;
# 	$template_file = "scan_bottom.html";
# }
# 
# elsif (defined($state->{'see_results'})) {
# 	# Scan done. Show found shares.
# 	my @ids = ();
# 	# Remap input into hash
# 	my %found;
# 	map { 
# 		if ($_) {
# 			my $id = $_->{'id'};
# 			my $status = $_->{'status'};
# 			$found{$id} = $status; 
# 		}
# 	} @{$state->{'found'}};
# 
# 	my $shares = $scan->get_found_shares([keys %found]);
# 	
# 	my (@new, @old);
# 	# Sort into new and existing sources
# 	for my $i (0..$#{@$shares}) {
# 		my $id = $shares->[$i]{'id'};
# 		my $status = $found{$id};
# 		push @old, $shares->[$i] if ($status eq 'old');
# 		push @new, $shares->[$i] if ($status eq 'new');
# 	}
# 	my $sqlConnectors = Sql::Connectors->new($dbh);
# 	my $connectors = $sqlConnectors->get_connectors();
# 	$template->process('scan_results.html', {
# 						'old' => \@old,
# 						'new' => \@new,
# 						'connectors' => $connectors })
# 		or croak $template->error();
# }
# 
# 
# if () {}
# else {
# 	# Show default page	
# 	my $sqlResults = Sql::ScanResults->new($dbh);
# 	$vars->{'results'} = $sqlResults->get_list_and_connector_name;
# }


($vars, $template_file) = $page->show($vars)
	unless defined $template_file;

print $cgi->header("text/html");

$template->process($template_file, $vars)
 		or croak $template->error();
