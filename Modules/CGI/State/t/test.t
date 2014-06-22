#!/usr/bin/perl -w

#Need to execute things in this begin block
#in order to ensure that debug information is
#printed after modules are loaded.
BEGIN {

  $|++;

  use Test;
  plan(tests => 4);
  print "\nBegin Testing...\n\n";
  print "\n\t- Test.pm loaded successfully (used to simplify testing)\n\n" if ok(1);

  use strict;
  use Carp qw(verbose);
  $SIG{__WARN__} = \&Carp::confess; #Super strict

  use CGI qw(-no_debug);
  print "\n\t- CGI.pm loaded successfully\n\n" if ok(1);

  use CGI::State;
  print "\n\t- CGI::State loaded successfully\n\n" if ok(1);
}

#Define a test CGI object, and simulate incoming data
my $cgi = CGI->new('products[0].products_id=1010&products[0].quantity=2&products[1].products_id=1020&products[1].quantity=3&products[2].products_id=1030&products[2].quantity=4&ontact.firstname=Dan&contact.lastname=Kubb&contact.company=my+company&contact.city=Vancouver&contact.state=BC&contact.country=Canada');

print "\n\t- Translation between CGI::State and CGI.pm tested\n\n" if ok(sub { translate($cgi) }, 0);

#Subroutines

=head2 translate($cgi)

This routine receives a CGI.pm object.  It will
return a 1 or a 0 depending on if CGI::State
does the translation between CGI.pm and itself.

=cut

sub translate {
  my $old_cgi = shift;

  my $state   = CGI::State->state($old_cgi);
  my $new_cgi = CGI::State->cgi($state);

  return scalar grep { $old_cgi->param($_) ne $new_cgi->param($_) } $old_cgi->param;
}