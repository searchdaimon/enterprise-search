#!/usr/bin/perl
use strict;
package Perlcrawl;
use Crawler;
our @ISA = qw(Crawler);

#use SOAP::Lite(  +trace => 'all', readable => 1, outputxml => 1, );
#use SOAP::Lite(readable => 1, outputxml => 1, );
#use XML::XPath;
use SD::sdCrawl;
use LWP::RobotUA;
use URI;
use Data::Dumper;
 

#use Data::Dumper;
my $pointer; 
#my $soap_client;
my $robot;
my $bot_name = "sdbot/0.1";
my $bot_email = "support\@searchdaimon.com";
sub init_robot { 
   my $timeout = 4;
 
   $robot = LWP::RobotUA->new($bot_name, $bot_email);
   $robot->delay(0); # "/60" to do seconds->minutes
   $robot->timeout($timeout);
   $robot->requests_redirectable([]); # comment this line to allow redirects
   $robot->protocols_allowed(['http','https']);  # disabling all others
}

sub path_access  {
	my (undef, $self, $opt) = @_;
    my $user = $opt->{'user'};
    my $passw  = $opt->{'password'};
    my $url = $opt->{"resource"};

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

sub crawl_update {	
    my (undef, $self, $opt) = @_;	

    my $user = $opt->{"user"};
    my $passw = $opt->{"password"};
    my $urls = $opt->{"url"};
    my $starting_url;
  
    my @urlList = split /;/, $urls;
    my @exclusionsUrlPart = qw ( );  # See Sharpoint crawler on how to use this
    my @exclusionQueryPart = qw(); # See Sharpoint crawler on how to use this


    SD::sdCrawl::process_starting_urls(@urlList);
    SD::sdCrawl::setDelay($opt->{delay} || 2);
    SD::sdCrawl::skipDynamic(0);
    SD::sdCrawl::set_download_images($opt->{download_images});
 
    foreach $starting_url(@urlList) {
       my $url = URI->new(@urlList[0]);
       my $acl = "Everyone";

      SD::sdCrawl::Init($self, $bot_name, , "email\@email.com", $acl, $user, $passw);
      SD::sdCrawl::Start($starting_url);
   }
}

1;

