#!/usr/bin/perl

#randomiserer urlkøen.

use String::CRC32;
use common qw(fin_domene);

use constant UrlQueuePostLength => 204;
use constant nrOfqueues => 5;

#make elementer man skal ha i minne fo hver kø. Når dette nåes lagres den køen på disk.
use constant maxQueuesElementsInMemory => 100;

our $terminated = 0;
my $queues = ();

if ($#ARGV == -1) {
	print qq{
Dette programet splitter UrlQueue i N (definert i: nrOfqueues) mindre 
køer basert på domene, som så merges, slik at urler fra samme domene 
ikke blir liggenede rett etter hverandre. Dette for å ungå at vi besøker
en server for ofte.

Bruk:
	perl splitUrlQueue.pl UrlQueueFil
};
	exit;
}

open(UDFILE,$ARGV[0]) or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);
flock(UDFILE,2);

my $ElementsInMemory = 0;
my $cont = 0;
$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
while ((!eof(UDFILE)) && (not $terminated)) {

	read(UDFILE,$post,UrlQueuePostLength);
	my ($url,$DocID) = unpack('A200,I',$post);
	
	my $UrlCRC32 = crc32(fin_domene($url));
	my $mod = $UrlCRC32 % nrOfqueues;
	#print "$mod - $url\n"; 
	
	push(@{ $queues[$mod] },$post);
	#push(@{ $queues[$mod] },$url);
	
	$ElementsInMemory++;
	
	if ($ElementsInMemory > maxQueuesElementsInMemory) {
		savequeue(@queues);
		#nuler ut køen nå nor den er lagret
		@queues = ();
		$ElementsInMemory = 0;
	}

	if (($cont % 100000) == 0) {
		print "Kommet til $cont\n";
	}

	$cont++;
}

close(UDFILE);

savequeue(@queues);






sub savequeue {
	my(@queue) = @_;
	#my $queuenr = shift;
	#my @queue = shift;

	print "saveing queus\n";
	
	#open(NEWUDFILE,'>>../data/UrlQueue') or die("Cant open ../data/UrlQueue: $!");
	#binmode(NEWUDFILE);
	#flock(NEWUDFILE,2);
	# Reset the file pointer to the end of the file, in case 
	# someone wrote to it while we waited for the lock...
	#seek(NEWUDFILE,0,2);


	#så lenge vi fortsetter å lese ut poster gjør vi det
	$posterigjen = 1;
	while ($posterigjen) {
	
	$posterigjen = 0;
	
		my $count = 0;
		foreach $i (@queue) {
		
			if (scalar(@{ $i} ) != 0) {
				my $post = shift(@{ $i});

				#### debug. vi urler som bli lag til
				my ($url,$DocID) = unpack('A200,I',$post);
				print "$count: $url\n";
				###

				
				#print NEWUDFILE $post;
				
				$posterigjen = 1;
			}
		
			$count++;
		}
	}
	
	#close(NEWUDFILE);
	

}

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
