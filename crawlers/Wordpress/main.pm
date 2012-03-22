#!/usr/bin/perl
use strict;
package Perlcrawl;
use Crawler;
our @ISA = qw(Crawler);

#use Data::Dumper;
use LWP::RobotUA;
use URI;
use Data::Dumper;
use warnings;
use URI;
use LWP;
use HTML::TokeParser;
use URI;
use sdMimeMap;
use LWP::RobotUA;
use HTML::LinkExtor;
use Date::Parse;
use Carp;
use Data::Dumper;
use Readonly;
use URI::Escape;

use constant bot_name => "sdbot/0.1";
use constant bot_email => "support\@searchdaimon.com";
use constant timeout => 4;
use constant delay => 1;
use constant verbose => 0;
use constant max_size => 26214400; #26214400=25 mb

Readonly::Hash my %LINK_IGNORE => map { $_ => 1 } qw(td script table form head link background);

our $skipDynamic = 0;
our $hit_count = 0;
our @starting_urls;
our $download_images = 0;

our $lasterror = "";



our $last_time_anything_said;
our $last_time_anything_muttered;
our %points_to;
our %notable_url_error;  # URL => error message
our %seen_url_before;
our @exclusionsUrlParts;
our $iisspecial = 0;
our @exclusionQueryParts;
our @schedule;
our $crawler;


sub path_access  {
    my (undef, $self, $opt) = @_;

    return 1;
}



sub crawl_update {
    my (undef, $self, $opt) = @_;

    $crawler = $self;

    my $user = $opt->{"user"};
    my $passw = $opt->{"password"};
    my $urls = $opt->{"url"};
    my $loginurl = $opt->{"loginurl"};
    my $starting_url;

    my @urlList = split /;/, $urls;
    my @exclusionsUrlPart = qw ( );  # See Sharpoint crawler on how to use this
    my @exclusionQueryPart = qw(); # See Sharpoint crawler on how to use this

    if (!exists $opt->{delay}) {
 $opt->{delay} = delay;
    }

    $download_images = $opt->{download_images};

    print "Name : ". bot_name."    mail :". bot_email."\n";
    my $robot = LWP::RobotUA->new( agent=>bot_name, from=>bot_email, keep_alive=>'1' );
    $robot->delay($opt->{delay}/60);
    $robot->timeout(timeout);
    $robot->max_size(max_size);
    $robot->requests_redirectable([]); # uncomment this line to disallow redirects
    $robot->protocols_allowed(['http','https']);  # disabling all others
    $robot->cookie_jar({});
    say("bot_name (bot_email) starting at ", scalar(localtime), "\n");


    process_starting_urls(@urlList);

    foreach $starting_url (@urlList) {
 	# Login to Wordpress, using http post
 	$robot->post($loginurl, [ 'log' => $user, 'pwd' => $passw, 'wp-submit' => 'Log In' ]) or warn("Cant post to login: $!");
 	# Continue as normal
 	schedule($starting_url);

    	main_loop( $robot,$user, $passw);
 	report( ) if $hit_count;
   }

   print "################################################\n";
   print "hit_count: " . $hit_count . ", error: " . $lasterror . "\n";

   if ($hit_count == 0) {
 	die($lasterror);
   }
}











sub setExclusionUrlParts {
   @exclusionsUrlParts = @_;
}

sub setIISpecial {
   $iisspecial = 1;
}


sub setExclusionQueryParts {
    @exclusionQueryParts = @_;
}

sub main_loop {
 my ($robot,$user, $passw) = @_;

  while(
    schedule_count( ) && $crawler->continue
  ) {
 my $url = next_scheduled_url( );
 if( near_url( $url ) )   {
  process_near_url($url,$robot,$user, $passw);
 }
 else {
  mutter ( "Far url $url" );
 }
  }

  return;
}





sub say {
  # Print timestamps:
  print "[" . localtime( time() ) . "] ", @_;

}

sub mutter {
  # Add timestamps as needed:
  unless(time( ) == ($last_time_anything_muttered || 0)) {
    $last_time_anything_muttered = time( );
    unshift @_, "[T$last_time_anything_muttered = " .
      localtime($last_time_anything_muttered) . "]\n";
  }
  print @_ if verbose;
  print "\n" if verbose;

  $lasterror = join(" ",@_);
}


sub near_url {   # Is the given URL "near"?
  #my $url = $_[0];
  my $url = URI->new($_[0]);
  foreach my $starting_uri (@starting_urls) {

     my $noObjectUrl = $starting_uri->path;

     if (!(($url->scheme eq 'http') || ($url->scheme eq 'https') )) {
 next;
     }

     if(    ( $url->host eq  $starting_uri->host )
 && ( $url->path =~ /$noObjectUrl/    )
     ) {
      mutter("  So $url is near");
       return 1;
    }
  }

  mutter("  So $url is far");
  return 0;
}


sub process_starting_urls {
  foreach my $url (@_) {
    my $u = URI->new($url);
       $u = $u->canonical;

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
  mutter "  For $url, Referer => $urls[0]";
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
    #skriver feilmelding, og lagrer den i $!
    mutter(sprintf "Can't get url %s: %s\n",$url, $response->status_line );
  }
  return;
}

