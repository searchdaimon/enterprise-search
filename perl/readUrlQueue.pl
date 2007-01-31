use String::CRC32;
use common qw(fin_domene);

use constant UrlQueuePostLength => 204;
use constant nrOfqueues => 2500;

#make elementer man skal ha i minne fo hver kø. Når dette nåes lagres den køen på disk.
use constant maxQueuesElementsInMemory => 30000;

our $terminated = 0;
my $queues = ();

if ($#ARGV == -1) {
        print qq{
Dette programet leser UrlQueue`en

Bruk:
        perl readUrlQueue.pl UrlQueueFil
};
        exit;
}

open(UDFILE,$ARGV[0]) or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);

my $ElementsInMemory = 0;
$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
while ((!eof(UDFILE)) && (not $terminated)) {

        read(UDFILE,$post,UrlQueuePostLength);
        my ($url,$DocID) = unpack('A200,I',$post);
	
	print "$DocID : $url\n";
}
close(UDFILE);

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


