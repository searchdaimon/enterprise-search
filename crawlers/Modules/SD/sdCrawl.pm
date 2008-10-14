
#!/usr/bin/perl
# sdCrawl.pm

package SD::sdCrawl;

use strict;
use warnings;
use URI;
use LWP;
use HTML::TokeParser;
use URI;
use SD::Crawl;
use sdMimeMap;
use LWP::RobotUA;
use HTML::LinkExtor;
use Date::Parse;

=pod 
# Switch processing:
my %option;
use Getopt::Std;
getopts('m:n:t:l:e:n:t:d:u:p:hv', \%option) || usage_quit(1);
usage_quit(0) if $option{'h'} or not @ARGV;
 
sub usage_quit {
  # Emit usage message, then exit with given error code.
  print <<"END_OF_MESSAGE"; exit($_[0] || 0);
Usage:
$0  [switches]  [urls]
  This will spider for links and report bad ones starting at the given URLs.
   
Switches:
 -h        display this help message
 -v        be verbose in messages to STDOUT  (default off)
 -m 123    run for at most 123 minutes.  (default 20)
 -n 456    cause at most 456 network hits.  (default 500)
 -d 7      delay for 7 seconds between hits.  (default 2)
 -l x.log  log to text file x.log. (default is to not log)
 -e y\@a.b  set bot admin address to y\@a.b  (no default!)
 -b Xyz    set bot name to Xyz.  (default: sdbot/0.1)  
 -t 34     set request timeout to 34 seconds.  (default 15)
 -u        set username
 -p        set password
 
END_OF_MESSAGE
}
 
my $expiration = ($option{'m'} ||  20) * 60 + time( );
my $hit_limit  =  $option{'n'} || 500;
my $log        =  $option{'l'};
my $verbose    =  $option{'v'};
my $bot_name   =  $option{'b'} || 'sdbot/0.1';
my $bot_email  =  $option{'e'} || 'bs@searchdaimon.com';
my $timeout    =  $option{'t'} || 15;
my $delay      =  $option{'d'} || 2;
my $user       =  $option{'u'};
my $passw      =  $option{'p'};

die "Specify your email address with -e\n" unless $bot_email and $bot_email =~ m/\@/;
 



initialize( );
process_starting_urls(@ARGV);
main_loop( );
report( ) if $hit_count;
say("Quitting.\n");
exit;
=cut

my $log; 
my $expiration = 20 * 60 + time( );
my $hit_limit = 5000;
my $verbose = 0;
my $user;
my $passw;
my $bot_name = 'sdbot/0.1';
my $timeout = 100;
my $delay = 2;
my $bot_email = "email\@email.com";

my $hit_count = 0;
my $robot;  # the user-agent itself
my $QUIT_NOW;
my @schedule;
my $last_time_anything_said;
my $last_time_anything_muttered;
my @starting_urls;
my %points_to;
my %notable_url_error;  # URL => error message
my %seen_url_before;
my $acl;
my $allow_far_urls = 0;
my @ip_start;
my @ip_end;
my @ip_country2;
my @exclusionsUrlParts;
my $iisspecial = 0;
my $countries = "";
my @exclusionQueryParts;
my @allowCountries;

my $crawler;
 


sub Init {
   ($crawler, $bot_name, $bot_email, $acl, $user, $passw) = @_;
    init_logging( );
    my $robot = init_robot( );
    init_signals( );
    if ( $allow_far_urls ) {
       InitCountry();
    }
    return $robot;
}

sub doFarUrls {
   $allow_far_urls = 1;
}

sub setAllowedCountries{
   (@allowCountries) = @_;
}

sub setDelay {
   ($delay) = @_;
}

sub setExclusionUrlParts {
   @exclusionsUrlParts = @_;
}

sub setIISpecial {
   $iisspecial = 1;
}

sub Start {
   my $url;
  ($url) = @_;
   schedule($url);

   main_loop( );
   report( ) if $hit_count;
   say("Quitting.\n");
}

sub setCountries {
   $countries = @_;
}

sub setExclusionQueryParts {
    @exclusionQueryParts = @_;
}
 
sub main_loop {
  while(
    schedule_count( )
    and $hit_count < $hit_limit
    and time( ) < $expiration
    and ! $QUIT_NOW
  ) {
    process_url( next_scheduled_url( ) );
  }
  return;
}

sub InitCountry {
   open (theFile,"../Modules/ip2country.txt") || die("Could not open file!");
   my $i=0;
   while (<theFile>) {
      chomp;
      my @ips = split (",");
      $ip_start[$i] = $ips[0];
      $ip_end[$i] = $ips[1];
      $ip_country2[$i] = $ips[2];
      $i++;
   }
   close theFile;
}


