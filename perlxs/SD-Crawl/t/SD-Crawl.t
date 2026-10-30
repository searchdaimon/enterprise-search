# Tests for the SD::Crawl XS module. Run from perlxs/SD-Crawl after the module
# has been built (`make` there, or `make perlxs-sdcrawl` at the top level):
#
#     prove -b t
#
# The .so references symbols (e.g. global_bbdnport) that only exist inside the
# crawlManager2 process that normally hosts this module, so outside the daemon
# it cannot be loaded at all. In that case the whole file is skipped rather than
# failed; if the module ever becomes standalone-loadable the tests below run.
use strict;
use warnings;
use Test::More;

my $loaded = eval { require SD::Crawl; 1 };
if (!$loaded) {
	my $err = $@;
	plan skip_all => "SD::Crawl only loads inside crawlManager2 (undefined symbol)" if $err =~ /undefined symbol/;
	plan skip_all => "SD::Crawl is not built (run make in perlxs/SD-Crawl)" if $err =~ /Can't locate|Can't load/;
	die $err;
}
plan tests => 5;

is(SD::Crawl::htttp_url_normalization('http://boitho.com'),
	'http://www.boitho.com/', 'bare domain gets www. and a trailing slash');
is(SD::Crawl::htttp_url_normalization('http://www.boitho.com:80/File.html'),
	'http://www.boitho.com/File.html', 'default port is dropped');
is(SD::Crawl::htttp_url_normalization('HTTP://www.Boitho.coM/File.html'),
	'http://www.boitho.com/File.html', 'scheme and host are lower-cased');
is(SD::Crawl::htttp_url_normalization('http://www.boitho.com/side.html#august'),
	'http://www.boitho.com/side.html', 'fragment is removed');
is(SD::Crawl::htttp_url_normalization('http://www.boitho.com/File.html'),
	'http://www.boitho.com/File.html', 'a canonical URL is unchanged');
