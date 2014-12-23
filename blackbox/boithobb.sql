# phpMyAdmin SQL Dump
# version 2.5.3-rc1
# http://www.phpmyadmin.net
#
# Vert: localhost
# Generert den: 25. Jan, 2008 klokka 15:41 PM
# Tjenerversjon: 3.23.58
# PHP-Versjon: 4.1.2
# 
# Database : `boithobb`
# 

# --------------------------------------------------------

#
# Tabellstruktur for tabell `collectionAuth`
#

#DROP TABLE IF EXISTS `collectionAuth`;
CREATE TABLE `collectionAuth` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(50) NOT NULL default '',
  `password` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `usepass` (`username`,`password`)
) TYPE=MyISAM AUTO_INCREMENT=48 ;

#
# Dataark for tabell `collectionAuth`
#


# --------------------------------------------------------

#
# Tabellstruktur for tabell `config`
#

#DROP TABLE IF EXISTS `config`;
CREATE TABLE `config` (
  `configkey` varchar(255) NOT NULL default '',
  `configvalue` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`configkey`)
) TYPE=MyISAM;

#
# Dataark for tabell `config`
#

INSERT INTO `config` VALUES ('authenticatmetod', 'msad');
#INSERT INTO `config` VALUES ('msad_domain', 'domain.example.com');
#INSERT INTO `config` VALUES ('msad_user', 'username');
#INSERT INTO `config` VALUES ('msad_password', 'password');
#INSERT INTO `config` VALUES ('msad_ip', '127.0.0.1');
#INSERT INTO `config` VALUES ('msad_port', '');
INSERT INTO `config` VALUES ('ldap_domain', '');
INSERT INTO `config` VALUES ('ldap_user', '');
INSERT INTO `config` VALUES ('ldap_password', '');
INSERT INTO `config` VALUES ('ldap_port', '');
INSERT INTO `config` VALUES ('ldap_ip', '');
INSERT INTO `config` VALUES ('msad_group', '');
INSERT INTO `config` VALUES ('default_crawl_rate', '1440');
INSERT INTO `config` VALUES ('dist_preference', 'production');
INSERT INTO `config` VALUES ('auth_key', 'c1a8ad17484619b51ca7f290f2e0a5e4  -');
INSERT INTO `config` VALUES ('auth_code', '');
INSERT INTO `config` VALUES ('gc_last_run', '1199976690');
INSERT INTO `config` VALUES ('msad_ldapbase', '');
INSERT INTO `config` VALUES ('recrawl_schedule_start', '');
INSERT INTO `config` VALUES ('gc_default_rate', '604800');
INSERT INTO `config` VALUES ('recrawl_schedule_end', '');
INSERT INTO `config` VALUES ('suggdict_run_hour', '10');
INSERT INTO `config` VALUES ('suggdict_last_run', '1199975440');
INSERT INTO `config` VALUES ('recrawl_recheck_rate', '1440');
INSERT INTO `config` VALUES ('no_setup', '1');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `connectors`
#

#DROP TABLE IF EXISTS `connectors`;
CREATE TABLE `connectors` (
  `id` int(7) unsigned NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `comment` text,
  `commandFirst` varchar(255) default NULL,
  `commandUpdate` varchar(255) default NULL,
  `commandTest` varchar(255) default NULL,
  `scantoolAvailable` tinyint(4) NOT NULL default '0',
  `inputFields` varchar(100) NOT NULL default '',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) TYPE=MyISAM AUTO_INCREMENT=11 ;

#
# Dataark for tabell `connectors`
#

INSERT INTO `connectors` VALUES (1, 'SMB', 'SMB (Server Message Block). Protocol used in Windows networks to allow resources such as files on one machine to be shared on other machines as if they were local.', 'smbcrawler.pl #host #user%#password', '', '', 1, 'authentication, user_prefix, connector, crawling');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `connectorsFor`
#

