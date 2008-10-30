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


-- New usersystem handling

CREATE TABLE system (
  id int(10) unsigned NOT NULL auto_increment,
  name varchar(255) NOT NULL default 'Unnamed system',
  ip varchar(32) default NULL,
  user varchar(255) default NULL,
  password varchar(255) default NULL,
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


-- Active directory system
INSERT INTO system (id, name, ip, user, password, is_primary, connector) SELECT 1 AS id, 'Active Directory' AS name, c3.configvalue AS ip, c1.configvalue AS user, c2.configvalue AS password, 1 AS is_primary, 1 AS connector FROM config AS c1, config AS c2, config AS c3 WHERE c1.configkey = 'msad_user' AND c2.configkey = 'msad_password' AND c3.configkey = 'msad_ip';

INSERT INTO systemParam VALUES (1,'ldapstring',NULL);
INSERT INTO systemParam VALUES (1,'domain',NULL);
INSERT INTO systemParam VALUES (1,'ldapbase',NULL);
INSERT INTO systemParam VALUES (1,'ldapgroupstring',NULL);

INSERT INTO systemParamValue (param, system, value) SELECT 'domain' as param, 1 AS system, configvalue AS value FROM config WHERE config.configkey = 'msad_domain'; 
INSERT INTO systemParamValue (param, system, value) VALUES('ldapstring', 1, '');
INSERT INTO systemParamValue (param, system, value) VALUES('ldapbase', 1, '');
INSERT INTO systemParamValue (param, system, value) VALUES('ldapgroupstring', 1, '');

INSERT INTO config (configkey, configvalue) VALUES('licensekey', '');
INSERT INTO config (configkey, configvalue) VALUES('licensesystem', 'ohSh7oow');

ALTER TABLE `shares` add `system` int(10);
UPDATE shares SET system = 1 WHERE system IS NULL;
