CREATE DATABASE IF NOT EXISTS `helios_db`;
USE `helios_db`;

/*	To contain all records of single events the server should fire off.*/
CREATE TABLE IF NOT EXISTS `schedEvents`(
	`id` int unsigned NOT NULL AUTO_INCREMENT,
	`group_id` int REFERENCES `repeatingSchedEvents`(`id`),
	`time` datetime NOT NULL,
	`name` tinytext NOT NULL,
	`comment` text,
	`topic` text NOT NULL,
	`command` text,
	`arguments` text,
	`executed` tinyint NOT NULL DEFAULT false REFERENCES `errorCodes`(`id`),
	`deleted` tinyint NOT NULL DEFAULT false REFERENCES `errorCodes`(`id`),
	PRIMARY KEY (id)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain all records of events which should be repeated in the single events table.*/
CREATE TABLE IF NOT EXISTS `repeatingSchedEvents`(
	`id` int NOT NULL AUTO_INCREMENT,
	`group_id` int,
	`start` datetime NOT NULL,
	`end` datetime NOT NULL,
	`period` int NOT NULL,
	`name` tinytext NOT NULL,
	`comment` text,
	`topic` text NOT NULL,
	`command` text,
	`arguments` text,
	`executed` tinyint NOT NULL DEFAULT False REFERENCES `errorCodes`(`id`),
	`deleted` tinyint NOT NULL DEFAULT False REFERENCES `errorCodes`(`id`),
	PRIMARY KEY (id)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain the setup strings the clients use to get their rules, expected topics, and the response the client should execute per topic.*/
CREATE TABLE IF NOT EXISTS `subsciberSetup`(
	`mac_addr` bigint UNSIGNED NOT NULL,
	`topic` text NOT NULL,
	`setup_string` text NOT NULL
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain a history of messages passing by the server. Mainly for debugging.*/
CREATE TABLE IF NOT EXISTS `eventsRecord`(
	`time` datetime,
	`topic` text,
	`message` text
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain all current readings to create graphs and compute things.*/
CREATE TABLE IF NOT EXISTS `currentRecordInstant`(
	`sensor_id` tinyint UNSIGNED,
	`time` datetime,
	`value` float(7,3)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain all current readings to create graphs and compute things.*/
CREATE TABLE IF NOT EXISTS `currentRecordHourly`(
	`sensor_id` tinyint UNSIGNED,
	`time` datetime,
	`value` float(7,3)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	To contain all messages the user or an admin may need.*/
CREATE TABLE IF NOT EXISTS `notifications`(
	`type` tinytext,
	`level` tinyint REFERENCES `errorCodes`(`id`),
	`time` datetime,
	`message` text
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

/*	List of error codes.*/
CREATE TABLE IF NOT EXISTS `errorCodes`(
	`id` smallint,
	`type` tinytext,
	`description` text
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
