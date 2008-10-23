use strict;
use File::Copy;

print "$#ARGV\n";

if ($#ARGV == -1) {

}

  use Getopt::Long;
  my $pre = '';
  my $post = '';
  my $initd;
  my $initdnostart;
  my $verbose;
  my $requires;
  my $sql;
  my $initdrestart;
  my $defattr;
  my $restartsw;
  my $sysconfig;
  my $defines = '';
  my $nostrip;

  my $result = GetOptions ("pre=s" 	 => \$pre,    	# rpm pre 
                        "post=s"   	 => \$post,      	# rpm post
                        "initd=s"   	 => \$initd,      	# rpm post
                        "initdnostart"   	 => \$initdnostart,      	# rpm post
                        "restartsw"   	 => \$restartsw,      	# rpm post
			"requires=s" 	 => \$requires, 	# rpm requires
			"sql=s" 	 => \$sql, 		# rpm sql
			"initdrestart=s" => \$initdrestart, 		# restarting av en init.d tjeneste
			"defattr=s" => \$defattr,
			"sysconfig=s" => \$sysconfig,
			"nostrip"	=> \$nostrip,
			"verbose"  	 => \$verbose);

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
#legger sql filen til som en vanlig fil
if (defined($sql)) {
	push(@files,$sql);
}
#legger til init.d funksjonsfilen, som init.d trenger
if (defined($initd)) {
	push(@files,"init.d/functions");
}

if (defined($sysconfig)) {
	push(@files, "sysconfig/rpcbind");
}

my $name_and_version = $name . '-' . $version;

#detlarerer mye brukte varibaler
my $command;

print "name: $name\n";
print "version: $version\n";
print "$source: $source\n";
print "$dest: $dest\n";

#mkdir -p $(boitho-ad_name)-$(boitho-ad_version)

#sletter gammel mappe
#toDo Er dette farlig. Kan vi ta en dagur her???
$command = "rm -rf -v \"$name_and_version\"";
print "running: $command\n";
system($command);


#lager en mappe
mkdir($name_and_version,0755) or warn("cant create $name_and_version: $!");

#kopierer inn
#system("cp -r $source $name_and_version");



#create install list
my $filesinstal = '';
my $fileslist = '';

for my $i (@files) {
	my $filedest = $dest . '/' . $i;

	$filesinstal .= "cp -r --parents $i \$DESTDIR/\n";

	$fileslist .= $filedest . "\n";
}
if (defined($initd)) {
	$filesinstal .= "install -D -m 755  init.d/$initd \$RPM_BUILD_ROOT/etc/init.d/$initd\n";
	$fileslist .=  "/etc/init.d/$initd\n";
}

if (defined($sysconfig)) {
	$filesinstal .= "install -D -m 755  sysconfig/$sysconfig \$RPM_BUILD_ROOT/etc/sysconfig/$sysconfig\n";
	$fileslist .=  "/etc/sysconfig/$sysconfig\n";
}


print "filesinstal:\n$filesinstal\n";
print "fileslist:\n$fileslist\n";

if (defined($nostrip)) {
	$defines .= qq {
# Disable rpm from stripping XFree86's modules, etc...
%define __spec_install_post /usr/lib/rpm/brp-compress
	};
}

if (defined($initd)) {

	#lager hva som skal kjøres før vi instalerer
	$pre .= qq{
if [ -f /etc/init.d/$initd ] ; then
	sh /etc/init.d/$initd stop
fi
	};

	$post .= qq{

	#run chkconfig to add it to rc
	chkconfig --add $initd

	};

	if (not $initdnostart) {
		$post .= qq{

		#start it
		sh /etc/init.d/$initd start

		};
	}
}
if (defined($restartsw)) {
$post .= qq{

#restarter all sd programer
if [ -f /etc/init.d/boithoad ] ; then
        sh /etc/init.d/boithoad restart
fi

if [ -f /etc/init.d/boitho-bbdn ] ; then
        sh /etc/init.d/boitho-bbdn restart
fi

if [ -f /etc/init.d/crawlManager ] ; then
        sh /etc/init.d/crawlManager restart
fi

if [ -f /etc/init.d/searchdbb ] ; then
        sh /etc/init.d/searchdbb restart
fi

}
}
if (defined($initdrestart)) {
	$post .= qq{

	#start it
	sh /etc/init.d/$initdrestart restart

	};

}

if (defined($sql)) {
	my $filedest = $dest . '/' . $sql;


	$post .= "mysql --force boithobb < $filedest\n";
}

for my $i (@files) {

	my $filesource = $source . '/' . $i;
	my $filedest = $name_and_version . '/' . $i;

	my $folder = $filedest;
	$folder =~ s/[^\/]+$//;	


	$command = "mkdir -p $folder";
	print "running: $command\n";
	system($command);

	print "cp $filesource -> $filedest\n";

	#copy($filesource, $filedest) or die "File cannot be copied: $!";
	$command = "cp -r $filesource $filedest";
	print "running: $command\n";
	system($command);
	
	
}
if (defined($initd)) {
	my $filesource = $source . '/init.d/' . $initd;
	my $filedest = $name_and_version . '/init.d/' . $initd;

	my $folder = $filedest;
	$folder =~ s/[^\/]+$//;	
	
	$command = "mkdir -p $folder";
	print "running: $command\n";
	system($command);

	print "cp $filesource -> $filedest\n";

	#copy($filesource, $filedest) or die "File cannot be copied: $!";
	$command = "cp -r $filesource $filedest";
	print "running: $command\n";
	system($command);
	

}
my $tarfile = $name_and_version . ".tar.gz ";
$command = "tar -z -c -f $tarfile $name_and_version";
print "running: $command\n";
system($command);

#flytter til redhat mappen
system("mv $tarfile ~/redhat/SOURCES/");

#prefikser
if ($requires ne '') {
	my $tmp = '';
	foreach my $i (split(/ /,$requires)) {
		$i =~ s/>=/ >= /g;
		$i =~ s/==/ == /g;
		$tmp .= "requires: " . $i . "\n";
	}
	$requires = $tmp;
	#$requires = "requires: " . $requires;
}

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


$spec =~ s/#requires/$requires/g;

$spec =~ s/#rpm_pre/$pre/g;
$spec =~ s/#defines/$defines/g;
$spec =~ s/#rpm_post/$post/g;
$spec =~ s/#defattrfile/$defattr/g;



my $specfile = $name . ".spec";

open(OUT,">$specfile");
print OUT $spec;
close(OUT);




