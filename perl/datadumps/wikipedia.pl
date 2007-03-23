  #!/usr/bin/perl -w
  
  use strict;
  use Parse::MediaWikiDump;
    
  my $file = shift(@ARGV) or die "must specify a Mediawiki dump file";
  my $pages = Parse::MediaWikiDump::Pages->new($file);
  my $page;
    
  while(defined($page = $pages->page)) {
    #main namespace only           
    next unless $page->namespace eq '';
  
    print $page->title, "\n" unless defined($page->categories);
  }

