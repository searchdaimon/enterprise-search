
use constant UrlQueuePostLength => 204;

our $terminated = 0;

use Boitho::Lot;
use Boitho::DocumentIndex;
use common qw {fin_domene};

if ($#ARGV == -1) {
        print qq{
Bruk:
        perl readUdfile.pl UrlQueueFil
};
        exit;
}

Boitho::DocumentIndex::popopen();

open(UDFILE,$ARGV[0]) or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
while ((!eof(UDFILE)) && (not $terminated)) {

        read(UDFILE,$post,UrlQueuePostLength);

        my ($url,$DocID) = unpack('A200,I',$post);

	my $domain = fin_domene($url);
	print "$domain\n";

}

close(UDFILE);

Boitho::DocumentIndex::popclose();

############################################################################################################################
# hånterer signaler
############################################################################################################################
sub signal_handler {
        if (not $terminated) {
                $terminated = 1;

                print "\n\aOk, begynner og avslutte\n\n";
        }
        else {
                die("Motok killsignal nr to, dør");
        }
}

