
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
use Carp;
use Data::Dumper;
use Readonly;

Readonly::Hash my %LINK_IGNORE => map { $_ => 1 } qw(td script table form head);

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
my @schedule;
my $last_time_anything_said;
my $last_time_anything_muttered;
my @starting_urls;
my %points_to;
my %notable_url_error;  # URL => error message
my %seen_url_before;
my $acl;
my @ip_start;
my @ip_end;
my @exclusionsUrlParts;
my $iisspecial = 0;
my @exclusionQueryParts;
my $download_images = 0;

my $crawler;
 


sub Init {
   ($crawler, $bot_name, $bot_email, $acl, $user, $passw) = @_;
    init_logging( );
    my $robot = init_robot( );
    return $robot;
}

sub set_download_images {
	$download_images = shift;
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

sub setExclusionQueryParts {
    @exclusionQueryParts = @_;
}
 
sub main_loop {
  while(
    schedule_count( )
    and $hit_count < $hit_limit
    and time( ) < $expiration
  ) {
	my $url = next_scheduled_url( );
	if( near_url( $url ) )   { 
		process_near_url($url);
	}
	else {
		mutter ( "Far url $url" );
	}
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
  $robot->requests_redirectable([]); # uncomment this line to disallow redirects
  $robot->protocols_allowed(['http','https']);  # disabling all others
   say("$bot_name ($bot_email) starting at ", scalar(localtime), "\n");
  return $robot;
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


sub near_url {   # Is the given URL "near"?
  #my $url = $_[0];
  my $url = URI->new($_[0]);
  foreach my $starting_url (@starting_urls) {
     my $starting_uri =URI->new($starting_url);
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
   my $data = shift;

   if ($data =~ /contacts/) { return "SPCategory=Contacts"; }
   if ($data =~ /Document%20Library/) { return "SPCategory=Document"; }
   #if ($data =~ /new DiscussionBoard/) { return "Discussion"; }
   #if ($data =~ /Tasks list to keep track of work related to this area/) { return "Calendar"; }
   #if ($data =~ /Provides a place to store documents for this area/) { return "DocumentLibrary"; }
   #if ($data =~ /L_DefaultContactsLink_Text/) { return "Contacts"; }
   #if ($data =~ /L_ExportToContactsApp/) { return "Contacts"; }

  return "SPCategory=Other";
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
	my $url = shift;

	my $url_normalized = $crawler->normalize_http_url($url);
	my $keep_doc = addOk($url_normalized);

	say("Fetching \"$url\"\n");

	my $req = HTTP::Request->new(GET => $url);
	$req->authorization_basic($user, $passw)
		if $user;

	my $response = $robot->request($req);
	return unless consider_response($response);

	if (!addOk($url_normalized)) {
		my $ct = mapMimeType($response->content_type);
		extract_links_from_response($response)
			if $response->content_type eq 'text/html';
		return;
	}

	my $ct = mapMimeType($response->content_type);

	my $title = "";
	if($response->content_type ne  'text/html') {
		$title = substr($url, rindex($url, "/")+1);
	}


	my $category = checkCategory($url_normalized);
	if (!$crawler->document_exists($url, 0, length($response->content))) {
		$crawler->add_document(
				url     => $url_normalized,
				title   => $title,
				content => $response->content,
				last_modified => str2time($response->header('Last-Modified')),
				type    => $ct,
				acl_allow => $acl,
				attributes => $category);
		#print "Length ", length($response->content);
	}	

	extract_links_from_response($response)
		if $response->content_type eq 'text/html';
	1;
}



sub extract_links_from_response {
	my $response = $_[0];
	my $base = URI->new( $response->base )->canonical;

	my $page_url = URI->new( $response->request->uri );


	my $page_parser = HTML::LinkExtor->new(undef, $base);
	$page_parser->parse($response->as_string)->eof;
	my @links = $page_parser->links;
	for my $l (@links) {
		next if $LINK_IGNORE{$l->[0]};

		if ($l->[1] eq 'href') {
			note_link_to($page_url, $l->[2]);
		}
		elsif ($l->[0] eq 'img' && $l->[1] eq 'src') {
			note_link_to($page_url, $l->[2])
				if $download_images;
		}
		elsif ($l->[0] eq 'iframe') {
			note_link_to($page_url, $l->[2]);
		}
		else {
			print "INFO: Unknown link type ignored: ", Dumper($l);
		}
	}
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
