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
#variabler:
$Sytem_Navn = "boitho";




###############################################################################################################
#Kjøres hver gang man bruker mymod.pm
###############################################################################################################





#henter server spesifik data fra fil
open(INF,"/home/boitho/boithoTools/setup.txt") or die("Can't open setup.txt: $!");
@setup_ary = <INF>;
close(INF);

foreach $line (@setup_ary) {
   	($name, $value) = split(/=/, $line);
	chomp($value); ## avoid \n on last field
	$SETUP{$name} = $value;
}

$dbh = DBI->connect("DBI:mysql:database=$SETUP{database};host=$SETUP{server};port=3306",$SETUP{user},$SETUP{Password}) or die("Can`t connect: $DBI::errstr");
