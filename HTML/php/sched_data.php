<?PHP

/** sched_data.php : Schedule Data RESTful Interface
  *
  * Arun Kalahasti
  * University of Texas at Arlington
  *
  * version: 0.1 alpha (2016-04-01)
  */
  
// Requests/reply via JSON string exchange
// =======================================


// Get database connection information.
require_once("includes/db_config.php");

// Parse input.
$json_out = array();
$input_json = json_decode(file_get_contents('php://input'), true);
if(!$input_json){
	$json_out["ERROR"] = "Unable to decode JSON input. Please check syntax";
}

// Keep a record of the input, just in case it needs to be looked at for errors, etc.
if(isset($input_json['echo'])){
	$json_out['RAW_INPUT'] = addslashes(file_get_contents('php://input'));
}

// Check for bare minimum input.
if(isset($input_json['type']))
	$input_type = strtolower($input_json['type']);
else {
	$json_out["ERROR"] = "No request type given.";
	ErrorExit(1);
}

// Switch based on type. Cases should return an array to be added to json_out or error exit.
switch($input_type){
	
	// Enters a new event with given parameters
	//	EXAMPLE INPUT:	{"type": "create_event", "topic":"/helios/test", "command":"off", "name":"Test", "time":1459900740}
	//					{"type": "create_event", "topic":"/helios/lights", "command":"on", "name":"All lights on", "time":1459900740, "comment":"Timer to turn on all the lights system wide."}
	case "create_event":
		$json_out = array_merge ($json_out, CreateSchedEvent($input_json));
		break;
	
	// Finds an event with a given ID#.
	// EXAMPLE INPUT:	{"type": "get_id", "id":1}
	//					{"type": "get_id", "id":30}
	case "get_id":
		if(isset($input_json['id'])){
			$json_out = array_merge ($json_out, GetRecordById($input_json['id']));
		}
		else {
			$json_out["ERROR"] = "Missing Arguments. Requests getting records by ID must have number named: 'id'";
			ErrorExit(1);
		}
		break;
		
	//	Gets schedule events based on a time and period length. If no beginning time is given, the period is assumed to start at the current time.
	//	Actually only sets up a period end time and passes to next function, which gets events based on period beginning time and end time.
	// EXAMPLE INPUT:	{"type": "get_timeslot", "per_len":"3600"}	// Get all events in the upcomming hour
	//					{"type": "get_timeslot", "per_beg":1459468800, "per_len":"86400"} // Get all events in the 24 hr after Apr 1, 2016 starts.
	case "get_timeslot":
	
		// Period start time and length are given.
		if(isset($input_json['per_beg']) && isset($input_json['per_len'])){
			if (!is_numeric($input_json['per_len'])){
				$json_out["ERROR"] = "'per-len' must be a numeric value in seconds.";
				ErrorExit(1);
			}
			// If given time is MySQL datetime, convert to unix time.
			if(!is_numeric($input_json['per_beg'])){
				// Convert to epoch time.
				try {
					$input_json['per_beg'] = strtotime($input_json['per_beg']);
				}
				catch (Exception $e) {
						$json_out["ERROR"] = "Could not convert given time to valid epoch time.";
						ErrorExit(1);
				}
			}
			// Add period length to period beginning and save.
			$input_json['per_end'] =  $input_json['per_beg'] + $input_json['per_len'];
		}
		
		// Only a period length is given. Assuming period begins at time of request.
		else if(!isset($input_json['per_beg']) && isset($input_json['per_len'])){
			$input_json['per_beg'] = time();
			//echo "current time is:".time();
			$input_json['per_end'] =  $input_json['per_beg'] + $input_json['per_len'];
		}
		
		else {
			$json_out["ERROR"] = "Missing Arguments. Requests getting records by time slot must have a period beginning('per-beg') and a period length('per_len').";
			ErrorExit(1);
		}
		
	//	Gets events between a given starting datetime and ending datetime. Can accept input in epoch time or MySQL datetime.
	// EXAMPLE INPUT:	{"type": "get_timerange", "per_beg":1459468800, "per_end":1459555200} // Get all events between Apr 1, 2016 and Apr 2, 2016.
	case "get_timerange":
		
		// Ensure both beginning and ending time are given.
		if(isset($input_json['per_beg']) && isset($input_json['per_end'])){
			
			// Check if beginning time is given as a number, as that should be in epoch time. If it is not, try converting to epoch.
			if (is_numeric($input_json['per_beg'])){
				try {
					$input_json['per_beg'] = date("Y-m-d H:i:s", $input_json['per_beg']);
				}
				catch (Exception $e) {
						$json_out["ERROR"] = "Could not convert given time to a valid MySQL time.";
						ErrorExit(1);
				}
			}
			
			// Check if ending time is given as a number, as that should be in epoch time. If it is not, try converting to epoch.
			if (is_numeric($input_json['per_end'])){
				try {
					$input_json['per_end'] = date("Y-m-d H:i:s", $input_json['per_end']);
				}
				catch (Exception $e) {
						$json_out["ERROR"] = "Could not convert given time to a valid MySQL time.";
						ErrorExit(1);
				}
			}
			
			GetEventsInRange($input_json);
		}
		else {
			$json_out["ERROR"] = "Missing Arguments. Requests getting records by time range must have period beginning('per-beg') and a period end('per_end').";
			ErrorExit(1);
		}
		break;
		
	// Deletes a list of IDs from database.
	// EXAMPLE INPUT:	{"type":"delete_ids", "ids":[1,2,3]}
	case "delete_ids":
	
		//	Ensure array exists.
		if(isset($input_json['ids'])){
			$json_out = array_merge ($json_out, DeleteRecords($input_json['ids']));
		}
		else {
			$json_out["ERROR"] = "Missing Arguments. Requests deleting records by ID#s must have list of numbers named: 'ids'";
			ErrorExit(1);
		}
		break;
		
	//
	default:
		$json_out["ERROR"] = "Type request not recognized";
		ErrorExit(1);
}


