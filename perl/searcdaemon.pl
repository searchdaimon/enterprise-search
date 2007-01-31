#!/usr/bin/perl
# dummyhttpd - start an HTTP daemon and print what the client sends

use strict;
use HTTP::Daemon;
use HTTP::Status;

my $d = new HTTP::Daemon->new(LocalPort => 8989);
print "Please contact me at: <URL:", $d->url, ">\n";
while (my $c = $d->accept) {
      my $r = $c->get_request;
      if ($r) {
          if ($r->method eq 'GET') {
           		#logger	
			  	print $r->url->query . "\n";
			  
			  	#$c->send_status_line(200);
				# get dekoder kale med $FORM{navn}
				my %FORM = ();
				my @pairs = split(/&/, $r->url->query);
				foreach my $pair (@pairs) {
					my ($name, $value) = split(/=/, $pair);
        			$value =~ tr/+/ /;
        			$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        			$value =~ s/<!--(.|\n)*-->//g;
        			$FORM{$name} = $value;
				}
				# /get dekoder
				
				#print "$FORM{'query'}\n";
				my $xml = `./searchkernel $FORM{'query'} > /tmp/boithosearsc`;

				#$r->content( $xml );
				#$c->send_response( $r );
				$c->send_file_response('/tmp/boithosearsc');
				
				
				print $c "\n";
          } else {
              $c->send_error(RC_FORBIDDEN)
          }
      }
	  
	  $c->close;
      $c = undef;  # close connection
}