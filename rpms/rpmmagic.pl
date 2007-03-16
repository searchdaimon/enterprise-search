use strict;
use File::Copy;

print "$#ARGV\n";

if ($#ARGV == -1) {

}

my $name = shift @ARGV or die("please suply a name");
my $version = shift @ARGV or die("please suply a version");

my $source = shift @ARGV or die("please suply a source");
my $dest = shift @ARGV or die("please suply a dest");

my @files;
while ($#ARGV > -1) {
	my $file = shift @ARGV;
	push(@files,$file);
	print "file: $file\n";	
}

my $name_and_version = $name . '-' . $version;

#detlarerer mye brukte varibaler
my $command;

print "name: $name\n";
print "version: $version\n";
print "$source: $source\n";
print "$dest: $dest\n";

#mkdir -p $(boitho-ad_name)-$(boitho-ad_version)

#lager en mappe
mkdir($name_and_version,0755) or warn("cant create $name_and_version: $!");

#kopierer inn
#system("cp -r $source $name_and_version");



#create install list
my $filesinstal = '';
my $fileslist = '';

for my $i (@files) {
	my $filedest = $dest . '/' . $i;
	#$filesinstal .= "install -s -m 755 $i \$DESTDIR/$i\n";
	$filesinstal .= "install -D -m 755 $i \$DESTDIR/$i\n";
	$fileslist .= $filedest . "\n";
}
print "filesinstal:\n$filesinstal\n";
print "fileslist:\n$fileslist\n";

for my $i (@files) {

	my $filesource = $source . '/' . $i;
	my $filedest = $name_and_version . '/' . $i;

	my $folder = $filedest;
	$folder =~ s/[^\/]+$//;	

	$command = "mkdir -p $folder";
	print "running: $command\n";
	system($command);

	print "cp $filesource -> $filedest\n";

	copy($filesource, $filedest) or die "File cannot be copied: $!";
	
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