// Should only get here if no errors were hit. Convert to JSON string and print to user.
header('Content-type: application/json');
echo stripslashes(json_encode($json_out));


function DeleteRecords($rec_ids){
	
	global $json_out;
	global	$db_config;
	$db_conn = mysqli_connect($db_config['host'], $db_config['user'], $db_config['password'], $db_config['database'])
		or die('Could not connect: ' . mysql_error());
	
	$sql_query = "DELETE from `schedEvents` WHERE `ID` IN (".implode( ",", $rec_ids ).")";
	
	if (!$result = $db_conn->query($sql_query)) {
					$json_out["ATTEMPTED_QUERY"] = $query;
					$json_out["ERRNO"] = $db_conn->errno;
					$json_out["ERROR"] = $db_conn->error;
					ErrorExit(1);
	}
	$json_out["SUCCESS"] = "Records (".implode( ",", $rec_ids ).") have been deleted.";
	
	return $json_out;
}

function GetRecordById($rec_id){
	
	global $json_out;
	global	$db_config;
	$db_conn = mysqli_connect($db_config['host'], $db_config['user'], $db_config['password'], $db_config['database'])
		or die('Could not connect: ' . mysql_error());
	
	/* check connection */
	if ($db_conn->connect_errno) {
		$json_out["ERROR"] = "Connect failed: %s\n".$db_conn->connect_error;
		ErrorExit(1);
	}
	//MySqli Select Query
	$query = "SELECT `ID`, `GroupID`, `Time`, `Name`, 
								`Comment`, `Topic`, `Command`,
								`Arguments`, `Executed`,
								`Deleted` FROM `schedEvents` WHERE `ID` = $rec_id";
								
	if (!$result = $db_conn->query($query)) {
					$json_out["ATTEMPTED_QUERY"] = $query;
					$json_out["ERRNO"] = $db_conn->errno;
					$json_out["ERROR"] = $db_conn->error;
					ErrorExit(1);
	}
		
	// Phew, we made it. We know our MySQL connection and query 
	// succeeded, but do we have a result?
	if ($result->num_rows === 0) {
		// Oh, no rows! Sometimes that's expected and okay, sometimes
		// it is not. You decide. In this case, maybe id number was too
		// large? 
		$json_out["ERROR"] = "Could not find a match for ID $rec_id.";
		ErrorExit(1);
	}
	
	$s_event = $result->fetch_assoc();
	return $s_event;
}

