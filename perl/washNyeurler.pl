use strict;
use common qw(gyldig_url);

use Boitho::Reposetory;
#struct updateFormat
#{
#    unsigned char       sha1[20];
#    unsigned char       url[200];
#    unsigned char       linktext[50];
#    unsigned int        DocID_from;
#};

use constant updateFormat => "A20 A200 A50 A2 I";
use constant updateFormatLength => 276;

use constant subname => "www";

if ($#ARGV == -1) {
        print qq{
Dette programet fjerner ugyldige urler fra en Nyeurler filer.

Bruk:
        ./washNyeurler.pl nyeurler [.. n]

};
        exit;
}

for my $i (0 .. $#ARGV) {

	my $file = $ARGV[$i];
	my $newname = $file . '_pwashed';



	print "$i $file -> $newname\n";

	open(INF,$file) or die($!);

	my $lasturl = '';
	while (!eof(INF)) {
		my $post;	
		read(INF,$post,updateFormatLength) or warn($!);

		my ($sha1,$url,$linktext,undef,$DocID_from) = unpack(updateFormat,$post);


		if ((gyldig_url($url)) && ($url ne $lasturl)) {	
			#print "url $url, DocID $DocID_from\n";
			Boitho::Reposetory::addNewUrl($url,$linktext,$DocID_from,$newname,subname);
			$lasturl = $url;
		}
		else {
			#print "NO: $url\n";
		}
	}
	close(INF);

	unlink($file);
}
