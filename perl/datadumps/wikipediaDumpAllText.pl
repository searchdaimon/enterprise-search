  
use File::Path;
#use utf8;
#binmode STDOUT, ":utf8";

#use Unicode::MapUTF8 qw(to_utf8 from_utf8 utf8_supported_charset);
#use strict;
use Parse::MediaWikiDump;
#use warnings;

my $timeout = 60; #1 min

    
my $file = shift(@ARGV) or die "must specify a Mediawiki dump file";
my $maindir = shift(@ARGV) or die "must specify a dir to save in";
my $pages = Parse::MediaWikiDump::Pages->new($file);
my $page;
  

$|++;







my $openlist = 0;
my $count = 0;
while(defined($page = $pages->next)) {



#main namespace only 
#	print "namespace: " . $page->namespace . "\n";          
	next unless $page->namespace eq '';
#	print "categories: ". $page->categories ."\n";
#print $count, ": ", $page->title, "\n" unless defined($page->categories);

#if (!defined($page->categories)) {
	my $namespace = $page->namespace;
	my $id = $page->id;
	my $title = $page->title;
	my $text = $page->text;

#$title = from_utf8(-string =>$title, -charset => 'ISO-8859-1');

#fjerner ting som ikke er lov i filnavnet
	$filename = $title;
	$filename =~ s/[^a-zA-Z0-9 ]/_/g;

	my $dir = $filename;
	$dir =~ s/[^a-zA-Z0-9]//g;
#	$dir =~ s/(..)/xx/;

#	next unless (length($dir) < 4);

	$dir =~ /(.)(.)?(.)?(.)?/;
	my ($a, $b, $c, $d) = ($1, $2, $3, $4);

#	print "$a $b $c $d\n";


#	$dir = $1;
	$a = lc($a);
	$b = lc($b);
	$c = lc($c);
	$d = lc($d);
	$dir = $maindir . "/$a";
	$dir .= "/$b" if defined $b;
	$dir .= "/$c" if defined $c;
	$dir .= "/$d" if defined $d;
	mkpath($dir) unless (-d $dir);
	$filename = $dir . "/" . $filename . ".txt";

#		print $count . ' '. $filename . ', len: ' . length($$text) . "\n";

	if (-e $filename) {
		print STDERR "file exists: $filename\n";
		next;
	}

	eval {
		local $SIG{ALRM} = sub { die "alarm\n" };       # NB \n required
			alarm $timeout;



		my @lines = split(/\n/,$$text);

		my $text = '';

		foreach my $i (@lines) {
			$i =~ s/\[\[([^\|]+\|)([^\]]+)\]\]/$2/g;
			#honterer linker med annen link en beskrivelse
			$i =~ s/\[\[([^\]]+)\]\]/$1/g;

			#honterer linker
			$i =~ s/\[\[([^\]]+)\]\]/$1/g;

			#seltter overskrift makup
			$i =~ s/'//g;

			$i =~ s/=//g;
			$text .= $i . "\n";				
#				print $i, "\n";

		} #/foreach

		open(OUT,">" . $filename) or die("Can't write $filename: $!");
		binmode(OUT, ':utf8');
		print OUT $text, "\n";
		close(OUT);

		alarm 0;
	};
	die if $@ && $@ ne "alarm\n";       # propagate errors
		if ($@) {
			warn("did time out.");
		}

	++$count;
#}

	if (($count % 1000) == 0) {
		print "$count\r";
	}
}

print "\n";


