#!/usr/bin/env perl
# Common::Validate (cgi-bin/webadmin/Common/Validate.pm)
use strict;
use warnings;
use Test::More tests => 5;
use FindBin;
use lib "$FindBin::Bin/../../../cgi-bin/webadmin";

use_ok('Common::Validate');

ok(valid_numeric({ id => 4 }), 'a numeric value is accepted');
ok(valid_numeric({ id => 4, page => 10 }), 'several numeric values are accepted');

eval { valid_numeric({ id => 'hello!' }) };
like($@, qr/not numeric/, 'a non-numeric value croaks');

eval { valid_numeric({ id => undef }) };
like($@, qr/not defined/, 'an undefined value croaks');
