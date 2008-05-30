
#!/usr/bin/perl
# sdCrawl.pm

package Perlcrawl;
use SD::Crawl;
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
my $pointer;


sub Init {
   ($pointer, $bot_name, $bot_email, $acl, $user, $passw) = @_;
    init_logging( );
    my $robot = init_robot( );
    init_signals( );
    return $robot;
}

sub setDelay {
   ($delay) = @_;
}

sub Start {
   my $url;
  ($url) = @_;
   schedule($url);

   main_loop( );
   report( ) if $hit_count;
   say("Quitting.\n");
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
  #$robot->requests_redirectable([]); # uncomment this line to disallow redirects
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
  # Generate a good Referer header for requesting this URL.
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

   if ($data =~ /new DiscussionBoard/) { return "Discussion"; }
   if ($data =~ /Tasks list to keep track of work related to this area/) { return "Calendar"; }
   if ($data =~ /Provides a place to store documents for this area/) { return "DocumentLibrary"; }
   if ($data =~ /L_DefaultContactsLink_Text/) { return "Contacts"; }
   if ($data =~ /L_ExportToContactsApp/) { return "Contacts"; }

  return "";
}

sub process_far_url {
  my $url = $_[0];
  say("HEADing far $url\n");
  ++$hit_count;
  my $response = $robot->head($url, refer($url));
  mutter("  That was hit #$hit_count\n");
  consider_response($response);  # set switch to configure for far urls!
}

sub authorize {
  my $req = $_[0];
  if ($user) { 
     mutter("Autorizing ".$user." with password ".$passw."\n");
     return $req->authorization_basic($user, $passw); 
  }
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
  print "Visiting : ", $url, "\n";
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

   if (not SD::Crawl::pdocumentExist($pointer, $url, 0, length($response->as_string ) )) {
      # pdocumentAdd( x, url, lastmodified, dokument_size, document, title, acl_allow, acl_denied )
      SD::Crawl::pdocumentAdd($pointer, $url, 0 ,length($response->as_string ), $response->as_string, $title, $ct, $acl, "",$category);		
   }
 
    mutter("  That was hit #$hit_count\n");
    return unless consider_response($response);
    #print $response->as_string;
    if($response->content_type eq 'text/html') {
       extract_links_from_response($response);     
    }
     return;
}


sub extract_links_from_response {
  my $response = $_[0];
  my $base = URI->new( $response->base )->canonical;
    # "canonical" returns it in the one "official" tidy form

  my $stream = HTML::TokeParser->new( $response->content_ref );
  my $page_url = URI->new( $response->request->uri );

  mutter( "Extracting links from $page_url\n" );

  my($tag, $link_url);
  while( $tag = $stream->get_tag('a') ) {
    next unless defined($link_url = $tag->[1]{'href'});
    #next if $link_url =~ m/\s/; # If it's got whitespace, it's a bad URL.
    next unless length $link_url; # sanity check!
    $link_url = URI->new_abs($link_url, $base)->canonical;
    next unless $link_url->scheme eq 'http'; # sanity
  
    #$link_url->fragment(undef); # chop off any "#foo" part

    note_link_to($page_url => $link_url)
      unless $link_url->eq($page_url); # Don't note links to itself!
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

sub schedule {
  # Add these URLs to the schedule
  foreach my $url (@_) {
     my $u = ref($url) ? $url : URI->new($url);
     $u = $u->canonical;  # force canonical form
     next unless 'http' eq ($u->scheme || '') or 'http' eq ($u->scheme || '');
     #next if defined $u->query; do if far urls
     #next if defined $u->userinfo; do if far urls
     $u->host( regularize_hostname( $u->host( ) ) );

    #return unless $u->host( ) =~ m/\./;
 
    #next if url_path_count($u) > 6;
    #next if $u->path =~ m<//> or $u->path =~ m</\.+(/|$)>;

    $u->fragment(undef);
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