function GetEventsInRange($input_json){
	global $json_out;
	global $db_config;
	$db_conn = mysqli_connect($db_config['host'], $db_config['user'], $db_config['password'], $db_config['database'])
		or die('Could not connect: ' . mysql_error());
		
	/* check connection */
	if ($db_conn->connect_errno) {
		$json_out["ERROR"] = "Connect failed: %s\n".$db_conn->connect_error;
		ErrorExit(1);
	}
	
	if(isset($input_json['per_beg']) && isset($input_json['per_end'])){
		$sql_query = "SELECT `ID` FROM `schedEvents` WHERE `Time` BETWEEN '".$input_json['per_beg']."' AND '".$input_json['per_end']."'";
		//echo $sql_query;
		
		if (!$result = $db_conn->query($sql_query)){
			$json_out["ATTEMPTED_QUERY"] = $sql_query;
			$json_out["ERRNO"] = $db_conn->errno;
			$json_out["ERROR"] = $db_conn->error;
			ErrorExit(1);
		}
		
		if ($result->num_rows > 0) {
			$results_array = array();
			// output data of each row
			while($row = $result->fetch_assoc()) {
				$results_array[] = GetRecordById($row['ID']);
			}
		} else {
			echo "0 results";
		}
		// $json_out = array_merge ($json_out, GetRecordById($db_conn->insert_id));
		if(isset($results_array)){
			$json_out["events"] = $results_array;
		}
		$json_out["type"] = "sched_event_list";
	}
}

function CreateSchedEvent($input_json){
	global $json_out;
	global $db_config;
	$db_conn = mysqli_connect($db_config['host'], $db_config['user'], $db_config['password'], $db_config['database'])
		or die('Could not connect: ' . mysql_error());
	
	/* check connection */
	if ($db_conn->connect_errno) {
		$json_out["ERROR"] = "Connect failed: %s\n".$db_conn->connect_error;
		ErrorExit(1);
	}
	
	if(		isset($input_json['time']) && 
			isset($input_json['topic']) && 
			isset($input_json['command']) && 
			isset($input_json['name'])) {
				/* Required fields found */
				$create_array = array();
				
				if (is_numeric($input_json['time'])){
					try {
						$input_json['time'] = date("Y-m-d H:i:s", $input_json['time']);
					} catch (Exception $e) {
						$json_out["ERROR"] = "Could not convert given time to a valid MySQL time.";
						ErrorExit(1);
					}
				}
				
				$sql_query = "INSERT INTO `schedEvents`".
				"(`GroupID`, `Time`, `Name`, `Comment`, `Topic`, `Command`, `Arguments`, `Executed`, `Deleted`) VALUES (".
				(!isset($input_json['groupId'])?"NULL":("'".$input_json['groupId']."'")) . ", ".
				(!isset($input_json['time'])?"NULL":("'".$input_json['time']."'")) . ", ".
				(!isset($input_json['name'])?"NULL":("'".$input_json['name']."'")) . ", ".
				(!isset($input_json['comment'])?"NULL":("'".$input_json['comment']."'")) . ", ".
				(!isset($input_json['topic'])?"NULL":("'".$input_json['topic']."'")) . ", ".
				(!isset($input_json['command'])?"NULL":("'".$input_json['command']."'")) . ", ".
				(!isset($input_json['arguments'])?"NULL":("'".$input_json['arguments']."'")) . ", ".
				(!isset($input_json['executed'])?"FALSE":("'".$input_json['executed']."'")) . ", ".
				(!isset($input_json['deleted'])?"FALSE":("'".$input_json['deleted']."'")).
				")";
				
				
				if (!$result = $db_conn->query($sql_query)){
					$json_out["ATTEMPTED_QUERY"] = $sql_query;
					$json_out["ERRNO"] = $db_conn->errno;
					$json_out["ERROR"] = $db_conn->error;
					ErrorExit(1);
				}
				
				$json_out = array_merge ($json_out, GetRecordById($db_conn->insert_id));
				
				$json_out["type"] = "sched_event";
				
			}
	else {
		$missing_fields = array();
		if (!isset($input_json['time'])){
			$missing_fields[] = "time";
			}
		if (!isset($input_json['topic'])){
			$missing_fields[] = "topic";
			}
		if (!isset($input_json['command'])){
			$missing_fields[] = "command";
			}
		if (!isset($input_json['name'])){
			$missing_fields[] = "name";
			}
		global $json_out;
		$json_out["ERROR"] = "Missing fields:".implode( ",", $missing_fields ).". Create event must have at minimum: \'time\', \'topic\', \'command\', \'name\'";
		
		ErrorExit(1);
	}
	return $json_out;
}

function ErrorExit($exit_status){
	global $json_out;
	header('Content-type: application/json');
	echo json_encode($json_out);
	exit($exit_status);
}
?>