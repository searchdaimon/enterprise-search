#!/usr/bin/perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Carp;
use Data::Dumper;
use Page::Scan;
use Page::Scan::Scanning;
use Page::Scan::Process;
use Switch;

my $page = Page::Scan->new();
my %state = $page->get_state();

my $tpl_file;
my $vars = {};

my $pageScanning = Page::Scan::Scanning->new($page->get_dbh);
my $pageProcess  = Page::Scan::Process->new($page->get_dbh);

if (defined $state{'action'} || defined $state{act}) { # TODO: use act only.
	my $action = $state{'action'} || $state{act};

	switch ($action) {
		case "new" {
		    # starting new scan
		    ($vars, $tpl_file) = $pageScanning->show($vars);
		}

                case "process" {
                    # process results from a scan
                    my $scan_id = $state{'id'};
                    ($vars, $tpl_file) = $pageProcess->show(
                            $vars, 
                            $scan_id, 
                            $state{scan_added});
                }

                case "del_result" {
                    my $scan_id = $state{id};
                    $pageProcess->delete_result($vars, $scan_id);
                    $tpl_file = $page->show($vars);
                }

		else {
		    croak "unknown action $action";
		}
	}
}


if (defined $state{submit}) {
    my $button = $state{submit};


    # user is running a scan.
    if ($button->{start_scan}) {
        # This process differs from normal page views.
        # Scan result has to be shown during scan, so we'll be printing html as we go.	
        my $tpl_processing = sub { 
            my ($file, $vars) = @_;
            $page->process_tpl($file, $vars, ( 
                        no_header => 1, 
                        tpl_folders => 'scan'
                        ));
        };

        print CGI::header("text/html");
        $pageScanning->run_scan($vars, $tpl_processing, $state{scan});

        exit();
    }
}
$tpl_file = $page->show($vars)
	unless defined $tpl_file;

$page->process_tpl($tpl_file, $vars, (tpl_folders => 'scan'));
