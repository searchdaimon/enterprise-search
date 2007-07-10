##
# Handle IPN notifications from PayPal.
# Losely based on sample script provided on paypal.com
#
# See eof for an usage example.
package SD::PayPal::IPN;
use strict;
use warnings;
use Carp;
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(pp_validate IPN_COMPLETED IPN_CANCELED_REVERSAL 
        IPN_PENDING IPN_FAILED IPN_DENIED IPN_REFUNDED IPN_REVERSED);

use SD::PayPal::X::Validate;

use constant IPN_COMPLETED => "Completed";
use constant IPN_CANCELED_REVERSAL => "Canceled_Reversal";
use constant IPN_PENDING => "Pending";
use constant IPN_FAILED => "Failed";
use constant IPN_DENIED => "Denied";
use constant IPN_REFUNDED => "Refunded";
use constant IPN_REVERSED => "Reversed";

my %vars; # vars parsed from notification

sub pp_validate {
    my $paypal_url = shift;
    croak "argument paypal_url missing" 
        unless defined $paypal_url;

    # read post from PayPal system and add the required 'cmd' attribute
    read (STDIN, my $query, $ENV{'CONTENT_LENGTH'});
    $query .= '&cmd=_notify-validate';

    # post back to PayPal system to validate
    use LWP::UserAgent;
    my $ua = new LWP::UserAgent;
    my $req = HTTP::Request->new(POST => $paypal_url);
    $req->content_type('application/x-www-form-urlencoded');
    $req->content($query);
    my $res = $ua->request($req);

    %vars = parse_vars($query);

    if ($res->is_error) {
	croak (X::Validate->new(  
	    reason => "HTTP error when verifying. " . $res->status_line, 
	    vars => \%vars 
	));
    }

    elsif ($res->content eq 'VERIFIED') {
	return %vars;
    }

    elsif ($res->content eq 'INVALID') {
        croak(X::Validate->new( 
            reason => "Invalid IPN call",
            vars => \%vars 
        ));
    }
    else {
        croak(X::Validate->new( 
            reason => "Unknown response from pp: " . $res->content,
            vars => \%vars 
        ));
    }
}

## 
# Parses variables from notification
sub parse_vars {
    my $query = shift;
    my @pairs = split(/&/, $query);
    my %parsed;
    foreach my $pair (@pairs) {
	my ($name, $value) = split(/=/, $pair);
	$value =~ tr/+/ /;
	$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
	$parsed{$name} = $value;
    }

    return %parsed;
}

# Example usage:
# > use SD::PayPal::IPN;
# > use X::Validate;
# > use Switch;
# > use CGI;
# >
# > my %pp_vars;
# > eval { %pp_vars = pp_validate($paypal_url) };
# > if ($@) {
# >  if (X::Validate->caught()) {
# >      # Validation error caught.
# >      # Log reason and data sent from paypal for further investigation.
# >     my ($reason, $vars_ref) = ($@->reason, $@->vars);
# >
# >     print CGI::header("text/plain"); # Return 200 OK, or paypal will 
# >                                      # continue to send the same notification for 1.5 days.
# >     exit;
# >  }
# >  else {
# >       # Unknown error
# >      croak $@;
# >  }
# > }
# >
# > # Paypal validation OK. Data is in %pp_vars.
#
# check the payment_status is Completed
# check that txn_id has not been previously processed
# check that receiver_email is your Primary PayPal email
# check that payment_amount/$payment_currency are correct
# process payment
#
# >
# > switch ($pp_vars{payment_status}) {
# >      case IPN_COMPLETED {
# >      }
# >      case IPN_CANCELED_REVERSAL {
# >      }
# >      case IPN_PENDING {
# >      }
# >      case IPN_FAILED {
# >      }
# >      case IPN_DENIED {
# >      }
# >      case IPN_REFUNDED {
# >      }
# >      case IPN_REVERSED {
# >      }
# >      else {
# >          croak "Unknown payment status $pp_vars{payment_status}. Don't know what to do.";
# >      }
# >  }
# > 
# > print CGI::header("text/plain");

1
