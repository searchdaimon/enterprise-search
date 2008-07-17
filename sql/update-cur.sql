alter table collectionAuth add `comment` varchar(255) default NULL;

INSERT INTO `config` VALUES ('msad_ldapstring','');
INSERT INTO `config` VALUES ('msad_ldapbase','');
delete from connectors where name <> "SMB";



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

ALTER TABLE connectors add `active` tinyint(4) NOT NULL default '0';
ALTER TABLE connectors add `extension` tinyint(4) default NULL;
ALTER TABLE connectors add `modified` datetime default NULL;

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
  
INSERT INTO `connectors` VALUES (9, 'Exchange', 'Microsoft exchange', NULL, NULL, NULL, 0, 'authentication, connector, crawling, exchange_user_select',1, NULL, 0);
INSERT INTO connectors VALUES (64,'Intranet',NULL,NULL,NULL,NULL,0,'custom_parameters, crawling',1,'2008-07-14 21:01:04',1);

INSERT INTO param VALUES (102,64,'isPasswordProtected','1 = user must be able to log in, 0 = skip online password check.');
INSERT INTO param VALUES (101,64,'url','http://www.example.com');

