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
my $header_tpl = "Content-Type: text/plain\n"
          . "Content-disposition: Attachment; "
          . "filename=%s\n"
          . "\n";

my $utime = time();
if (my $filename = $state->{'logfile'}) {
    croak "Invalid logfile"
        unless $CONFIG{logfiles}->{$filename};

    print sprintf $header_tpl, "$filename-$utime.log";

    Page::Logs::download($filename);
}
elsif ($state->{logs_compressed}) {
    print sprintf $header_tpl, "logs-$utime.zip";
    Page::Logs::downl_all_zip();
}
else {
	print "No logfile selected.";
}