sub getCountry {
   my $url = URI->new(@_);
   my $host = $url->host();
   $host = "nslookup ".$host;
   my $ip_address;
   my @lookup = `$host`;
   my $line;

   foreach $line ( @lookup ) {
      if ($line =~ /Address/) {
         my @address = split / /, $line;
         $ip_address =  $address[1];
      }
   }

   my @ipp = split (/\./,$ip_address);
   my $ip_number = $ipp[0]*256*256*256+$ipp[1]*256*256+$ipp[2]*256+$ipp[3];

   my $j = 0;
   while ($ip_number > $ip_end[$j]) {
      $j++;
   }

   my $country;

   if ($ip_number > $ip_start[$j]) {
      $country = $ip_country2[$j];
   } else {
      $country = "NA";
   }
   return $country;
}

 
sub init_logging {
  my $selected = select(STDERR);
  $| = 1; # Make STDERR unbuffered.
  if($log) {
    open LOG, ">>$log" or die "Can't append-open $log: $!";
    select(LOG);
    $| = 1; # Make LOG unbuffered
  }
  select($selected);
  print "Logging to $log\n" if $log;
  return;
}
 
sub init_robot {

  print "Name : ". $bot_name."    mail :".$bot_email."\n";
  $robot = LWP::RobotUA->new($bot_name, $bot_email);
  $robot->delay($delay/60); 
  $robot->timeout($timeout);
  $robot->requests_redirectable([]); # uncomment this line to disallow redirects
  $robot->protocols_allowed(['http','https']);  # disabling all others
   say("$bot_name ($bot_email) starting at ", scalar(localtime), "\n");
  return $robot;
}
 
sub init_signals {  # catch control-C's
  $SIG{'INT'} = sub { $QUIT_NOW = 1; return;};
  return;
}
 
sub say {
  # Add timestamps as needed:
  unless(time( ) == ($last_time_anything_said || 0)) {
    $last_time_anything_said = time( );
    unshift @_, "[T$last_time_anything_said = " .
      localtime($last_time_anything_said) . "]\n";
  }
  print LOG @_ if $log;
  print @_;
}

sub mutter {
  # Add timestamps as needed:
  unless(time( ) == ($last_time_anything_muttered || 0)) {
    $last_time_anything_muttered = time( );
    unshift @_, "[T$last_time_anything_muttered = " .
      localtime($last_time_anything_muttered) . "]\n";
  }
  print LOG @_ if $log;
  print @_ if $verbose;
}

sub process_url {
  my $url = $_[0];
   if( near_url($url) )   { process_near_url($url) }
  else                   { process_far_url($url) }
  return;
}

sub near_url {   # Is the given URL "near"?
  #my $url = $_[0];
  my $url = URI->new($_[0]);
  foreach my $starting_url (@starting_urls) {
     my $starting_uri =URI->new($starting_url);
     #if( substr($url, 0, length($starting_url))
     #eq $starting_url
    if( $url->host() eq  $starting_uri->host()
    ) {
      mutter("  So $url is near\n");
       return 1;
    }
  }
  mutter("  So $url is far\n");   
  return 0;
}


sub process_starting_urls {
  foreach my $url (@_) {
    my $u = URI->new($url)->canonical;
     push @starting_urls, $u;
  }
   #schedule($starting_urls[0]);
   #return;
}

sub refer {
  my $url = $_[0];
  my $links_to_it = $points_to{$url};
   # the set (hash) of all things that link to $url
  return( ) unless $links_to_it and keys %$links_to_it;

  my @urls = keys %$links_to_it; # in no special order!
  mutter "  For $url, Referer => $urls[0]\n";
  return "Referer" => $urls[0];
}


sub note_error_response {
  my $response = $_[0];
  return unless $response->is_error;

  my $code = $response->code;
  my $url = URI->new( $response->request->uri )->canonical;

  if(  $code == 404 or $code == 410 or $code == 500  ) {
    mutter(sprintf "Noting {%s} error at %s\n",$response->status_line, $url );
    $notable_url_error{$url} = $response->status_line;
  } else {
    mutter(sprintf "Not really noting {%s} error at %s\n",$response->status_line, $url );
  }
  return;
}

sub consider_response {
  # Return 1 if it's successful, otherwise return 0
  my $response = $_[0];
  mutter("Consider ", $response->status_line, "\n");
  return 1 if $response->is_success;
  
  if($response->is_redirect) {
    my $to_url = $response->header('Location');
    if(defined $to_url and length $to_url and 
      $to_url !~ m/\s/
    ) {
      my $from_url = $response->request->uri;
      $to_url = URI->new_abs($to_url, $from_url);
      mutter("Noting redirection\n  from $from_url\n","    to $to_url\n");
      note_link_to( $from_url => $to_url );
    }
  } else {
    note_error_response($response);
  }
  return 0;
}

