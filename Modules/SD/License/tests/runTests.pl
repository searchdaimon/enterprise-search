use strict;
use warnings;

use Test::Harness qw(&runtests $verbose);
$verbose = 1;
runtests("Server.t", "Client.t");

