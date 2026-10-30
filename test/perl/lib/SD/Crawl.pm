package SD::Crawl;
# Compile-time stand-in for the real SD::Crawl XS module (perlxs/SD-Crawl), used
# ONLY by test/perl/t/00-syntax.t. The real module can only be loaded inside
# crawlManager2's embedded interpreter (its .so needs symbols the daemon
# provides), so `perl -c` of anything that `use SD::Crawl` would otherwise
# fail. The real module exports nothing by default, so neither does this.
use strict;
use warnings;
use Exporter qw(import);
our @EXPORT = ();
our @EXPORT_OK = ();
our $VERSION = '0.01';
1;
