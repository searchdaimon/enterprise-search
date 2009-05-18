#!/usr/bin/perl

use warnings;
use strict;

BEGIN {
	push @INC, $ENV{BOITHOHOME}."/crawlers/Modules/";
}


#use Email::Simple;
use Email::MIME;
use Date::Parse;
use Readonly;
use File::Temp;
use Carp;
use Data::Dumper;
use sdMimeMap;

our $DIRFILTER_TPL = "/tmp/dirfilter-XXXXXXXXX";


sub fetch_attachments {
	my ($p, $fileprefix, $fh) = @_;

	my @attachments;
	my @parts = $p->parts;

	foreach (@parts) {
		my $fn;
		if ($_->content_type =~ /^multipart\//) {
			my @sub = $_->subparts;
			foreach my $subp (@sub) {
				push @attachments, fetch_attachments($subp, $fileprefix, $fh);
			}
			next;	
		}

		my $filename = $_->filename;
		print $fh "$filename\n" if $filename;

		my ($ct) = split(";", $_->content_type);
		$fn = $fileprefix . $_->invent_filename($ct);
		my $suffix;
		$suffix = sdMimeMap::mapMimeType($ct);
		$suffix = 'dat' if $suffix eq '';
		if ($ct eq 'application/octet-stream') {
			if (defined($_->{ct}) && defined($_->{ct}->{attributes}) &&
			    defined($_->{ct}->{attributes}{name}) && $_->{ct}->{attributes}{name} =~ /\.(\w+)$/) {
				$suffix = $1;
				$fn .= ".$suffix";
			}
		}
		push @attachments, [$suffix, $fn];
		open(my $wf, "> $fn") || die "$fn: $!";
		print $wf $_->body;
		close $wf;
	}

	return @attachments;
}



sub dump {
	#croak Dumper(\@_);
	my (undef, $params) = @_;

	my $fileemail = $params->{file};
	my $fileprefix = mktemp($DIRFILTER_TPL);

#debug: lagrer filen slik at vi har .eml filene.
#system("cp $fileemail /tmp/dirfilter-rb-tmp/");

	my $message;
	{
		open my $fh, "<", $fileemail 
			or croak "Unable to open file '$fileemail'";
		local $/;
		$message = <$fh>;
		close $fh;
	}

	my $parsed = Email::MIME->new($message);


	my $headername =  $fileprefix . $parsed->invent_filename . ".header.";
	open my $mh, ">", $headername 
		or die "$!: '$headername'";

	my @attachments = fetch_attachments($parsed, $fileprefix, $mh);

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
	close $mh;
	
	my $file_list_str = '';
	for my $a (@attachments) {
		my ($ext, $filepath) = @{$a};
		$file_list_str .= "$ext $filepath\n";
	}
	$file_list_str .= "txt $headername\n";



	# Add to data reference
	${$params->{data}} = $file_list_str;

	# Add metadata
	if (my ($date) = $parsed->header("Date")) {
		$params->{metadata}{lastmodified} = str2time($date);
	}

	return 1;
}
