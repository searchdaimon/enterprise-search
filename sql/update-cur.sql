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

