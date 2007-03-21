#!/usr/bin/perl -w

BEGIN {
  use strict;
  use Carp qw(verbose);
  $SIG{__WARN__} = \&Carp::confess;  #Be VERY honest
}

use ExtUtils::MakeMaker;

#Base Make File Config
my %config = (
  NAME         => 'CGI::State',
  VERSION_FROM => 'State.pm',
  PREREQ_PM    => {
    strict   => 0,
    integer  => 0,
    overload => 0,
    CGI      => 0,
    vars     => 0,
  },
);

#Required to allow this module to build into a PPM package
#As specified at:
#http://www.activestate.com/Products/ActivePerl/docs/faq/ActivePerl-faq2.html#how_to_make_ppm_distribution
if($] ge '5.005') {
  $config{AUTHOR}   = 'Dan Kubb (dkubb@cpan.org)';
  $config{ABSTRACT} = 'Converts CGI parameters into a multi-dimensional hash';
}

#Default Install Config
ExtUtils::MakeMaker::WriteMakefile(%config);