alter table collectionAuth add `comment` varchar(255) default NULL;

INSERT INTO `config` VALUES ('msad_ldapstring','');
INSERT INTO `config` VALUES ('msad_ldapbase','');
delete from connectors where name <> "SMB";

INSERT INTO `connectors` VALUES (9, 'Exchange', 'Microsoft exchange', NULL, NULL, NULL, 0, 'authentication, connector, crawling, exchange_user_select');

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

