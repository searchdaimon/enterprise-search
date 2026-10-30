#!/usr/bin/env perl
# Common::FormFlow (cgi-bin/webadmin/Common/FormFlow.pm): the wizard step runner
use strict;
use warnings;
use Test::More tests => 8;
use FindBin;
use lib "$FindBin::Bin/../../../cgi-bin/webadmin";

use_ok('Common::FormFlow');

my $flow = Common::FormFlow->new;
isa_ok($flow, 'Common::FormFlow');

my @ran;
$flow->add(step1 => sub { push @ran, 'step1'; 'r1' })
     ->add(step2 => sub { push @ran, 'step2'; 'r2' })
     ->add(step3 => sub { push @ran, 'step3'; 'r3' });

is($flow->process('step2'), 'r2', 'process runs the handler for the named form and returns its value');
is_deeply(\@ran, ['step2'], 'only the named form ran');

is($flow->process_next, 'r3', 'process_next runs the form after the last processed one');

eval { $flow->process('nope') };
like($@, qr/not found/, 'an unknown form croaks');

eval { $flow->process(undef) };
like($@, qr/No form id/, 'a missing form id croaks');

eval { Common::FormFlow->new->add('x') };
like($@, qr/not defined/, 'add without a handler croaks');
