<?php

	$dbhost = 'localhost';
	$dbuser = 'user';
	$dbpass = 'password';
	$dbname = 'helios_db';
	   
	$db_conn = mysql_connect($dbhost, $dbuser, $dbpass, $dbname)
		or die('Could not connect: ');
	mysql_select_db($dbname) or die('Could not select database');
	$query = 'SELECT * FROM currentRecordHourly ORDER BY time DESC LIMIT 24';
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC)){
		$dateArray[] = $line['time'];
		$valueArray[] = $line['value'];
	}
	echo json_encode([$dateArray, $valueArray]);
	
	mysql_close($db_conn);
?>