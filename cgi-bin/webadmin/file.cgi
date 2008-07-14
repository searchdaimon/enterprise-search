#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use CGI::State;
BEGIN {
	#push @INC, 'Modules';
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::WebFile qw(get);


# Description:
my $usage = qq^
	     = Images =
	     Usage: file.cgi?icon=ok_button&size=32x32&ext=png
	     Defaults to size 32x32, extension png. Same as file.cgi?icon=ok_button

	     = CSS =
	     Usage: file.cgi?css=default
	     ^;

my @valid_size = qw(16x16 22x22 32x32 48x48 64x64 128x128 other);
my @valid_ext  = qw(png jpg jpeg gif);

Boitho::WebFile::get(['image', 'css', 'javascript', 'swf'], \@valid_size, \@valid_ext);

