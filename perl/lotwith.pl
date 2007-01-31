
if ($#ARGV == -1 ) {
	print "Spesifiser en fil å lete etter\n";
	exit;
}


print "Leter etter loter med $ARGV[0]\n";

 

       
        use File::Find;

       my @lots = ();
	
        open(INF,'../maplist.conf') or die("Cant open ../maplist.conf: $!");

        my @a = <INF>;

        foreach my $MainlotPath (@a) {
                chomp($MainlotPath);

                #print "$MainlotPath\n";

                if (-e $MainlotPath) {
                        #nå må en lot ha reposetory får å bli med. Er det riktig ?
                        my $comand = "ls -1 -d $MainlotPath/*/$ARGV[0]";

                        #print "comand: $comand\n";
                        my $output = `$comand`;

                        my @lotpath = split("\n",$output);

                        foreach my $lotPath (@lotpath) {
                                #$lotPath =~ s/\/reposetory//i;
                                #print "-$lotPath-\n";
				push(@lots,$lotPath);                                
                        }

                }

        }
        close(INF);

foreach my $i (@lots){

	print "$i\n";
}



