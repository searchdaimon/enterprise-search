use String::CRC32;

my $word = shift(@ARGV) or die("usage: queryToIndexid.pl word");

my $WordID = crc32($word);

$mod = $WordID % 64;

print "WordID: $WordID\nIndex: $mod\n";
