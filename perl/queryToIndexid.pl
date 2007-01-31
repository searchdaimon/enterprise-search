use String::CRC32;

$mod = crc32($ARGV[0]) % 64;
print "$mod\n";
