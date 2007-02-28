<html>
<head>
<title>BB Phone Home Request</title>
</head>

<body>



<?php

function startphonehome() {
	print "Starting phome home\n";
	exec("perl /home/eirik/Boitho/boitho/bb-phone-home/bb-client.pl start", $out, $retval);
	if ($retval == 0) {
		return 0;
	} else {
		return -1;
	}
}

function stopphonehome() {
	print "Stopping phome home\n";
	exec("perl /home/eirik/Boitho/boitho/bb-phone-home/bb-client.pl stop", $out, $retval);

	if ($retval == 0) {
		return 0;
	} else {
		return -1;
	}
}

function phoenhomerunning() {
	$pid = exec("perl /home/eirik/Boitho/boitho/bb-phone-home/bb-client.pl running", $out, $retval);

	return ($retval == 0 ? 1 : 0);
}

if ($_REQUEST['phonehome']) {
	print startphonehome();
}
elseif (defined($_REQUEST['phonehomestop'])) {
	print stopphonehome();
}



?>


<form action="bb-request.php" method="POST">
<input type="hidden" name="phonehome" value="help" />
<input type="submit" value="Request help" />
</form>

<form action="bb-request.php" method="POST">
<input type="hidden" name="phonehomestop" value="help" />
<input type="submit" value="Shut down" />
</form>



<?php

$ret = phoenhomerunning();
if ($ret == 1) {
	print "<h4>Phone home is running</h4>\n";
}
else {
	print $ret;
}

?>

</body>
</html>
