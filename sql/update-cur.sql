alter table collectionAuth add `comment` varchar(255) default NULL;

INSERT INTO `config` VALUES ('msad_ldapstring','');
INSERT INTO `config` VALUES ('msad_ldapbase','');



CREATE TABLE `shareUsers` (
  `name` varchar(255) NOT NULL default '',
  `share` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`,`share`)
) TYPE=MyISAM;

CREATE TABLE `sessionData` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `success` enum('0','1') default NULL,
  `type` varchar(30) default NULL,
  `data` text,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

ALTER TABLE connectors add `extension` tinyint(4) default NULL;
ALTER TABLE connectors add `modified` datetime default NULL;
ALTER TABLE connectors add `active` tinyint(4) NOT NULL default '1';
ALTER TABLE connectors add `read_only` tinyint(4) NOT NULL default '1';

UPDATE connectors set active = 1;

CREATE TABLE param (
  id int(11) NOT NULL auto_increment,
  connector int(11) default NULL,
  param varchar(255) default NULL,
  example varchar(255) default NULL,
  PRIMARY KEY  (id),
  UNIQUE KEY connector (connector,param)
) TYPE=MyISAM;

CREATE TABLE shareParam (
  share int(11) NOT NULL default '0',
  param int(11) NOT NULL default '0',
  value varchar(255) default NULL,
  PRIMARY KEY  (share,param) 
) TYPE=MyISAM;
  
INSERT INTO `connectors` VALUES (9, 'Exchange', 'Microsoft exchange', NULL, NULL, NULL, 0, 'authentication, connector, crawling, exchange_user_select',1, NULL, 0, 1);
INSERT INTO `connectors` VALUES (64, 'Intranet', NULL, NULL, NULL, NULL, 0, 'custom_parameters, crawling, authentication', 1, '2008-08-13 09:24:43', 1, 1);

INSERT INTO param VALUES (101,64,'url','http://www.example.com');


ALTER TABLE `shares` ADD `crawl_pid` int(10) unsigned default NULL;

INSERT INTO `config` ( `configkey` , `configvalue` ) 
VALUES (
'msad_ldapgroupstring', ''
);


INSERT INTO `config` ( `configkey` , `configvalue` ) 
VALUES (
'authentication_timeout', '100'
);

-- New usersystem handling

CREATE TABLE system (
  id int(10) unsigned NOT NULL auto_increment,
  name varchar(255) NOT NULL default 'Unnamed system',
  is_primary tinyint(4) default '0',
  connector int(10) unsigned default NULL,
  PRIMARY KEY  (id)
) TYPE=MyISAM COMMENT='Brukersystem';


CREATE TABLE systemMapping (
  system int(10) unsigned NOT NULL default '0',
  prim_usr varchar(245) NOT NULL default '',
  secnd_usr varchar(245) NOT NULL default '',
  PRIMARY KEY  (system,prim_usr,secnd_usr)
) TYPE=MyISAM COMMENT='Mapping secondary systems to primary';


CREATE TABLE systemParam (
  connector int(11) unsigned NOT NULL default '0',
  param varchar(255) NOT NULL default '',
  note varchar(255) default NULL,
  required TINYINT DEFAULT '0' NOT NULL,
  PRIMARY KEY  (connector,param),
  UNIQUE KEY systemParam (connector,param)
) TYPE=MyISAM;


CREATE TABLE systemParamValue (
  param varchar(255) NOT NULL default '',
  system int(10) unsigned NOT NULL default '0',
  value varchar(255) default NULL,
  PRIMARY KEY  (param,system)
) TYPE=MyISAM;

CREATE TABLE foreignUserSystem (
  usersystem int(11) NOT NULL default '0',
  username varchar(255) NOT NULL default '',
  groupname varchar(255) NOT NULL default ''
) TYPE=MyISAM;



INSERT INTO config (configkey, configvalue) VALUES('licensekey', '');
INSERT INTO config (configkey, configvalue) VALUES('licensesystem', 'ohSh7oow');

ALTER TABLE `shares` add `system` int(10);
UPDATE shares SET system = 1 WHERE system IS NULL;


CREATE TABLE shareParam (
  share int(11) NOT NULL default '0',
  param int(11) NOT NULL default '0',
  value varchar(255) default NULL,
  PRIMARY KEY  (share,param)
) TYPE=MyISAM;

CREATE TABLE shareResults (
  share int(11) NOT NULL default '0',
  summary enum('snippet','start') NOT NULL default 'snippet',
  filter_same_url tinyint(4) NOT NULL default '1',
  filter_same_domain tinyint(4) NOT NULL default '1',
  filter_TLDs tinyint(4) NOT NULL default '1',
  filter_response tinyint(4) NOT NULL default '1',
  filter_same_crc32 tinyint(4) NOT NULL default '1',
  rank_author_array varchar(255) NOT NULL default '1',
  rank_title_array varchar(255) NOT NULL default '15',
  rank_title_first_word int(11) NOT NULL default '25',
  rank_headline_array varchar(255) NOT NULL default '4,6,7,8',
  rank_body_array varchar(255) NOT NULL default '1,2,4,7,9,10,12,14,16,17,18,19,20,21,22,23,24',
  rank_url_array varchar(255) NOT NULL default '5',
  rank_url_main_word int(11) NOT NULL default '30',
  cache_link tinyint(4) NOT NULL default '1',
  PRIMARY KEY  (share)
) TYPE=MyISAM COMMENT='searchd cfg values';

CREATE TABLE param (
  id int(11) NOT NULL auto_increment,
  connector int(11) default NULL,
  param varchar(255) default NULL,
  example varchar(255) default NULL,
  PRIMARY KEY  (id),
  UNIQUE KEY connector (connector,param)
) TYPE=MyISAM;

UPDATE connectors SET inputFields = CONCAT('user_system, ', inputFields) WHERE inputFields NOT LIKE "%user_system%";


INSERT INTO `connectors` VALUES (70, 'Superoffice', 'Superoffice push crawler', NULL, NULL, NULL, 0, 'custom_parameters, crawling, user_system, authentication', NULL, NULL, 1, 1);

# Runarb: 21 jan. Disabler denne da SharePoint crawleren ikke fungerer.	
# INSERT INTO `connectors` VALUES (73, 'SharePoint', NULL, NULL, NULL, NULL, 0, 'user_system, custom_parameters, crawling, authentication', 1, '2008-10-14 18:18:24', 1, 1);
# INSERT INTO `param` VALUES (121, 73, 'delay', 'Delay in seconds between crawl.');
# INSERT INTO `param` VALUES (108, 73, 'url', 'http://www.example.com');
# INSERT INTO `param` VALUES (124, 73, 'download_images', '1 to activate, 0 to disable.');

CREATE TABLE `activeUsers` (
  `user` varchar(255) default NULL,
  UNIQUE KEY `user` (`user`)
) TYPE=MyISAM;


INSERT INTO `param` VALUES (NULL, 70, 'Host', 'example.com');



INSERT INTO `connectors` VALUES (99, 'ExchangePublic', 'Microsoft Exchange Public Folder', NULL, NULL, NULL, 0, 'user_system, authentication, connector, crawling', 0, NULL, 1, 1);

-- Active directory system
INSERT INTO `system` (id, name, is_primary, connector) VALUES (1, 'Active Directory', 1, 1);


#INSERT INTO systemParamValue (param, system, value) SELECT 'domain' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_domain'; 
#INSERT INTO systemParamValue (param, system, value) VALUES('ldapstring', 1, '');
#INSERT INTO systemParamValue (param, system, value) VALUES('ldapbase', 1, '');
#INSERT INTO systemParamValue (param, system, value) VALUES('ldapgroupstring', 1, '');

# setter opp hva som trengs for ad
INSERT INTO `systemParam` VALUES (1, 'ip', NULL, 1);
INSERT INTO `systemParam` VALUES (1, 'user', NULL, 1);
INSERT INTO `systemParam` VALUES (1, 'password', NULL, 1);
INSERT INTO `systemParam` VALUES (1, 'domain', NULL, 1);
INSERT INTO `systemParam` VALUES (1, 'ldapstring', NULL, 0);
INSERT INTO `systemParam` VALUES (1, 'ldapbase', NULL, 0);
INSERT INTO `systemParam` VALUES (1, 'ldapgroupstring' ,NULL, 0);

# kopierer over ldap info til brukersys.
# msad_group er depricated
INSERT INTO systemParamValue (param, system, value) SELECT 'ip' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_ip'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'user' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_user'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'password' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_password'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'domain' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_domain'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'ldapgroupstring' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_ldapgroupstring'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'ldapbase' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_ldapbase'; 
INSERT INTO systemParamValue (param, system, value) SELECT 'ldapstring' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_ldapstring'; 

# setter ordbok rerun til 7 hvis den er kl 10.00, som er standar.
update config set configvalue = '7' where configkey = 'suggdict_run_hour' and configvalue  = '10';

#setter cache til 100 hvis den er tom
update config set configvalue = '100' where configkey = 'authentication_timeout' and configvalue  = '';

