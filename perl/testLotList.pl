use Boitho::Lot;

if ($#ARGV == -1) {
print "no lot given";
exit;
}

Boitho::Lot::lotlistLoad();

print Boitho::Lot::lotlistGetServer($ARGV[0]) . "\n";
