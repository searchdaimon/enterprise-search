
my $file = $ARGV[0];

#sjekker om vi fikk inn et filnavn
if ($file eq '') {
	print "Fikke ikke inn noe filnavn\nBruke:\n\n\tPerl forsok2.pl filnavn.doc\n";
	exit;
}

#opner filen
open(INF,$file);
binmode(INF);

#word filer består av blokker på 512 bytes
#en Word fil begynner med en som block 1.
#denne skal begynne med hex verdien "d0 cf 11 e0  a1 b1 1a e1"
#vi tester derfor om dette er en word fil vi kan lese ved å teste for "d0 cf 11 e0  a1 b1 1a e1"
seek(INF,0,0) or die($!);
read(INF,$heder,8);

#tester for heder
if (not (unpack('H16',$heder) eq 'd0cf11e0a1b11ae1')) {
	die("Kan ikke lese denne filen, er ikke en Word 8 fil\n");
}


#block nr 2 er den såkalte  FIB (File Information Block) som ineholder informasjon og pekere til data og blokker.
#verdien med offset 24 er en peker til selve teksten. Dette er datatype "long"
#Blokk 2 begynner på adresse 512 (hver blokk er på 512 bytes og vi begynner på 1, ikke 0)
#altså er ofset 24 i adresse 24 + 512 = 536.

#søker til adresse 536
seek(INF,536,0) or die($!);

#leser 4 bytes
read(INF,$data,4) or die($!);

#finner start verdien. Dette er ofsettet der dataene begynner
$fcMin = unpack('L',$data);

#Verdien på block 2 offsett 28 ineholder hvor dataen stopper
#ofset 28 i adresse 28 + 512 = 540.

#søker til adresse 540
seek(INF,540,0) or die($!);

#leser 4 bytes
read(INF,$data,4) or die($!);


$fcMac = unpack('L',$data);

#$fcMin og $fcMac er ofsett verdier fra blok 3. Altså må vi legge til 512 * 2 = 1024 bytes
$fcMin += 1024;
$fcMac += 1024;

$fcMac -= 22;	#det ser ut som om $fcMac oppgis 22 bytes for stor.

#data størelse er $fcMac - $fcMin
$DataSize = $fcMac - $fcMin;

#print "$fcMin -> $fcMac, $DataSize\n";

#Søker til $fcMin og leser $DataSize

seek(INF,$fcMin,0) or die($!);
read(INF,$data,$DataSize) or die($!);

$data =~ s/\0//g;	#fjerner  NULL på slutten
$data =~ s/\x0D/\n/g; 	#hex 0D, oct 15 er CR (Carage Return). Gjøres om til denne platformens new line (\n)
#$data =~ s/[^a-zA-Z0-9 \n]//g;

print $data;

