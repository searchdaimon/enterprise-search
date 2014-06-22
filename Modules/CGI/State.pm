package CGI::State;

use strict;
use integer;
use overload;
use CGI ();

use vars qw($VERSION);

$VERSION = (qw$Revision: 1.1 $)[-1];

#Returns a state hashref
sub state {
  my $class = shift;
  my $cgi   = shift;
  my $state = shift || {};

  #Alternative calling method is with a hash, rather
  #than a CGI object.  This allows validation before
  #building a multi-dimensional hash from submitted
  #values.
  if(ref $cgi eq 'HASH') {
    $cgi = CGI->new($cgi);
  }

  foreach my $param ($cgi->param) {

    my @words = split(/[\.\[\]]+/o, $param);
    my $node  = $state;

    for( my $w = 0;          #start at the first word
         $w < scalar @words; #go until the end
         $w++ ){             #increment the w count

      #If the next word is undefined, then we must
      #be looking at the last node.  If the next word
      #is a non-number, it must be a hashref, otherwise,
      #it is an arrayref.
      my $over_write = 0;

      my $next = (not defined $words[$w + 1])
        ? ($over_write++, $cgi->param($param))
        : $words[$w + 1] =~ /\D/
          ? {}
          : [];

      #Figures out if this is a reference to a hash,
      #array, an object or even an over-loaded object.
      my ($ref) = overload::StrVal($node) =~ /^(?:.*\=)?([^=]*)\([^\(]*\)$/o;

      $node = $ref eq 'HASH'
          ? $over_write
            ? ($node->{ $words[$w] }   = $next)
            : ($node->{ $words[$w] } ||= $next)
          : $over_write
            ? ($node->[ $words[$w] ]   = $next)
            : ($node->[ $words[$w] ] ||= $next);
    }
  }

  return $state;
}

#Takes the state, and returns a CGI object representing
#a "flattened" representation of state.
sub cgi {
  my $class  = shift;
  my $state  = shift;
  my $cgi    = shift || CGI->new('');
  my $param  = shift || '';

  #Figures out if this is a reference to a hash,
  #array, an object or even an over-loaded object.
  if(my $ref = ref $state) {

    #If it's really necessary, then use overload::StrVal
    unless($ref eq 'HASH' or $ref eq 'ARRAY') {
      $ref = overload::StrVal($state) =~ /([^=]*)\(/o;
    }

    if($ref eq 'ARRAY') {

      for( my $index = 0;           #start at the first word
           $index < scalar @$state; #go until the end
           $index++ ){
        $class->cgi( $state->[$index], $cgi, "$param\[$index\]" );
      }

    } elsif($ref eq 'HASH') {

      $param .= '.' unless $param eq '';

      foreach my $key (keys %$state) {
        $class->cgi( $state->{$key}, $cgi, $param.$key );
      }

    }

  } elsif(defined $state and $param ne '') {

    $cgi->param($param, $state);

  }

  return $cgi;
}

1;

__END__

=head1 NAME

CGI::State - Converts CGI parameters into a multi-dimensional hash

=head1 SYNOPSIS

First you make your HTML form to submit to your CGI script.  For example:

  <form action="order.cgi">
    <input type="text" name="Contact.first_name" value="Dan" />
    <input type="text" name="Contact.email" value="dan@mealtips.com" />
    <input type="hidden" name="item[0].price" value="10.00" />
    <input type="hidden" name="item[0].description" value="Widget" />
    <input type="submit" value="Order Now!" />
  </form>

Notice the names of the hidden and text fields? Keep this in mind.  Then
you create a CGI script to receive the form variables after they're submitted:

  #!/usr/bin/perl -wT

  use strict;
  use CGI;
  use CGI::State;
  use Data::Dumper;

  #Simulate receiving CGI parameters
  my $cgi = CGI->new;

  #Un-Flatten the data structure
  my $state = CGI::State->state( $cgi );

  #Show us what the $state hashref now looks like...
  print $cgi->header('text/plain'), Data::Dumper->new([$state], [qw(state)])->Indent(3)->Quotekeys(0)->Dump;

  #Which would print out the following data structure:
  $state = {
             Contact => {
                          first_name => 'Dan',
                          email => 'dan@mealtips.com'
                        },
             item => [
                       #0
                       {
                         description => 'Widget',
                         price => '10.00'
                       }
                     ]
           };

  #Also, you can change $state back into the original query string:
  $cgi = CGI::State->cgi( $state );

  print $cgi->query_string;

  #Which would print out: (in no particular order)
  Contact.first_name=Dan&Contact.email=dan@mealtips.com&item[0].price=10.00&item[0].description=Widget

=head1 DESCRIPTION

This module takes incoming B<CGI form variables>, and transforms
them into a B<multi-dimensional data structure>.  It can recreate
a hash of hashes, a hash of lists, a hash of lists of hashes
etc, any number of levels deep.

A limitation of CGI is it's inability to naturally group
together submitted variables.  For example, you can't have
someone fill in an order form and have all their contact and
item information grouped separate from each other in a perl
data structure without specifically doing this in the perl
code.

This module was originally written because I always hated
receiving CGI parameters, putting them into a hash, and have
this hash contain 20 or more elements.  I think it's messy,
and very tedious writing code to group related CGI parameters
together.  I wanted parameters to be put into a
multi-dimensional data structure automatically for me.

=head1 METHODS

=over 4

=item $state = CGI::State-E<gt>state( $cgi )

This routine takes one argument, a CGI.pm object reference.

It will return a hashref containing as many levels as
specified in the input parameters.

It allows you to logically group together form
elements, so that when the CGI script receives them, it
has to do no logic of it's own to group things together.

This routine cycles through all the form variables
and looks for the following format:

  $object_name[$index].$attribute <-- Multivalued

  OR

  $object_name.$attribute         <-- Single valued

  ...And translates them into the following:

  $hashref = {

    $object_name => [

      #$index = 0
      { $attribute => $value },

      #$index = 1
      { $attribute => $value },

      #...

    ],
  }

It also puts all non-objects into the top-level
of the hashref.

The format I choose to describe the data structure closely
resembles Javascript's style.  I know, I know. But the syntax
is close to perl, and should be fairly simple for most perl
programmers to pick up.  In the future I may change this module
to allow you to specify different delimeters, to allow more
perl-like syntax.

=item $cgi = CGI::State-E<gt>cgi( $state )

This routine takes one argument, a multi-dimensional hash.

It will return a I<flattened> CGI object based on the values
referenced by $state.  Very useful for maintaining state
across various CGI invocations.

=back

=head1 EXAMPLES

One major advantage to grouping parameters together in a
multi-dimensional data structure is that you have
everything I<map> into your database cleanly.

For example, let's say that we have a relational table
called  I<Contact>, which stores information about
a customer.  Inside this table there are three
columns called first_name, last_name, and email.

Imagine there is a form where customer information
is collected, such as the following:

  <form action="save_customer.cgi">
    <input type="text" name="Contact.first_name" />
    <input type="text" name="Contact.last_name" />
    <input type="text" name="Contact.email"  />
  </form>

When this form is submitted, we create a CGI.pm object
to capture the data, then pass this object off to
CGI::State-E<gt>state, which returns a hash reference:

  my $cgi   = CGI->new;           #Create the CGI Object
  my $state = CGI::State( $cgi ); #$state is a hash reference

Assuming that we submit the form, the hash reference
would look like this, as shown by Data::Dumper:

  $state = {
             Contact => {
                          first_name => 'Dan',
                          last_name  => 'Kubb'
                          email      => 'dan@mealtips.com'
                        },
  };

With this structure, it would be rather easy just
to pass off $state-E<gt>{Contact} to a subroutine
that inserts Contact information into a database.
There's no sorting, grouping or hard-coding the
column names anywhere in your code!  I am a firm
believer that the database table and column names
should dictate the HTML form parameter names, and
perl hash element names.  This module helps enforce
that and make it easier to write code that will
"map" HTML forms into a database with minimal effort.

=head1 LIMITATIONS

Having the ability to manipulate the data with simple HTML
can either be seen as a benefit or a liability. I find it a
benefit, but I can imagine that others might not see it that
way, especially when non-programmers are responsible for
constructing all web forms.

Anything you expect to be a hash key should not be a number.

=head1 TODO

=over 4

=item *

Add more security measures and error checking.

=back

=head1 SEE ALSO

L<CGI>

=head1 AUTHOR

Copyright 2001, Dan Kubb <dan@mealtips.com>

This module is distributed under the same terms as Perl itself.  Feel
free to use, modify and redistribute it as long as you retain the
correct attribution.

=cut