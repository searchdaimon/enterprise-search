#!/usr/bin/env perl
# Compile-check (perl -c) every Perl source in the tree. This catches the class
# of error that only shows up as a 500 in the admin panel: a module that does
# not compile under the current perl (e.g. `for my $x qw(...)`, removed in 5.18).
#
# Files that are known not to compile outside the appliance are listed in
# syntax-skip.txt (one path per line, relative to the repo root) and reported
# as skipped rather than failed.
use strict;
use warnings;
use Test::More;
use File::Find;
use File::Spec;
use FindBin;

my $root = File::Spec->rel2abs("$FindBin::Bin/../../..");
chdir $root or die "chdir $root: $!";

my %skip;
if (open my $fh, '<', "$FindBin::Bin/../syntax-skip.txt") {
	while (<$fh>) {
		chomp;
		s/#.*//;
		s/^\s+|\s+$//g;
		$skip{$_} = 1 if length;
	}
}

my @dirs = grep { -d } qw(cgi-bin/webadmin public_html/webclient2 Modules crawlers perlxs/SD-Crawl/lib);
my @files;
find({
	no_chdir => 1,
	wanted => sub {
		return unless -f $_;
		return unless /\.(pm|pl|cgi)$/;
		return if m{/\.svn/} or m{/blib/} or m{/tpl/};
		push @files, File::Spec->abs2rel($_, $root);
	},
}, @dirs);

@files = sort @files;
plan tests => scalar @files;

# Include paths the CGIs rely on at run time. Each file's own directory goes
# first (the CGIs chdir there), so e.g. webclient2/config.pm wins over
# webadmin/config.pm for the web client sources. test/perl/lib holds a stub
# SD::Crawl, because the real XS module only loads inside crawlManager2.
my @roots = qw(cgi-bin/webadmin public_html/webclient2 Modules crawlers/Modules);

for my $file (@files) {
	SKIP: {
		skip "$file: listed in syntax-skip.txt", 1 if $skip{$file};
		my ($dir) = $file =~ m{^(.*)/[^/]+$};
		# The component the file belongs to comes right after its own directory,
		# so its config.pm shadows the other component's.
		my ($own) = grep { index($file, "$_/") == 0 } @roots;
		my @inc = map { "-I$_" } $dir, ($own ? $own : ()), "$FindBin::Bin/../lib", @roots;
		my $out = qx{perl -c @inc "$file" 2>&1};
		my $ok = $? == 0;
		ok($ok, "perl -c $file") or diag($out);
	}
}