sub consider_response {
  # Return 1 if it's successful, otherwise return 0
  my $response = $_[0];
  mutter("Consider ", $response->status_line);
  return 1 if $response->is_success;

  if($response->is_redirect) {
    my $to_url = $response->header('Location');
    if(defined $to_url and length $to_url and
      $to_url !~ m/\s/
    ) {
      my $from_url = $response->request->uri;
      $to_url = URI->new_abs($to_url, $from_url);
      mutter("Noting redirection\n  from $from_url\n","    to $to_url");
      note_link_to( $from_url => $to_url );
    }
  } else {
    note_error_response($response);
  }
  return 0;
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
 my ($url,$robot,$user, $passw) = @_;

 mutter("process_near_url(url='$url', robot=$robot, user='user')");

if ($url =~ /wp-/) {
	return;
}
 my $url_normalized = $crawler->normalize_http_url($url);
 my $keep_doc = addOk($url_normalized);

 say("Fetching " . $hit_count . ":" . schedule_count() . " \"$url\"\n");

 my $req = HTTP::Request->new(GET => $url);

 my $response = $robot->request($req);

 if (($response->code == 401) && ($user)) {
  print "asked to authenticate by method: " . $response->www_authenticate . "\n";
  if ($response->www_authenticate =~ m/basic/i) {
   $req->authorization_basic($user, $passw);
  }
  elsif (($response->www_authenticate =~ m/negotiate/i) || ($response->www_authenticate =~ m/ntlm/i)) {
   $url =~ m/http:\/\/([^\/]+)/i;
   my $server = $1 . ":80";
   print "Trying to authenticate by NTLM to server $server\n";
   $robot->credentials($server, '', $user, $passw);
  }
  else {
   die("Unknown authentication method \"" . $response->www_authenticate . "\".");
  }

  #rerun the request
  $response = $robot->request($req);
 }

print "Code: " . $response->code . "\n";
print $response->status_line . "\n";
 return unless consider_response($response);

 if (!addOk($url_normalized)) {
  extract_links_from_response($response)
   if $response->content_type eq 'text/html';
  return;
 }

 my $ct = mapMimeType($response->content_type);

 my $title = "";
 if($response->content_type ne  'text/html') {
  $title = substr($url, rindex($url, "/")+1);
  $title = uri_unescape($title);
 }



 if (!$crawler->document_exists($url, str2time($response->header('Last-Modified') || 0), length($response->content))) {

  # do a basic, but exspensiv regex to see if ther is a robot noindex metatag
#  if (($response->content_type eq 'text/html') && ($response->content =~ /<META +NAME.*?ROBOTS.*?NOINDEX.*?>/i)) {
#   print "Skiping indexint of $url_normalized because of Robots meta-tag restriction.\n";
#  }
#  else {

   # Remove tag cloud to minimize noise.
   my $htmlcontent = $response->content;
   $htmlcontent =~ s/<div class="tagcloud">.*?<\/div>//is;

   print "Added: $url_normalized\n";
   $crawler->add_document(
     url     => $url_normalized,
     title   => $title,
     content => $htmlcontent,
     last_modified => str2time($response->header('Last-Modified')),
     type    => $ct,
     acl_allow => "Everyone",
    );
   #print "Length ", length($response->content);
  }
 #}

 extract_links_from_response($response)
  if $response->content_type eq 'text/html'; # && $response->content !~ /<META +NAME.*?ROBOTS.*?NOFOLLOW.*?>/i;

 $hit_count++;
 1;
}



sub extract_links_from_response {
 my $response = $_[0];
 my $count = 0;
 my $base = URI->new( $response->base )->canonical;

 my $page_url = URI->new( $response->request->uri );


 my $page_parser = HTML::LinkExtor->new(undef, $base);
 $page_parser->parse($response->as_string)->eof;
 my @links = $page_parser->links;
 for my $l (@links) {
  next if $LINK_IGNORE{$l->[0]};

  if ($l->[1] eq 'href') {
   note_link_to($page_url, $l->[2]);
   $count++;
  }
  elsif ($l->[0] eq 'img' && $l->[1] eq 'src') {
   note_link_to($page_url, $l->[2])
    if $download_images;
  }
  elsif ($l->[0] eq 'iframe') {
   note_link_to($page_url, $l->[2]);
   $count++;
  }
  else {
   print "INFO: Unknown link type ignored: ", Dumper($l);
  }

 }

 $count--;
 print "Exstracted $count from $page_url\n";
}



sub note_link_to {
  my($from_url, $to_url) = @_;
  $points_to{ $to_url }{ $from_url } = 1;
  if (($skipDynamic == 1) && ($to_url =~ /\?/)) {
 printf("skipping dynamic url $to_url\n");
 return;
  }

  $to_url =~ s/\#.*//; # strip internal referanses


  if ($from_url eq $to_url) {
 print "Url points to self\n";
 return;
  }

if ($to_url =~ /wp-/) {
	print "Skiping wp- url: " .  $to_url ."\n";
	return;
}

  if ( near_url( $to_url ) ) {
   mutter("Noting link\n  from $from_url\n    to $to_url\n");
    schedule($to_url);
  }
  return;
}

sub next_scheduled_url {
  my $url = splice @schedule, rand(@schedule), 1;
  mutter("\nnext_scheduled_url: Pulling from schedule: ", $url || "[nil]","\n  with ", scalar(@schedule)," items left in schedule.\n");
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

sub report {
  say(
    "Ending at ", scalar(localtime),
    " after ", time( ) - $^T,
    "s of runtime and $hit_count hits.\n",
  );
  unless(keys %notable_url_error) {
    say( "No bad links seen!\n" );
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
