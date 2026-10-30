
my $file = $ARGV[0];

#check whether we got a filename
if ($file eq '') {
	print "Fikke ikke inn noe filnavn\nBruke:\n\n\tPerl forsok2.pl filnavn.doc\n";
	exit;
}

#open the file
open(INF,$file);
binmode(INF);

#word files consist of blocks of 512 bytes
#a Word file begins with one as block 1.
#this should begin with the hex value "d0 cf 11 e0  a1 b1 1a e1"
#so we test whether this is a word file we can read by testing for "d0 cf 11 e0  a1 b1 1a e1"
seek(INF,0,0) or die($!);
read(INF,$heder,8);

#test for header
if (not (unpack('H16',$heder) eq 'd0cf11e0a1b11ae1')) {
	die("Kan ikke lese denne filen, er ikke en Word 8 fil\n");
}


#block no 2 is the so-called FIB (File Information Block) which contains information and pointers to data and blocks.
#the value at offset 24 is a pointer to the text itself. This is data type "long"
#Block 2 begins at address 512 (each block is 512 bytes and we start at 1, not 0)
#so offset 24 is at address 24 + 512 = 536.

#seek to address 536
seek(INF,536,0) or die($!);

#read 4 bytes
read(INF,$data,4) or die($!);

#find the start value. This is the offset where the data begins
$fcMin = unpack('L',$data);

#The value at block 2 offset 28 contains where the data stops
#offset 28 is at address 28 + 512 = 540.

#seek to address 540
seek(INF,540,0) or die($!);

#read 4 bytes
read(INF,$data,4) or die($!);


$fcMac = unpack('L',$data);

#$fcMin and $fcMac are offset values from block 3. So we must add 512 * 2 = 1024 bytes
$fcMin += 1024;
$fcMac += 1024;

$fcMac -= 22;	#it looks like $fcMac is reported 22 bytes too large.

#data size is $fcMac - $fcMin
$DataSize = $fcMac - $fcMin;

#print "$fcMin -> $fcMac, $DataSize\n";

#Seek to $fcMin and read $DataSize

seek(INF,$fcMin,0) or die($!);
read(INF,$data,$DataSize) or die($!);

$data =~ s/\0//g;	#remove NULL at the end
$data =~ s/\x0D/\n/g; 	#hex 0D, oct 15 is CR (Carriage Return). Converted to this platform's new line (\n)
#$data =~ s/[^a-zA-Z0-9 \n]//g;

print $data;

