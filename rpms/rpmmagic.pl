use strict;

print "$#ARGV\n";

if ($#ARGV == -1) {

}

my $name = shift @ARGV or die("please suply a name");
my $version = shift @ARGV or die("please suply a version");
my $dest = shift @ARGV or die("please suply a dest");

#my $source = shift @ARGV or die("please suply a source");

my $source = '';
while ($#ARGV > -1) {
	my $file = shift @ARGV;
	$source .= $file . ' ';
	print "geting\n";
}


my $name_and_version = $name . '-' . $version;

#detlarerer mye brukte varibaler
my $command;

print "name: $name\n";
print "version: $version\n";
print "source: $source\n";

#mkdir -p $(boitho-ad_name)-$(boitho-ad_version)

#lager en mappe
mkdir($name_and_version,0755) or warn("cant create $name_and_version: $!");

#kopierer inn
system("cp -r $source $name_and_version");


#lager en liste over alle filene
my $findout = `find $name_and_version -type f`;
print "findout: $findout\n";
my @files = split(/\n/,$findout);


#går gjenom hver linje of fjerner mappen
my $remove = $name_and_version . '/';

for my $i (0 .. $#files) {
	print "bb $i : $files[$i]\n";
	#ikke 100% riktig dette, skal ha ^ $ eller noe får å pare treff i begyndelsen. Har dog ikke g så skal jo bare treffe en gang, og alle skal jo starte på det
	$files[$i] =~ s/$remove//;
	print "file: $i : $files[$i]\n";

}

#create install list
my $filesinstal = '';
for my $i (@files) {
	$filesinstal .= "install -s -m 755 $i \$DESTDIR/$i\n";
}
#print "$filesinstal\n";

#create file list
my $fileslist = '';
for my $i (@files) {
	$fileslist .= "$dest/$i\n";
}


my $tarfile = $name_and_version . ".tar.gz ";
$command = "tar -z -c -f $tarfile $name_and_version";
print "running: $command\n";
system($command);

#flytter til redhat mappen
system("mv $tarfile /home/boitho/redhat/SOURCES/");

#lager spec file
open(INF,"defult.spec") or die("defult.spec: $!");

my @arr = <INF>;

close("INF");

my $spec = join("",@arr);


$spec =~ s/#name/$name/g;
$spec =~ s/#version/$version/g;
$spec =~ s/#filesinstal/$filesinstal/g;
$spec =~ s/#fileslist/$fileslist/g;
$spec =~ s/#destdir/$dest/g;

my $specfile = $name . ".spec";

open(OUT,">$specfile");
print OUT $spec;
close(OUT);




