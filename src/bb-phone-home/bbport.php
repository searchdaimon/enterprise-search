<?php

$retval = 0;
$cmd = "/home/boitho/bb-phone-home-get-port.pl";
$args = "".$_GET['hostname'] . "-" . $_SERVER['REMOTE_ADDR'] . "";

if (!preg_match('/^[\w.\-\d\/]+$/', $args)) {
	exit("Illegal hostname");
}


$line = system($cmd . " \"$args\"", $retval);

if ($retval != 0) {
	print "\nAn Error has occured. Please contact support.\n";
}



?>
