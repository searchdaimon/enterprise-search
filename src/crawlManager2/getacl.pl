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
    proxy => 'http://administrator:Ju12brzz@search/_vti_bin/Permissions.asmx'
  ;
$soap_client->on_action(sub {
#print Dumper(\@_);
	return $_[0].$_[1];
   });


#Namespace must end with /
$soap_client->ns("http://schemas.microsoft.com/sharepoint/soap/directory/","tns");
$objectName = SOAP::Data->type("xsd:string")->name("tns:objectName" => 'http://search/');
$objectType = SOAP::Data->type("xsd:string")->name("tns:objectType" => 'web');
#$objectType = SOAP::Data->type("xsd:string")->name("tns:objectType" => 'list'); for sharepoint lists
my $xml = $soap_client->GetPermissionCollection($objectName, $objectType);    
my $xp = XML::XPath->new(xml => $xml);  
my $nodeset =  $xp->findnodes('//Permission/@UserLogin');
    
foreach my $node ($nodeset->get_nodelist) {
   #put in a list and use join instead when more usernames available
   my $usr = XML::XPath::XMLParser::as_string($node);
   $usr = substr($usr,index($usr,"\"")+1);
   $usr = substr($usr,0,length($usr)-1);
   $acl = $acl.$usr.',';
}


SD::sdCrawl::Init($pointer, "sdbot/0.1", "email\@email.com", $acl, "search\\administrator", "Ju12brzz");
#SD::sdCrawl::process_starting_urls("http://en.wikipedia.org/wiki/Web_crawler", "http://en.wikipedia.org/");
SD::sdCrawl::process_starting_urls("http://search");
SD::sdCrawl::Start();
}





