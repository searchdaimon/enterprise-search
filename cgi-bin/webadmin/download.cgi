#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;;
use CGI::State;
use Page::Logs;
use config qw(%CONFIG);

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);

if (my $filename = $state->{'logfile'}) {
    croak "Invalid logfile"
        unless $CONFIG{logfiles}->{$filename};

    my $utime = time();
    print "Content-Type: text/plain\n",
          "Content-disposition: Attachment; ",
          "filename=$filename-$utime.log\n",
          "\n";

    Page::Logs::download($filename);
}
else {
	print "No logfile selected.";
}
