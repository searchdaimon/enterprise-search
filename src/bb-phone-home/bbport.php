<?php

$retval = 0;
$cmd = "/bin/bb-phone-home-get-port.pl";
$args = $_GET['hostname'];

if (!preg_match('/^[\w.\-\d\/]+$/', $args)) {
	exit("Illegal hostname");
}



$line = system($cmd . " " .$args, $retval);

if ($retval != 0) {
	print "\nSomething went wrong.\n";
}



?>
