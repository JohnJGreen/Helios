<?php

	$dbhost = 'localhost';
	$dbuser = 'user';
	$dbpass = 'password';
	$dbname = 'helios_db';
	   
	$db_conn = mysql_connect($dbhost, $dbuser, $dbpass, $dbname)
		or die('Could not connect: ');
	mysql_select_db($dbname) or die('Could not select database');
	$query = 'SELECT * FROM lastCurrentRecord where sensor_id=1';
	$result = mysql_query($query) or die('Query failed: ' . mysql_error());
	$line = mysql_fetch_array($result, MYSQL_ASSOC);
	
	echo json_encode($line);
	
	mysql_close($db_conn);
?>