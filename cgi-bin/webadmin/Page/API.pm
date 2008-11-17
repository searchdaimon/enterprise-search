##
# Common methods for javascript API
package Page::API;
use strict;
use warnings;
use Carp;

sub api_error {
    my ($s, $api_vars, $err) = @_;
    if ($err) {
        carp $err;
        $err =~ s/at .* line \d+$//g;
        $api_vars->{error} = $err;
        return 1;
    }
    return;
}

1;
