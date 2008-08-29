package Perlcrawl;
use Carp;
use Data::Dumper;
use strict;
use warnings;

use Crawler;
our @ISA = qw(Crawler);

use Carp;
use JSON::XS qw(decode_json);
use Data::Dumper;
use LWP::Simple qw(get);

use SD::Crawl;
use SOAP::Lite(readable => 1, outputxml => 1, );
use XML::XPath;
use SD::sdCrawl;
use LWP::RobotUA;
use URI;


my $bot_name = "sdbot/0.1";
my $bot_email = "email\@email.com";

##
# Main loop for a crawl update.
# This is where a resource is crawled, and documents added.
sub crawl_update {
    my (undef, $self, $opt) = @_;

    my $user = $opt->{"user"};
    my $passw = $opt->{"password"};
    my $urls = $opt->{"resource"};
    my $starting_url;

    my @urlList = split /;/, $urls;
    my @exclusionsUrlPart = qw (editform.aspx newform.aspx /forms/allitems.aspx /contacts/allitems.aspx /tasks/allitems.aspx /tasks/active.aspx /tasks/byowner.aspx orms/recentchanges.aspx forms/allpages.aspx backlinks.aspx /calendar.aspx /_layouts/ );

    SD::sdCrawl::process_starting_urls(@urlList);
    SD::sdCrawl::setDelay(0.1);
    SD::sdCrawl::setExclusionUrlParts(@exclusionsUrlPart);
    SD::sdCrawl::setIISpecial();

    foreach $starting_url(@urlList) {
       my $url = URI->new(@urlList[0]);
       print "Current url is " . $url . "\n";
       my $host = $url->host();

       my $soap_client = new SOAP::Lite
       uri => 'http://schemas.microsoft.com/sharepoint/soap/directory',
       proxy =>"http://".$user.":".$passw."@".$host."/_vti_bin/UserGroup.asmx";

      $soap_client->on_action(sub {
         #print Dumper(\@_);
       return $_[0]."/". $_[1];
      });

      my $acl = "";

      my $xml = $soap_client->GetAllUserCollectionFromWeb();
      print $xml;
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
#      $acl = "Everyone";
      SD::sdCrawl::Init($self, $bot_name, , $bot_email, $acl, $user, $passw);
      SD::sdCrawl::Start($starting_url);
   }

};


my $robot;
sub init_robot {
   my $timeout = 4;

   $robot = LWP::RobotUA->new($bot_name, $bot_email);
   $robot->delay(0); # "/60" to do seconds->minutes
   $robot->timeout($timeout);
   $robot->requests_redirectable([]); # comment this line to allow redirects
   $robot->protocols_allowed(['http','https']);  # disabling all others
}

sub path_access {
    my ($undef, $self, $opt) = @_;
# print Data::Dumper->Dump([$opt]);
    my $user = $opt->{'user'};
    my $passw  = $opt->{'password'};
    my $url = $opt->{'resource'};

    if (!defined($robot)) { init_robot() ; }

    my $req = HTTP::Request->new(HEAD => $url);
    print "Authenticating :  ", $user, "  password  ",  $passw, " at ", $url , "\n";

    if ($user) {
        $req->authorization_basic($user, $passw);
    }

    my $response = $robot->request($req);

    if ($response->is_success) { return 1; }

    print "Not authenticated :  ", $user, "  password  ", $passw, " at ", $url , "\n";
    return 0;
}

1;
