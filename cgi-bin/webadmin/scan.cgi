#!/usr/bin/perl
use strict;
use warnings;
use Template;
use CGI;
use Carp;
use Sql::Sql;
use Sql::Connectors;
use Sql::Shares;
use Sql::CollectionAuth;
use Sql::ScanResults;
use Common::Scan;
use CGI::State;
use Data::Dumper;
use Common::Generic;

#TODO: Create a page class for scan.

use config;
my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');


my $template = Template->new(
	{INCLUDE_PATH => './templates:./templates/scan:./templates/common'});
my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();
my $scan = Common::Scan->new($dbh);
my $common = Common::Generic->new($dbh);
my $vars = { };
my $template_file = "scan_main.html";

if (defined($state->{'delete_result'})) {
	# User is pressed some delete result button
	my $id = $common->request($state->{'delete_result'});
	my $sqlResults = Sql::ScanResults->new($dbh);
	$sqlResults->delete_result($id);
	$vars->{'success_delete_result'} = 1;
}

if (defined($state->{'start_scan'})) {
	$template->process('scan_top.html');
	my $result_id = $scan->scan_start($state->{'scan'});
	$vars->{'result_id'} = $result_id;
	$template_file = "scan_bottom.html";
}

elsif (defined($state->{'see_results'})) {
	# Scan done. Show found shares.
	my @ids = ();
	# Remap input into hash
	my %found;
	map { 
		if ($_) {
			my $id = $_->{'id'};
			my $status = $_->{'status'};
			$found{$id} = $status; 
		}
	} @{$state->{'found'}};

	my $shares = $scan->get_found_shares([keys %found]);
	
	my (@new, @old);
	# Sort into new and existing sources
	for my $i (0..$#{@$shares}) {
		my $id = $shares->[$i]{'id'};
		my $status = $found{$id};
		push @old, $shares->[$i] if ($status eq 'old');
		push @new, $shares->[$i] if ($status eq 'new');
	}
	my $sqlConnectors = Sql::Connectors->new($dbh);
	my $connectors = $sqlConnectors->get_connectors();
	$template->process('scan_results.html', {
						'old' => \@old,
						'new' => \@new,
						'connectors' => $connectors })
		or croak $template->error();
}


elsif (defined($state->{'action'})) {
	my $action = $state->{'action'};

	if ($action eq 'new') {
		# Show scan form.
		my $sqlConnectors = Sql::Connectors->new($dbh);
		my $sqlAuth = Sql::CollectionAuth->new($dbh);
		my $authentication = $sqlAuth->get_authentication;
		$vars->{'connectors'} = $sqlConnectors->get_connectors_with_scantool();
		$vars->{'authentication'} = $sqlAuth->get_authentication;
		$template_file = "scan_new.html";

	}

	if ($action eq 'process') {
		# Show process form for a given result
		my $id = $state->{'id'};
		my ($results, $connector) = $scan->process($id);
		if ($results and $connector) {
			$vars->{'results'} = $results;
			$vars->{'connector'} = $connector;
			$vars->{'id'} = $id;
		}
		
		$template_file = "scan_results.html";
	}
}
else {
	# Show default page	
	my $sqlResults = Sql::ScanResults->new($dbh);
	$vars->{'results'} = $sqlResults->get_list_and_connector_name;
}




$template->process($template_file, $vars)
 		or croak $template->error();