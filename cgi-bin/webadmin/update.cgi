#!/usr/bin/perl
use strict;
use warnings;

my $command = $ENV{'BOITHOHOME'} . '/setuid/yumupdate';


print `$command`;

