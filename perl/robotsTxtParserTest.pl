use constant RobotName => 'boitho.com-dc';



$url = 'http://www.vg.no/test.htm';

#$RobotsTxtData = qq{
##test robts.txt fil
#User-agent: boitho.com-dc
#Disallow: /
#};


my $domene = fin_domene($url);
my $robotstxturl = "http://$domene/robots.txt";


#opner databasen
tie %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");


my $DomenesRobottxt = $DBMrobottxt{$domene};
my ($RobotsTxtType,$RobotsTxtData) = unpack('A A*',$DomenesRobottxt);

untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");


print "RobotsTxtType: $RobotsTxtType\nrobots.txt: $RobotsTxtData\n\n";

print "\nData: $RobotsTxtData\nType: $RobotsTxtType\n";

use commonRobotstxt qw(is_RobotsAllowd);
use common qw{fin_domene};




if (is_RobotsAllowd(RobotName,$RobotsTxtData,$robotstxturl,$url)) {

	print "is RobotsAllowd\n";

}
else {
	print "is NOT RobotsAllowd!!!\n";
}
