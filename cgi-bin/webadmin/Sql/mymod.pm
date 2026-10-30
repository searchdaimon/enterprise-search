#!/usr/bin/perl
# mymod.pm
#
package mymod;
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw();
@EXPORT_OK = qw($dbh);


###############################################################################################################
#settup
#####
#variables:
$Sytem_Navn = "boitho";




###############################################################################################################
#Runs every time mymod.pm is used
###############################################################################################################





#fetches server-specific data from file
open(INF,"/home/boitho/boithoTools/setup.txt") or die("Can't open setup.txt: $!");
@setup_ary = <INF>;
close(INF);

foreach $line (@setup_ary) {
   	($name, $value) = split(/=/, $line);
	chomp($value); ## avoid \n on last field
	$SETUP{$name} = $value;
}

$dbh = DBI->connect("DBI:mysql:database=$SETUP{database};host=$SETUP{server};port=3306",$SETUP{user},$SETUP{Password}) or die("Can`t connect: $DBI::errstr");
