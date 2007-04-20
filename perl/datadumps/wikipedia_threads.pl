#!/usr/bin/perl

  use URI::URL;
#  use strict;
  use Parse::MediaWikiDump;
  require HTML::LinkExtor;
  use threads; 
  use Thread::Queue;


  binmode(STDOUT, ':utf8');
  binmode(STDERR, ':utf8');
    
  my $file = shift(@ARGV) or die "must specify a Mediawiki dump file";
  my $baseurl = shift(@ARGV) or die "must specify a base url";
  my $outfile = shift(@ARGV) or die "must specify a file";

  my $nrofthreads = 1;

  my $pages = Parse::MediaWikiDump::Pages->new($file);
  my $page;

  my $fileacces : shared = 0;

  open(OUT,">$outfile") or die($file);
  binmode(OUT, ':utf8');

  my $DataQueue = Thread::Queue->new;

for my $i (1..$nrofthreads) {
	print "Starting $i\n";
	$thr[$i] = threads->new(workthread);
}

my $count = 0;    
while(defined($page = $pages->page)) {
    	#main namespace only          

	#print $page->title, "\n"; 

	#print "namespace: " . $page->namespace . "\n";
	#print "redirect: " . $page->redirect . "\n";
 
    	next unless $page->namespace eq '';
	#next unless defined($page->namespace);
    	#next unless $page->redirect eq '';
	next if defined($page->redirect);
	#unless defined($page->categories);

	#print "aa\n";
	#print $page->title, "\n"; 
		
	#print "\tnamespace: " . $page->namespace . "\n";
	#print "\tredirect: " . $page->redirect . "\n";

	#legger til artikkelen selv
	my $wurl = $baseurl . $page->title;
	$wurl =~ s/ /_/g;
	#print "w: $wurl\n";
	printurl($wurl);
  
	
	if (!defined($page->text) ) {
		next;
	}


	$DataQueue->enqueue(${ $page->text });

	
	if (($count % 1000) == 0) {
		print "$count\n";
	}

	++$count;
  }

	close(OUT);

for my $i (0..$nrofthreads) {
	$thr[$i]->join;
}


sub workthread {

	#use Text::MediawikiFormat prefix => $baseurl;

	#my $p = HTML::LinkExtor->new(\&cb, $baseurl);

	while ($text = $DataQueue->dequeue) { 
            	#print "Popped one off the queue\n";
	
		#my $html = wikiformat ($DataElement);

		#$p->parse($html);
        	my @urls = ();
        	while ($text =~ m/\[([^\]\[]+)\]/g) {
        	        #print "link: $1\n";
        	        push(@urls,$1);
        	}
        	#exit;
        	foreach my $i (@urls) {

#			print "o: $i\n";

                	if (($i =~ /http:\/\//) && ($i !~ /wikipedia\.org/)) {
                	        #print "$i -> ";

                	        if ($i =~ /\|/) {
                	                ($i, undef) = split(/\|/,$i);
                	                #print "| $i\n";
                	        }
                	        elsif ($i =~ / /) {
                	                $i =~ s/ .*//g;
                	        }

#                	        print "a: $i\n";
				printurl($i);
        	        }
			else {
				#print "n $i\n";
			}
	        }
		#print "\n";
        } 

}

sub cb {
     	my($tag, %links) = @_;
	#print "$tag @{[%links]}\n";

	#foreach my $i (keys %links) {
	#	print "$i -> $links{$i}\n";
	#}
	#if (!exists $links{'href'}) {
	#	die("dident hav a href");
	#}

	my $url = $links{'href'};
	if ($url !~ /$baseurl/) {

		#print "cp: $FerdigUrl\n";
		printurl($url);
	}

 }


sub printurl {
	my($purl) = @_;

	$purl = ResulveUrl('http://www.boitho.com/addyourlink.htm.en',$purl);

	{

		lock($fileacces);

		#print "wiki: $purl\n";
		print OUT $purl, "\n";
	}
}

sub ResulveUrl {
        my($BaseUrl,$NyUrl) = @_;

        $link = new URI::URL $NyUrl;
        $FerdigUrl = $link->abs($BaseUrl);

        return $FerdigUrl;
}