sub checkCategory {
   my $data = @_;

   #if ($data =~ /new DiscussionBoard/) { return "Discussion"; }
   #if ($data =~ /Tasks list to keep track of work related to this area/) { return "Calendar"; }
   #if ($data =~ /Provides a place to store documents for this area/) { return "DocumentLibrary"; }
   #if ($data =~ /L_DefaultContactsLink_Text/) { return "Contacts"; }
   #if ($data =~ /L_ExportToContactsApp/) { return "Contacts"; }

  return "";
}

sub country_ok {
   my ($currentCountry) = @_;
   my $country;

   if (!@allowCountries) {
      return 1;
   }
   foreach $country(@allowCountries) {
      if ($currentCountry eq $country) {
         return 1;
      }
   }
   return 0;
}

sub process_far_url {
  my $url = $_[0];
  say("HEADing far $url\n");
  ++$hit_count;
  mutter("  That was hit #$hit_count\n");

   if (!$allow_far_urls) { return; }

   return unless country_ok(getCountry($url));

   my $req = HTTP::Request->new(GET => $url);

   my $response = $robot->request($req);
   my $ct = mapMimeType($response->content_type);

    my $title = "";
    if($ct ne  'text/html') {
       $title = substr($url, rindex($url, "/")+1);
     }
   
  
   if (not $crawler->document_exists($url, 0, length($response->as_string))) {
     $url = $crawler->normalize_http_url($url);
	 $crawler->add_document(
	 	url     => $url,
		title   => $title,
		type    => $ct,
		acl_allow => $acl,
		content => $response->content,
		last_modified => str2time($response->header('Last-Modified'))
		);
   }
 
    mutter("  That was hit #$hit_count\n");
    return unless consider_response($response);
 
    if($ct eq 'text/html') {
       extract_links_from_response($response);     
    }
     return;
}

sub authorize {
  my $req = $_[0];
  if ($user) { 
     mutter("Autorizing ".$user." with password ".$passw."\n");
     return $req->authorization_basic($user, $passw); 
  }
}

sub addOk {
   my $url;
   my  $restricted;
   ($url) = @_;
   my $uri = URI->new($url);
    foreach $restricted (@exclusionsUrlParts) {
       $uri->path();
      if ($uri->path() =~ /$restricted/) {
          return 0;
       }
     }
   return 1;
}

sub process_near_url {
   my $url = $_[0];

   my $htmlcontent;
  my $refurl = refer($url); # can be user for courtesy like $robot->request($req, $refurl);
  mutter("HEADing near $url\n");
  ++$hit_count;

  my $req = HTTP::Request->new(HEAD => $url);

  if ($user) { 
      mutter("Autorizing ".$user." with password ".$passw."\n");
      $req->authorization_basic($user, $passw); 
  }
  print "Examine : ", $url, "\n";
  my $response = $robot->request($req);

  mutter("  That was hit #$hit_count\n");
  return unless consider_response($response);

  if(length ${ $response->content_ref }) {
    mutter("  Hm, that had content!  Using it...\n" );
    say("Using head-gotten $url\n");
  } else {
    mutter("It's HTML!\n");
    say("Getting $url\n");
    ++$hit_count;
  }
 
      $req = HTTP::Request->new(GET => $url);

    if ($user) { 
       mutter("Autorizing ".$user." with password ".$passw."\n");
       $req->authorization_basic($user, $passw); 
    }

    $response = $robot->request($req);
    my $ct = mapMimeType($response->content_type);

    my $title = "";
    if($response->content_type ne  'text/html') {
       $title = substr($url, rindex($url, "/")+1);
   }
   
   my $category = checkCategory($response->as_string);
    if (not $crawler->document_exists($url, 0, length($response->as_string ) )) {
	  $url = $crawler->normalize_http_url($url);
       if (addOk($url)) {
           $crawler->add_document(
			url     => $url,
			title   => $title,
			content => $response->content,
			last_modified => str2time($response->header('Last-Modified')),
			type    => $ct,
			acl_allow => $acl,
			attributes => $category);
      }	
   }
    mutter("  That was hit #$hit_count\n");
    return unless consider_response($response);
    #print $response->as_string;
    if($response->content_type eq 'text/html') {
       extract_links_from_response($response);     
    }
     return;
}


#sub extract_links_from_response {
#  my $response = $_[0];
#  my $base = URI->new( $response->base )->canonical;
    # "canonical" returns it in the one "official" tidy form

#  my $stream = HTML::TokeParser->new( $response->content_ref );
#  my $page_url = URI->new( $response->request->uri );

#  mutter( "Extracting links from $page_url\n" );

#  my($tag, $link_url);
#  while( $tag = $stream->get_tag('a') ) {

#    next unless defined($link_url = $tag->[1]{'href'});
#    next unless length $link_url; # sanity check!
#    $link_url = URI->new_abs($link_url, $base)->canonical;
#    next unless $link_url->scheme eq 'http'; # sanity
  
