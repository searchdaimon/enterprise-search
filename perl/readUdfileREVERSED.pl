
use constant UrlQueuePostLength => 204;

our $terminated = 0;

use Boitho::Lot;
use Boitho::DocumentIndex;

if ($#ARGV == -1) {
        print qq{
Bruk:
        perl readUdfile.pl UrlQueueFil
};
        exit;
}

Boitho::DocumentIndex::popopen("/home/boitho/config/popindex");

open(UDFILE,$ARGV[0]) or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);
flock(UDFILE,2) or die($!);

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
my $posisjon = UrlQueuePostLength;



while (not $terminated) {

	#print "Posisjon $posisjon\n";
	#søker en posisjon opp
	#seek(UDFILE, - UrlQueuePostLength,SEEK_CUR);
	seek(UDFILE,0 - $posisjon,2);
	$posisjon =  $posisjon + UrlQueuePostLength;

        read(UDFILE,$post,UrlQueuePostLength) or die("Can't read at $posisjon: $!");

        my ($url,$DocID) = unpack('A200,I',$post);
	my $rank = Boitho::DocumentIndex::popRankForDocID($DocID);


        print "$DocID-" .  Boitho::Lot::rLotForDOCid($DocID) ." $url, rank: $rank\n";
	#print "$url\n";
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

