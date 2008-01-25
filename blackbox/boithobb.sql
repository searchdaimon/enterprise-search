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

DROP TABLE IF EXISTS `collectionAuth`;
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

INSERT INTO `collectionAuth` VALUES (17, 'hello there', 'hi to you');
INSERT INTO `collectionAuth` VALUES (25, 'mysql_pass', 'password');
INSERT INTO `collectionAuth` VALUES (20, 'Hei ther', 'ASDF');
INSERT INTO `collectionAuth` VALUES (21, 'Boitho', '1234Asd');
INSERT INTO `collectionAuth` VALUES (22, 'boitho', 'G7J7v5L5Y7');
INSERT INTO `collectionAuth` VALUES (33, 'runarb', '359mujiQ');
INSERT INTO `collectionAuth` VALUES (32, 'runarb', 'resistLL');
INSERT INTO `collectionAuth` VALUES (34, 'guest', 'guest');
INSERT INTO `collectionAuth` VALUES (39, 'espeno', '2L84You');
INSERT INTO `collectionAuth` VALUES (44, 'aaaaaaa', 'bbbbbbbb');
INSERT INTO `collectionAuth` VALUES (45, 'rb', 'Hurra123');
INSERT INTO `collectionAuth` VALUES (46, 'test', 'test2');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `config`
#

DROP TABLE IF EXISTS `config`;
CREATE TABLE `config` (
  `configkey` varchar(255) NOT NULL default '',
  `configvalue` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`configkey`)
) TYPE=MyISAM;

#
# Dataark for tabell `config`
#

INSERT INTO `config` VALUES ('authenticatmetod', 'msad');
INSERT INTO `config` VALUES ('msad_domain', 'bbtest.searchdaimon.com');
INSERT INTO `config` VALUES ('msad_user', 'Boitho');
INSERT INTO `config` VALUES ('msad_password', '1234Asd');
INSERT INTO `config` VALUES ('msad_ip', '213.179.58.120');
INSERT INTO `config` VALUES ('msad_port', '');
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
INSERT INTO `config` VALUES ('recrawl_schedule_start', '12');
INSERT INTO `config` VALUES ('gc_default_rate', '604800');
INSERT INTO `config` VALUES ('recrawl_schedule_end', '7');
INSERT INTO `config` VALUES ('suggdict_run_hour', '10');
INSERT INTO `config` VALUES ('suggdict_last_run', '1199975440');
INSERT INTO `config` VALUES ('recrawl_recheck_rate', '1440');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `connectors`
#

DROP TABLE IF EXISTS `connectors`;
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
INSERT INTO `connectors` VALUES (2, 'ODBC', 'ODBC (Open DataBase Connectivity). A standard application programming interface (API) for accessing data in both relational and non-relational database management systems.', 'odbcclient $server $user $password "select * from table" $tempdir', '', '', 0, 'authentication, groups, connector, crawling');
INSERT INTO `connectors` VALUES (3, 'LDAP', 'LDAP (Lightweight Directory Access Protocol). It is a protocol for accessing information directories such as organizations, individuals, phone numbers, and addresses.', 'ldapcclient $server $user $password "select * from table" $tempdir', '', '', 0, 'authentication, groups, connector, crawling');
INSERT INTO `connectors` VALUES (4, 'MySQL', 'MySQL is the world\'s most popular open source database. This is for the native api. One can also use odbc.', 'mysqlcclient $server $user $password "select * from table" $tempdir', '', '', 0, 'authentication, user_prefix, groups, connector, crawling');
INSERT INTO `connectors` VALUES (5, 'NFS', 'NFS (Network File System). Standard for accessing files on a remote computer appearing as a local volume.', '', '', '', 0, 'authentication, connector, crawling');
INSERT INTO `connectors` VALUES (6, 'Local', 'Local files on your black box. This is mostly used to index exotic network based file systems. You will have to login by ssh and mount the file system, then tell the crawler where it is.', '', '', '', 0, 'authentication, connector, crawling');
INSERT INTO `connectors` VALUES (7, 'sFTP', 'Secure File Transfer Protocol. Like normal FTP, but secure over a ssl connection. Require a ssl 2 capable server.', NULL, NULL, NULL, 0, 'authentication, connector, crawling');
INSERT INTO `connectors` VALUES (8, 'http', 'Normal webpages. Hyper Text Transfer Protocol, the actual communications protocol that enables Web browsing.', NULL, NULL, NULL, 0, 'authentication, groups, connector, crawling');
INSERT INTO `connectors` VALUES (9, 'Exchange', 'Microsoft exchange', NULL, NULL, NULL, 0, 'authentication, connector, crawling, exchange_user_select');
INSERT INTO `connectors` VALUES (10, '24SevenOffice', '24Seven Office mail crawler', NULL, NULL, NULL, 0, 'authentication, connector, crawling');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `connectorsFor`
#

