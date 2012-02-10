package Perlcrawl;
use Carp;
use Data::Dumper;
use strict;
use warnings;
use Filesys::SmbClient;

use Crawler;
our @ISA = qw(Crawler);

sub recursdir {

	my($sd,$smb,$dirname) = @_;

	print "recursdir($dirname)\n";

	my $fd = $smb->opendir('smb://' . $dirname) or warn("can't open dir \"$dirname\": $!") && return 0;
  	while (my $f = $smb->readdir_struct($fd)) {
		print "$f->[1]\n";
		#tar ikke med . .. og filer som begynner med .
		if ($f->[1] =~ /^\./) {
			next;
		}

		my $enerytpath = $dirname . "/" . $f->[1];

	    	if ($f->[0] == SMBC_DIR) {
			print "recursdir: Directory ",$f->[1],"\n";
			recursdir($sd,$smb,$enerytpath);
		}
    		elsif ($f->[0] == SMBC_FILE) {

			if (not $sd->document_exists($enerytpath, 0, 0 )) {

				print "recursdir: File: ",$f->[1],"\n";
				my $fileh = $smb->open($enerytpath, 0666) or print "Can't read file:", $!, "\n";
				my $filecontent = '';
				my $buf;
				while(defined($buf = $smb->read($fileh))) {
					last if $buf eq '';
					$filecontent .= $buf;
				}
				$smb->close($fileh);

				#send the fil to the search system
				$sd->add_document((
			            content   => $filecontent,
			            title     => $f->[1],
			            url       => $enerytpath,
			            acl_allow => "Everyone", # permissions
			            last_modified => 0, # unixtime
			        ));
			}

		}
  	  	
  	}
  	$smb->closedir($fd);

	return 1;
}

sub crawl_update {
	my (undef, $self, $opt) = @_;


	print "Options:\n";
	foreach my $i (keys %{$opt} ) {
		print "$i: \"" . $opt->{$i} . "\"\n";
	}


	my $smb = new Filesys::SmbClient(username  => $opt->{'user'},
                                       password  => $opt->{'password'},
                                       debug     => 0);

	#Corect format is //host/shares not \\host\shares. Convertinig "\" to "/"
	$opt->{'resource'} =~ s/\\/\//g;

	# do a test connect
	my $fd = $smb->opendir('smb://' . $opt->{'resource'}) or die("can't open dir \"" . $opt->{'resource'} . "\": $!");
  	$smb->closedir($fd);

	# crawl it
	recursdir($self, $smb, $opt->{'resource'} );


}

sub path_access {
	my ($undef, $self, $opt) = @_;


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

