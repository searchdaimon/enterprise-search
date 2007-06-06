#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use CGI;
use Carp;
use Sql::Sql;
use Sql::Shares;
use Sql::Connectors;
use CGI::State;
use Data::Dumper;
use Page::Add;
use Template;


my $cgi = CGI->new;
my $state = CGI::State->state($cgi);



my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();

my $add = Page::Add->new($dbh, $state);
my $vars = {};
my $template_file = "";

my %misc_opts;
$misc_opts{from_scan} = $state->{from_scan};
if ($state->{'submit_first_form'}) {
	#User submittet the first form.
	
	my $share = $state->{'share'};
	($vars, $template_file) = 
		$add->submit_first_form($vars, $share, %misc_opts);
}

elsif ($state->{'submit_second_form'}) {
	# Form "wizard" complete, add to database.
	
	my $share = $state->{'share'};
	($vars, $template_file) = $add->submit_second_form($vars, $share, %misc_opts);
}

else {
	if (defined($state->{'from_scan_result'})) {
		# User clicked add share from a scan result (scan.cgi)
		#carp Dumper($state);
		my $share = $state->{share};
		$vars->{share} = $share;
		$vars->{from_scan} = $state->{from_scan_result};
		$vars = $add->vars_from_scan($vars, $share);
	}
	($vars, $template_file) = $add->show_first_form($vars);
}

my $template  =  Template->new(
	{INCLUDE_PATH => './templates:./templates/add:./templates/common'});
print $cgi->header('text/html');
$template->process($template_file, $vars)
		or croak ("template: ", $template->error());