DROP TABLE IF EXISTS `connectorsFor`;
CREATE TABLE `connectorsFor` (
  `id` int(7) unsigned NOT NULL auto_increment,
  `connectorsId` int(7) unsigned NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=12 ;

#
# Dataark for tabell `connectorsFor`
#

INSERT INTO `connectorsFor` VALUES (1, 1, 'Boitho Personnal Space');
INSERT INTO `connectorsFor` VALUES (2, 1, 'Shared files from Microsoft Windows');
INSERT INTO `connectorsFor` VALUES (3, 2, 'Microsoft SQL server');
INSERT INTO `connectorsFor` VALUES (4, 2, 'Oracle database server');
INSERT INTO `connectorsFor` VALUES (5, 2, 'Sybase');
INSERT INTO `connectorsFor` VALUES (6, 2, 'DB2');
INSERT INTO `connectorsFor` VALUES (7, 2, 'IBM AS/400');
INSERT INTO `connectorsFor` VALUES (8, 2, 'MySQL SQL server odbc');
INSERT INTO `connectorsFor` VALUES (9, 3, 'Microsoft Exchange server');
INSERT INTO `connectorsFor` VALUES (10, 3, 'Eudora Internet Mail Server');
INSERT INTO `connectorsFor` VALUES (11, 4, 'MySQL SQL server native api (faster and easyer the odbc)');

# --------------------------------------------------------

#
# Tabellstruktur for tabell `scanResults`
#

DROP TABLE IF EXISTS `scanResults`;
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

INSERT INTO `scanResults` VALUES (26, 1, '2007-06-06 15:41:00', '213.179.58.120', '', 0, NULL);
INSERT INTO `scanResults` VALUES (38, 1, '2007-06-06 23:59:29', '213.179.58-59.120', '', 0, NULL);
INSERT INTO `scanResults` VALUES (70, 1, '2008-01-22 18:59:38', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boitho-docs2</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (57, 1, '2007-09-28 17:58:38', '213.179.58.120', '', 0, 21);
INSERT INTO `scanResults` VALUES (58, 1, '2007-09-28 17:58:51', '213.179.58.120', '', 0, 21);
INSERT INTO `scanResults` VALUES (66, 1, '2007-12-06 12:56:50', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup=""></host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (60, 1, '2007-09-28 18:13:57', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup=""></host>\n</result>', 1, NULL);
INSERT INTO `scanResults` VALUES (61, 1, '2007-09-28 18:14:20', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (62, 1, '2007-09-28 18:59:11', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (63, 1, '2007-10-03 19:02:33', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (64, 1, '2007-10-03 19:02:38', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);
INSERT INTO `scanResults` VALUES (65, 1, '2007-10-03 19:08:52', '213.179.58.120', '\n<result>\n  <host addr="213.179.58.120" name="" workgroup="">\n    <share>ADMIN$</share>\n    <share>boithodocs</share>\n    <share>C$</share>\n    <share>E$</share>\n    <share>kontorplassen</share>\n    <share>NETLOGON</share>\n    <share>Shared</share>\n    <share>SYSVOL</share>\n    <share>wikipedia</share>\n  </host>\n</result>', 1, 21);

# --------------------------------------------------------

#
# Tabellstruktur for tabell `search_logg`
#

DROP TABLE IF EXISTS `search_logg`;
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

DROP TABLE IF EXISTS `session_id`;
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

DROP TABLE IF EXISTS `shareGroups`;
CREATE TABLE `shareGroups` (
  `name` varchar(50) NOT NULL default '',
  `share` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`,`share`)
) TYPE=MyISAM;

#
# Dataark for tabell `shareGroups`
#

INSERT INTO `shareGroups` VALUES ('Account Operators', 62);
INSERT INTO `shareGroups` VALUES ('Account Operators', 138);
INSERT INTO `shareGroups` VALUES ('Administrators', 62);
INSERT INTO `shareGroups` VALUES ('Backup Operators', 45);
INSERT INTO `shareGroups` VALUES ('Backup Operators', 62);
INSERT INTO `shareGroups` VALUES ('Backup Operators', 67);
INSERT INTO `shareGroups` VALUES ('Backup Operators', 138);
INSERT INTO `shareGroups` VALUES ('Cert Publishers', 62);
INSERT INTO `shareGroups` VALUES ('Distributed COM Users', 45);
INSERT INTO `shareGroups` VALUES ('Distributed COM Users', 62);
INSERT INTO `shareGroups` VALUES ('Distributed COM Users', 67);
INSERT INTO `shareGroups` VALUES ('DnsAdmins', 62);
INSERT INTO `shareGroups` VALUES ('DnsAdmins', 138);
INSERT INTO `shareGroups` VALUES ('DnsUpdateProxy', 62);
INSERT INTO `shareGroups` VALUES ('Domain Admins', 62);
INSERT INTO `shareGroups` VALUES ('Domain Computers', 45);
INSERT INTO `shareGroups` VALUES ('Domain Controllers', 45);
INSERT INTO `shareGroups` VALUES ('Guests', 45);
INSERT INTO `shareGroups` VALUES ('Marketing', 138);
INSERT INTO `shareGroups` VALUES ('Terminal Server License Servers', 45);
INSERT INTO `shareGroups` VALUES ('Users', 29);
INSERT INTO `shareGroups` VALUES ('Users', 55);
INSERT INTO `shareGroups` VALUES ('Users', 71);

# --------------------------------------------------------

#
# Tabellstruktur for tabell `shareUsers`
#

DROP TABLE IF EXISTS `shareUsers`;
CREATE TABLE `shareUsers` (
  `name` varchar(255) NOT NULL default '',
  `share` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`,`share`)
) TYPE=MyISAM;

#
# Dataark for tabell `shareUsers`
#

INSERT INTO `shareUsers` VALUES ('eirik', 154);
INSERT INTO `shareUsers` VALUES ('espeno', 154);
INSERT INTO `shareUsers` VALUES ('rb', 154);

# --------------------------------------------------------

#
# Tabellstruktur for tabell `shares`
#

DROP TABLE IF EXISTS `shares`;
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

INSERT INTO `shares` VALUES (72, '', 5, 1, NULL, 0, '2006-11-30 11:55:50', 0, '', '', '', '', 'bbh-001.boitho.com:/home/boitho/boithoTools', '', 'Crawling it now. Click "Overview" again to see progression.', 'nfs test', NULL);
INSERT INTO `shares` VALUES (100, 'bbh-001.boitho.com', 7, 1, 32, 0, '2006-12-21 17:17:25', 0, '', '', '', '', '/home/runarb', '', 'can\'t conect to bbdn (boitho backend document server)', 'runarb at bbh', NULL);
INSERT INTO `shares` VALUES (168, '', 1, 1, 21, 0, NULL, 0, '', '', '', '', '\\\\samba.boitho.com\\sharedFolder', NULL, '', 'aaa aaa', '');
INSERT INTO `shares` VALUES (43, '', 1, 1, 21, 1, '2008-01-25 13:32:29', 1440, '', '', '', '', '\\\\213.179.58.120\\boithodocs', '', 'Ok', 'boithodocs', '');
INSERT INTO `shares` VALUES (97, 'localhost', 7, 1, 32, 0, '2006-12-20 20:36:55', 0, '', '', '', '', '/tmp', '', 'can\'t conect to bbdn (boitho backend document server)', 'tmp2', NULL);
INSERT INTO `shares` VALUES (98, '', 8, 1, NULL, 0, '2006-12-20 20:38:57', 0, '', '', '', '', 'http://www.searchdaimon.com/blog/', '', 'can\'t conect to bbdn (boitho backend document server)', 'searchdaimon blog', NULL);
INSERT INTO `shares` VALUES (99, '', 8, 1, NULL, 0, '2006-12-20 18:44:17', 0, '', '', '', '', 'http://www.pixelclub.net/', '', 'can\'t conect to bbdn (boitho backend document server)', 'pixelclub', NULL);
INSERT INTO `shares` VALUES (49, 'localhost', 4, 1, 22, 0, '2007-02-17 21:25:15', 0, 'sirdf_com', '', '', '', '', '', 'can\'t conect to bbdn (boitho backend document server)', 'sirdf_com', NULL);
INSERT INTO `shares` VALUES (101, 'hegg.idi.ntnu.no', 7, 1, 33, 0, '2006-12-21 15:12:29', 0, '', '', '', '', '/home/b/9/runarb', '', 'can\'t conect to bbdn (boitho backend document server)', 'runarb at idi', NULL);
INSERT INTO `shares` VALUES (73, '', 6, 1, NULL, 0, '2007-02-22 21:28:11', 0, '', '', '', '', '/tmp/localtest', '', 'can\'t conect to bbdn (boitho backend document server)', '/tmp/localtest', NULL);
INSERT INTO `shares` VALUES (77, '', 6, 1, NULL, 0, '2007-01-22 16:38:33', 40320, '', '', '', '', '/tmp', '', 'can\'t conect to bbdn (boitho backend document server)', '/tmp', NULL);
INSERT INTO `shares` VALUES (135, '', 6, 1, NULL, 0, '2007-03-16 19:34:28', 0, '', '', '', '', '/tmp/localtest', NULL, 'can\'t conect to bbdn (boitho backend document server)', 'localtest', NULL);
INSERT INTO `shares` VALUES (166, '213.179.58.120', 10, 1, 21, 1, '2007-11-12 18:27:05', 0, '', '', '', '', '213.179.58.120/kontorplassen/email_dev', NULL, 'Ok', '24sevenmail_dev', NULL);
INSERT INTO `shares` VALUES (161, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'o', '');
INSERT INTO `shares` VALUES (162, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'p://10.0.0.10/exchange', NULL, '', 'random_tPx8l', '');
INSERT INTO `shares` VALUES (138, 'vg.no', 2, 1, NULL, 0, '2007-05-21 16:55:28', 0, '', '', '', '', '', NULL, 'can\'t conect to bbdn (boitho backend document server)', 'random_bEh5c', '');
INSERT INTO `shares` VALUES (157, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'random_iJSj8', '');
INSERT INTO `shares` VALUES (158, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'random_r5Ogq', '');
INSERT INTO `shares` VALUES (159, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'random_GIOZV', '');
INSERT INTO `shares` VALUES (173, '', 1, 1, 21, 1, '2008-01-17 15:43:04', 0, '', '', '', '', '\\\\213.179.58.120\\boithodocs', NULL, 'Ok', 'boithodocs2', '');
INSERT INTO `shares` VALUES (153, '', 1, 1, 21, 1, '2008-01-15 22:04:45', 0, '', '', '', '', '\\\\213.179.58.120\\Shared', NULL, 'Ok', 'Shared', '');
INSERT INTO `shares` VALUES (154, '', 9, 1, 45, 1, '2007-08-20 20:42:29', 1440, '', '', '', '', 'http://129.241.50.208', NULL, 'Ok', 'Exchangetest', '');
INSERT INTO `shares` VALUES (155, '', 9, 1, 44, 0, NULL, 40320, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'admintest', '');
INSERT INTO `shares` VALUES (156, '213.179.58.120', 10, 1, 21, 0, '2007-10-08 09:28:02', 0, '', '', '', '', '213.179.58.120/kontorplassen/email', NULL, 'Crawling it now. Click "Overview" again to see progression.', '24sevenmail', NULL);
INSERT INTO `shares` VALUES (160, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'http://10.0.0.10/exchange/rb/', NULL, '', 'random_1dpdX', '');
INSERT INTO `shares` VALUES (163, '', 9, 1, NULL, 0, NULL, 0, '', '', '', '', 'p://10.0.0.10/exchange', NULL, '', 'random_BcKQ0', '');
INSERT INTO `shares` VALUES (170, '', 1, 1, 21, 1, '2008-01-15 22:04:45', 0, '', '', '', '', '\\\\213.179.58.120\\boitho-docs2', NULL, 'Ok', 'space name', '');
    
