<?php

	$dbhost = 'localhost';
	$dbuser = 'user';
	$dbpass = 'password';
	$dbname = 'helios_db';
	
	$count = 0;
	$sum = 0;
	   
	$db_conn = mysql_connect($dbhost, $dbuser, $dbpass, $dbname)
		or die('Could not connect: ');
	mysql_select_db($dbname) or die('Could not select database');
	$query = 'SELECT * FROM currentRecordHourly ORDER BY time DESC LIMIT 168';
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC)){
		if ($count != 0 && $count%3 == 0) {
			$valueArray[] = $sum/3.0;
			$dateArray[] = $line['time'];
			$sum = $line['value'];
		} else {
			$sum += $line['value'];
		}
		$count ++;
	}
	echo json_encode([$dateArray, $valueArray]);
	
	mysql_close($db_conn);
?>