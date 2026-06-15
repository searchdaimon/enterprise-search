package MySQLDcConnect;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw();
@EXPORT_OK = qw();



####################################################################
#setup
#####
#MySQL setup
$user = "boitho";
$Password = "G7J7v5L5Y7";
$server = "localhost";
$database = "boithodc";

use DBI; #uses the DBI database interface

#connects and returns the database handle
sub GetHandler {
	# this runs every time mymod.pm is called
	my $dbh = DBI->connect("DBI:mysql:database=$database;host=$server;port=3306",						#connects to the database
                             $user, $Password) or warn("Can`t connect: $DBI::errstr");	#
	
return $dbh;						 
}
							 
		
