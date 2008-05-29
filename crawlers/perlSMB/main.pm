use strict;

package Perlcrawl;
use SD::Crawl;

use Filesys::SmbClient;

sub recursdir {

	my($pointer,$smb,$dirname) = @_;

	print "recursdir($dirname)\n";

	my $fd = $smb->opendir($dirname) or warn("can't open dir \"$dirname\"");
  	while (my $f = $smb->readdir_struct($fd)) {
		print "$f->[1]\n";
		#tar ikke med . .. og filer som begynner med .
		if ($f->[1] =~ /^\./) {
			next;
		}

		my $enerytpath = $dirname . "/" . $f->[1];

	    	if ($f->[0] == SMBC_DIR) {
			print "recursdir: Directory ",$f->[1],"\n";
			recursdir($pointer,$smb,$enerytpath);
		}
    		elsif ($f->[0] == SMBC_FILE) {
			if (not SD::Crawl::pdocumentExist($pointer, $enerytpath, 0, 0 )) {

				print "recursdir: File: ",$f->[1],"\n";
				my $fileh = $smb->open($enerytpath, 0666) or print "Can't read file:", $!, "\n";
				my $filecontent = '';
				my $buf;
				while(defined($buf = $smb->read($fileh))) {
					last if $buf eq '';
					$filecontent .= $buf;
					#print "recursdir: length: " . length($filecontent) . "\n";
				}
				$smb->close($fileh);

				#send the fil to the search system
				SD::Crawl::pdocumentAdd($pointer, $enerytpath, 0 ,length($filecontent), $filecontent, $f->[1], "", "Everyone", "");		

			}
		}
  	  	
  	}
  	$smb->closedir($fd);

}

sub crawlupdate {
	my ($self, $pointer, $opt ) = @_;

	print "crawlupdate(pointer=$pointer)\n";

	print "Options:\n";
	foreach my $i (%{ $opt} ) {
		print "$i: \"$opt->{$i}\"\n";
	}


	my $smb = new Filesys::SmbClient(username  => $opt->{'user'},
                                       password  => $opt->{'password'},
                                       debug     => 0);


	recursdir($pointer, $smb, $opt->{'resource'} );



}

sub crawlpatAcces {
	my ($self, $pointer, $opt ) = @_;

	print "crawlpatAcces(pointer=$pointer)\n";

	print "Options:\n";
	foreach my $i (%{ $opt} ) {
		print "$i: \"$opt->{$i}\"\n";
	}

	#setter opp tilkoblingen
	my $smb = new Filesys::SmbClient(username  => $opt->{'user'},
                                       password  => $opt->{'password'},
                                       debug     => 0);
	
	#gjør det faktiske open() kallet mot resursen.
	my $fd = $smb->open($opt->{"resource"},0666);
	
	if (!$fd) {
		#hvis vi ikke fikk til å åpne filen skal vi returnere 0
		warn("can't open $opt->{'resource'} for user $opt->{'user'}: $!");
		return 0;
	}

  	$smb->closedir($fd);

	return 1;
}

