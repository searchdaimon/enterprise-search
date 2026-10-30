#!/usr/bin/env perl
# SD::ISO639 (Modules/SD/ISO639.pm): language code table used by the web UI
use strict;
use warnings;
use Test::More tests => 6;
use FindBin;
use lib "$FindBin::Bin/../../../Modules";

BEGIN { use_ok('SD::ISO639') }

is($ISO639_TWO_LETTER{no}, 'Norwegian', 'no => Norwegian');
is($ISO639_TWO_LETTER{en}, 'English', 'en => English');
is($ISO639_TWO_LETTER{da}, 'Danish', 'da => Danish');
ok(!exists $ISO639_TWO_LETTER{xx}, 'unknown code is absent');

eval { $ISO639_TWO_LETTER{no} = 'changed' };
like($@, qr/read-only/i, 'the table is read-only');
