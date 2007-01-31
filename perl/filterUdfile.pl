
use constant UrlQueuePostLength => 204;

our $terminated = 0;



if ($#ARGV != 2) {
        print qq{
Bruk:
        perl readUdfile.pl UrlQueueFilFrom UrlQueueFilMatch UrlQueueFilNoMach
};
        exit;
}


open(UDFILE,$ARGV[0]) or die("Cant open $ARGV[0]: $!");
binmode(UDFILE);

open(UDFILEMatch,">>$ARGV[1]") or die("Cant open $ARGV[1]: $!");
binmode(UDFILEMatch);

open(UDFILENoMatch,">>$ARGV[2]") or die("Cant open $ARGV[2]: $!");
binmode(UDFILENoMatch);

$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
while ((!eof(UDFILE)) && (not $terminated)) {

        read(UDFILE,$post,UrlQueuePostLength);

        my ($url,$DocID) = unpack('A200,I',$post);

	if ($url =~ /\.no\//) {
        	#print "$DocID $url\n";
		print UDFILEMatch $post;
	}
	else {
		print UDFILENoMatch $post;
	}
	#print "$url\n";
}

close(UDFILE);
close(UDFILEMatch);
close(UDFILENoMatch);

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