#DROP TABLE IF EXISTS `connectorsFor`;
CREATE TABLE `connectorsFor` (
  `id` int(7) unsigned NOT NULL auto_increment,
  `connectorsId` int(7) unsigned NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=12 ;

#
# Dataark for tabell `connectorsFor`
#

INSERT INTO `connectorsFor` VALUES (1, 1, 'Shared files from Microsoft Windows');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `scanResults`
#

#DROP TABLE IF EXISTS `scanResults`;
CREATE TABLE `scanResults` (
  `id` int(11) NOT NULL auto_increment,
  `connector` int(11) NOT NULL default '0',
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `range` varchar(80) default NULL,
  `result_xml` text NOT NULL,
  `done` tinyint(4) NOT NULL default '0',
  `authid` int(11) default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=71 ;

#
# Dataark for tabell `scanResults`
#


# --------------------------------------------------------

#
# Tabellstruktur for tabell `search_logg`
#

#DROP TABLE IF EXISTS `search_logg`;
CREATE TABLE `search_logg` (
  `id` int(6) NOT NULL auto_increment,
  `tid` datetime NOT NULL default '0000-00-00 00:00:00',
  `query` varchar(30) NOT NULL default '',
  `search_bruker` varchar(20) default NULL,
  `treff` int(5) unsigned NOT NULL default '0',
  `search_tid` decimal(12,10) unsigned NOT NULL default '0.0000000000',
  `ip_adresse` varchar(15) NOT NULL default '',
  `side` int(3) NOT NULL default '0',
  `betaler_keywords_treff` int(3) NOT NULL default '0',
  `HTTP_ACCEPT_LANGUAGE` varchar(255) default NULL,
  `HTTP_USER_AGENT` varchar(255) default NULL,
  `HTTP_REFERER` varchar(255) default NULL,
  `GeoIPLang` char(2) default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

#
# Dataark for tabell `search_logg`
#


# --------------------------------------------------------

#
# Tabellstruktur for tabell `session_id`
#

#DROP TABLE IF EXISTS `session_id`;
CREATE TABLE `session_id` (
  `Id` int(9) unsigned NOT NULL default '0',
  `User_Name` char(100) NOT NULL default '',
  `Log_in_Date` datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`Id`)
) TYPE=MyISAM;

#
# Dataark for tabell `session_id`
#


# --------------------------------------------------------

#
# Tabellstruktur for tabell `shareGroups`
#

#DROP TABLE IF EXISTS `shareGroups`;
CREATE TABLE `shareGroups` (
  `name` varchar(50) NOT NULL default '',
  `share` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`,`share`)
) TYPE=MyISAM;

#
# Dataark for tabell `shareGroups`
#

# --------------------------------------------------------

#
# Tabellstruktur for tabell `shareUsers`
#

#DROP TABLE IF EXISTS `shareUsers`;
CREATE TABLE `shareUsers` (
  `name` varchar(255) NOT NULL default '',
  `share` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`,`share`)
) TYPE=MyISAM;

#
# Dataark for tabell `shareUsers`
#


# --------------------------------------------------------

#
# Tabellstruktur for tabell `shares`
#

#DROP TABLE IF EXISTS `shares`;
CREATE TABLE `shares` (
  `id` int(11) NOT NULL auto_increment,
  `host` varchar(32) NOT NULL default '',
  `connector` int(11) NOT NULL default '0',
  `active` tinyint(4) NOT NULL default '0',
  `auth_id` int(11) default NULL,
  `crawler_success` tinyint(4) NOT NULL default '0',
  `last` datetime default NULL,
  `rate` int(11) NOT NULL default '0',
  `query1` varchar(255) NOT NULL default '',
  `query2` varchar(255) NOT NULL default '',
  `smb_workgroup` varchar(50) NOT NULL default '',
  `smb_name` varchar(50) NOT NULL default '',
  `resource` varchar(255) NOT NULL default '',
  `domain` varchar(255) default NULL,
  `crawler_message` varchar(255) NOT NULL default '',
  `collection_name` varchar(255) NOT NULL default '',
  `userprefix` varchar(20) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `collection_name` (`collection_name`),
  KEY `query2` (`query2`)
) TYPE=MyISAM AUTO_INCREMENT=175 ;

#
# Dataark for tabell `shares`
#

#
#Runarb: 14 des 2010. licensekey har blitt flyttet fra sql/update-cur.sql til blackbox/boithobb.sql som er base pakken. Dete betyr at alle nye instalasjoner nå har lisensnøkkel som standar
INSERT INTO config (configkey, configvalue) VALUES('licensekey', '5W4CEAA4AAZRBXRA9RADFK56');
