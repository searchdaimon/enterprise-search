#!/usr/bin/perl

package Perlcrawl;
use SD::Crawl;

#use SOAP::Lite(  +trace => 'all', readable => 1, outputxml => 1, );
use SOAP::Lite(readable => 1, outputxml => 1, );
use XML::XPath;
use SD::sdCrawl;
 
 

#use Data::Dumper;
my $acl;
my $pointer; 
my $soap_client;


sub crawlupdate {	
   ($pointer ) = @_;	

    $soap_client = new SOAP::Lite
    uri => 'http://schemas.microsoft.com/sharepoint/soap/directory',
    proxy => 'http://www.xsolive.com/_vti_bin/Permissions.asmx'
  ;
$soap_client->on_action(sub {
#print Dumper(\@_);
	return $_[0].$_[1];
   });


   my $xml = $soap_client->GetAllUserCollectionFromWeb();   
   my $xp = XML::XPath->new(xml => $xml);  
   my $nodeset =  $xp->findnodes('//User/@Sid');
    
foreach my $node ($nodeset->get_nodelist) {
   #put in a list and use join instead when more usernames available
   my $usr = XML::XPath::XMLParser::as_string($node);
   $usr = substr($usr,index($usr,"\"")+1);
   $usr = substr($usr,0,length($usr)-1);
   $acl = $acl.$usr.',';
}
if (length($acl)) {
   $acl = substr($acl, 0, length($acl)-1);
}

SD::sdCrawl::Init($pointer, "sdbot/0.1", "email\@email.com", $acl, "demo\\test", "Xsolive2007");
SD::sdCrawl::process_starting_urls("http://www.xsolive.com");
SD::sdCrawl::Start();
}