#    if ($allow_far_urls) {
#       $link_url->fragment(undef); # chop off any "#foo" part
#       next if $link_url =~ m/\s/; # If it's got whitespace, it's a bad URL.
#    }

#    note_link_to($page_url => $link_url)
#      unless $link_url->eq($page_url); # Don't note links to itself!
#  }
#  return;
#}

sub extract_links_from_response {
  my $response = $_[0];
  my $base = URI->new( $response->base )->canonical;

  my $page_url = URI->new( $response->request->uri );
 

   my $page_parser = HTML::LinkExtor->new(undef, $base);
   $page_parser->parse($response->as_string)->eof;
   my @links = $page_parser->links;
   my  $link;
	foreach $link (@links) {
           if ($$link[1] eq "href") {
              note_link_to($page_url => $$link[2])
            }
         }

 return;
}


sub note_link_to {
  my($from_url => $to_url) = @_;
  $points_to{ $to_url }{ $from_url } = 1;
  mutter("Noting link\n  from $from_url\n    to $to_url\n");
   schedule($to_url);
  return;
}

sub next_scheduled_url {
   my $url = splice @schedule, rand(@schedule), 1;
  mutter("\nPulling from schedule: ", $url || "[nil]","\n  with ", scalar(@schedule)," items left in schedule.\n");
  return $url;
}

sub schedule_count     { return scalar @schedule }

sub modifyQueryPart {
   my ($u) = @_;
   my $exclusiontags;
   my $query;
   my $qs = $u->query();
 
   if (!$qs) { return; }

   my @newqs;
   my @queyTags = split /&/, $qs;

   if (!@exclusionQueryParts) { return; }

   foreach $query (@queyTags) {
      my @tags = split /=/, $query;
      my $exclude = 0;
      foreach $exclusiontags (@exclusionQueryParts) {
         if ($exclusiontags eq $tags[0]) {
           $exclude = 1;
         }
      }
      if (!$exclude) {
          push @newqs, $tags[0];
          push @newqs, $tags[1]; 
      }
   }
   if (@newqs) {
      $u->query_form(@newqs);
   } else {
      $u->query(undef);
   }

   return $u
}

sub schedule {
  # Add these URLs to the schedule
  foreach my $url (@_) {
     my $u = ref($url) ? $url : URI->new($url);
     $u = $u->canonical;  # force canonical form
     next unless 'http' eq ($u->scheme || '') or 'https' eq ($u->scheme || '');
     
    $u->host( regularize_hostname( $u->host( ) ) );

    if ($allow_far_urls) {
       next if defined $u->query; 
       next if defined $u->userinfo; 
       return unless $u->host( ) =~ m/\./; 
       next if url_path_count($u) > 6;
       next if $u->path =~ m<//> or $u->path =~ m</\.+(/|$)>;
       $u->fragment(undef);
 
    }

    if (lc($u->as_string) =~ "default.aspx") {
       $u = URI->new(substr($u->as_string, 0, rindex(lc($u->as_string), "default.aspx")));
    }

     modifyQueryPart($u);
 
     if ($iisspecial) {

         my $qs = $u->query();
         my $lc_url = lc($u);
         $u = URI->new($lc_url);
         $u->query($qs);
      }

    if( $seen_url_before{ $u->as_string }++ ) {
      mutter("  Skipping the already-seen $u\n");
    } else {
      mutter("  Scheduling $u\n");
      push @schedule, $u;
    }
  }
  return;
}

sub regularize_hostname {
  my $host = lc $_[0];
  $host =~ s/\.+/\./g; # foo..com => foo.com
  $host =~ s/^\.//;    # .foo.com => foo.com
  $host =~ s/\.$//;    # foo.com. => foo.com
  return 'localhost' if $host =~ m/^0*127\.0+\.0+\.0*1$/;
  return $host;
}

sub url_path_count {
  # Return 4 for "http://fee.foo.int/dedle/dudle/dum/sum"
  my $url = $_[0];
  my @parts = $url->path_segments;
  shift @parts if @parts and $parts[ 0] eq '';
  pop   @parts if @parts and $parts[-1] eq '';
  return scalar @parts;
}
 
sub report {  
  say(
    "\n\nEnding at ", scalar(localtime),
    " after ", time( ) - $^T,
    "s of runtime and $hit_count hits.\n\n",
  );
  unless(keys %notable_url_error) {
    say( "\nNo bad links seen!\n" );
    return;
  }
 
  say( "BAD LINKS SEEN:\n" );
  foreach my $url (sort keys %notable_url_error) {
    say( "\n$url\n  Error: $notable_url_error{$url}\n" );
    foreach my $linker (sort keys %{ $points_to{$url} } ) {        
      say( "  < $linker\n" );
    }
  }
  return;
}


1;
