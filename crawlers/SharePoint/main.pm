

#!/usr/bin/perl
use strict;
package Perlcrawl;
use SD::Crawl;

#use SOAP::Lite(  +trace => 'all', readable => 1, outputxml => 1, );
use SOAP::Lite(readable => 1, outputxml => 1, );
use XML::XPath;
use SD::sdCrawl;
use LWP::RobotUA;
use URI;
 
 

#use Data::Dumper;
my $pointer; 
my $soap_client;
my $robot;
my $bot_name = "sdbot/0.1";
my $bot_email = "bs\@searchdaimon.com";

sub init_robot { 
   my $timeout = 4;
 
   $robot = LWP::RobotUA->new($bot_name, $bot_email);
   $robot->delay(0); # "/60" to do seconds->minutes
   $robot->timeout($timeout);
   $robot->requests_redirectable([]); # comment this line to allow redirects
   $robot->protocols_allowed(['http','https']);  # disabling all others
}

sub crawlpatAcces  {
    my ($self, $pointer, $opt ) = @_;
    my $user = $opt->{'user'};
    my $passw  => $opt->{'password'};
    my $url = $opt->{"resource"};

    if (!defined($robot)) { init_robot() ; }

    my $req = HTTP::Request->new(HEAD => $url);

    if ($user) { 
        $req->authorization_basic($user, $passw); 
    }

    my $response = $robot->request($req);
    if (!$response->is_success) { return 1; }
    return 0;
}

sub crawlupdate {	
   my ($self, $pointer, $opt ) = @_;	

    my $user = $opt->{"user"};
    my $passw = $opt->{"password"};
    my $Urls = $opt->{"resource"};
    my $starting_url;
    my @urlList = split /;/, $Urls;

    SD::sdCrawl::process_starting_urls(@urlList);
    SD::sdCrawl::setDelay(2);
    SD::sdCrawl::doFarUrls();

    foreach $starting_url(@urlList) {
       my $url = URI->new(@urlList[0]);
       my $host = $url->host();
    
       #$soap_client = new SOAP::Lite
       #uri => 'http://schemas.microsoft.com/sharepoint/soap/directory',
       #proxy =>"http://".$user.":".$passw."@".$host."/_vti_bin/UserGroup.asmx";

      #$soap_client->on_action(sub {
      #   #print Dumper(\@_);
      # return $_[0]."/". $_[1];
      #});

      my $acl = "";

      #my $xml = $soap_client->GetAllUserCollectionFromWeb();   
      #my $xp = XML::XPath->new(xml => $xml);  
      #my $nodeset =  $xp->findnodes('//User/@Sid');
    
      #foreach my $node ($nodeset->get_nodelist) {
      #   #put in a list and use join instead when more usernames available
      #   my $usr = XML::XPath::XMLParser::as_string($node);
      #   $usr = substr($usr,index($usr,"\"")+1);
      #   $usr = substr($usr,0,length($usr)-1);
      #   $acl = $acl.$usr.',';
      #}

      if (length($acl)) {
         $acl = substr($acl, 0, length($acl)-1);
      }
      SD::sdCrawl::Init($pointer, $bot_name, , "email\@email.com", $acl, $user, $passw);
      SD::sdCrawl::Start($starting_url);
   }
}





