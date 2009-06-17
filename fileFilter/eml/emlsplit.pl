#!/usr/bin/perl

use warnings;
use strict;

BEGIN {
	push @INC, $ENV{BOITHOHOME}."/crawlers/Modules/";
}

#use Email::Simple;
use Email::MIME;
use Data::Dumper;
use sdMimeMap;

# A bit modified list stolen from mime.types in ubuntu

my $fileemail;

my $dirfiltername = "/tmp/dirfilter-" . $< . "/";

$fileemail = shift @ARGV or die "Usage: ./emlsplit.pl emlfile" ;

#debug: lagrer filen slik at vi har .eml filene.
#system("cp $fileemail /tmp/dirfilter-rb-tmp/");

my $message;
{
	local(*FH);
	open(FH, $fileemail) or die "Unable to open file\n";
	$message = do { local($/) ; <FH> } ;
}

my $parsed = Email::MIME->new($message);

#runarb: 01.11.07 må lage dirfilter mappen hvis vi ikke har
if (!(-e "$dirfiltername")) {
	mkdir("$dirfiltername") or die("mkdir $dirfiltername");
}

my $filenames = '';

sub charset_from_header {
	my ($header) = @_;

	my @pairs = split(/\s+/, $header);
	foreach my $pair (@pairs) {
		my ($key, $value) = split("=", $pair);

		if ($key eq 'charset') {
			return $value;
		}
	}

	return undef;
}

sub writemail {
	my ($p) = @_;

	my @parts = $p->parts;

	foreach (@parts) {
		my $fn;

		if (defined $_->content_type and $_->content_type =~ /^multipart\//) {
			my @sub = $_->subparts;
			foreach my $subp (@sub) {
				writemail($subp);
			}
			next;	
		}

		#my $charset = charset_from_header($_->header("Content-Type"));
		#print "Got charset: $charset\n";
		#print Dumper($_->header("Content-Type"));
		#print Dumper($_);

		my $filename = $_->filename;
		if (defined $filename) {
			$filenames .= $filename . "\n";
		}
		
		my ($ct) = defined $_->content_type ? split(";", $_->content_type) : "text/txt";
		$fn = "$dirfiltername".$_->invent_filename($ct);
		my $suffix;
		$suffix = mapMimeType($ct);
		$suffix = 'dat' if $suffix eq '';
		if ($ct eq 'application/octet-stream') {
			if (defined($_->{ct}) && defined($_->{ct}->{attributes}) &&
			    defined($_->{ct}->{attributes}{name}) && $_->{ct}->{attributes}{name} =~ /\.(\w+)$/) {
				$suffix = $1;
				$fn .= ".$suffix";
			}
		}
		print ($suffix ." ".$fn."\n");
		open(my $wf, "> $fn") || die "$fn: $!";
		eval {
			print $wf $_->body;
		};
		close $wf;
	}
}

writemail($parsed);

my $headername = "$dirfiltername".$parsed->invent_filename.".header.";
open(my $mh, "> $headername") or die "$!: $headername";

print $mh $filenames . "\n";

foreach my $hn (qw(Subject From To Cc Bcc Reply-to Date)) {
	my @values;
	eval {
		@values = $parsed->header($hn);
	} or next;
	
	next if (length(@values) == 0);
	print $mh "$hn:";
	print $mh join(", ", @values);
	print $mh "\n";
}
close($mh);


print ("txt" ." ".$headername."\n");

use Date::Parse;

if (defined($ENV{SDMETAFILE}))  {
	# Write some data to the metaspec file
	my $metafile = $ENV{SDMETAFILE};
	open(FH, "> $metafile");
	my @values = $parsed->header("Date");
	if (defined($values[0])) {
		print FH "lastmodified = " . str2time($values[0]) . "\n";
	}
	close(FH);
}
