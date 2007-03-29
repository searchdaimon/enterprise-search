#!/usr/bin/perl

  use URI::URL;
#  use strict;
  use Parse::MediaWikiDump;
  require HTML::LinkExtor;


  binmode(STDOUT, ':utf8');
  binmode(STDERR, ':utf8');
    
  my $file = shift(@ARGV) or die "must specify a Mediawiki dump file";
  my $baseurl = shift(@ARGV) or die "must specify a base url";
  my $outfile = shift(@ARGV) or die "must specify a file";

  my $pages = Parse::MediaWikiDump::Pages->new($file);
  my $page;

  open(OUT,">$outfile") or die($file);
  binmode(OUT, ':utf8');

use Text::MediawikiFormat prefix => $baseurl;

my $p = HTML::LinkExtor->new(\&cb, $baseurl);

my $count = 0;    
while(defined($page = $pages->page)) {
    	#main namespace only           
    	next unless $page->namespace eq '';
    	next unless $page->redirect eq '';
	#unless defined($page->categories);


	#print $page->title, "\n"; 
	
	#print "\tnamespace: " . $page->namespace . "\n";
	#print "\tredirect: " . $page->redirect . "\n";

	#legger til artikkelen selv
	my $wurl = $baseurl . $page->title;
	$wurl =~ s/ /_/g;
	#print "w: $wurl\n";
	printurl($wurl);
  
	#print ${ $page->text };
	
	if (!defined($page->text) ) {
		next;
	}

	#print "\n";

	my $html = wikiformat (${ $page->text });
	#print $html, "\n";

	$p->parse($html);
	
	if (($count % 1000) == 0) {
		print "$count\n";
	}

	++$count;
  }

	close(OUT);

sub cb {
     	my($tag, %links) = @_;
	#print "$tag @{[%links]}\n";

	#foreach my $i (keys %links) {
	#	print "$i -> $links{$i}\n";
	#}
	my $url = $links{'href'};
	if ($url !~ /$baseurl/) {
		my $link = new URI::URL $url;
                my $FerdigUrl = $link->abs($wurl);

		#print "cp: $FerdigUrl\n";
		printurl($FerdigUrl);
	}

 }


sub printurl {
	my $purl = shift;

	#print $purl;
	print OUT $purl, "\n";

}

