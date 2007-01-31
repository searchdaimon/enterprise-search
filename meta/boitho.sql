-- MySQL dump 8.23
--
-- Host: localhost    Database: boitho
---------------------------------------------------------
-- Server version	3.23.58

--
-- Table structure for table `betaler_keywords`
--

CREATE TABLE betaler_keywords (
  kid int(7) unsigned NOT NULL auto_increment,
  betaler_side_id  int(7) unsigned NOT NULL default '0',
  keyword char(20) NOT NULL default '',
  betaler float NOT NULL default '0',
  PRIMARY KEY  (kid),
  KEY betaler_side_id (betaler_side_id),
  KEY keyword (keyword)
) TYPE=MyISAM;

CREATE TABLE betaler_keywords_visninger (
  id int(7) unsigned NOT NULL auto_increment,
  kid int(7) unsigned NOT NULL,
  betaler_side_id  int(7) unsigned NOT NULL,
  tid datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (id)
)
--
-- Table structure for table `betaler_sider`
--

CREATE TABLE betaler_sider (
  id int(7) unsigned NOT NULL auto_increment,
  url varchar(200) NOT NULL default '',
  tittel varchar(80) NOT NULL default '',
  beskrivelse text NOT NULL,
  bruker_navn varchar(20) NOT NULL default '',
  sprok char(2) default NULL,
  PRIMARY KEY  (id)
) TYPE=MyISAM;

--
-- Table structure for table `brukere`
--

CREATE TABLE brukere (
  bruker_navn char(20) NOT NULL default '',
  passord char(10) NOT NULL default '',
  mail char(50) NOT NULL default '',
  navn char(50) NOT NULL default '',
  adresse char(50) NOT NULL default '',
  penger float NOT NULL default '0',
  nettsteded char(255) NOT NULL default '',
  status char(5) NOT NULL default '',
  PRIMARY KEY  (bruker_navn)
) TYPE=MyISAM;

--
-- Table structure for table `gruppe`
--

CREATE TABLE gruppe (
  gruppe char(20) NOT NULL default '',
  bruker_navn char(20) NOT NULL default '',
  PRIMARY KEY  (gruppe,bruker_navn)
) TYPE=MyISAM;

--
-- Table structure for table `keywords_10`
--

--
-- Table structure for table `news`
--

CREATE TABLE news (
  Id int(5) unsigned NOT NULL auto_increment,
  Headline varchar(255) NOT NULL default '',
  Message text NOT NULL,
  Create_date date NOT NULL default '0000-00-00',
  PRIMARY KEY  (Id)
) TYPE=MyISAM;


CREATE TABLE short_out_logg (
        betaler_side_id int(7) NOT NULL,
        ip_adresse char(15) NOT NULL,
        clickfrequency int(7) NOT NULL default '0',
        tid datetime NOT NULL default '0000-00-00 00:00:00',
        PRIMARY KEY  (betaler_side_id,ip_adresse,tid)
)

--
-- Table structure for table `out_logg`
--

CREATE TABLE out_logg (
  id int(6) NOT NULL auto_increment,
  tid datetime NOT NULL default '0000-00-00 00:00:00',
  query char(30) NOT NULL default '',
  search_bruker char(20) default NULL,
  link_bruker char(20) default NULL,
  betaler_keyword_id int(7) NOT NULL default '0',
  ip_adresse char(15) default NULL,
  referer char(255) default NULL,
  betaler float NOT NULL default '0',
  status varchar(32),
  PRIMARY KEY  (id)
) TYPE=MyISAM;

--
-- Table structure for table `search_logg`
--

CREATE TABLE search_logg (
  id int(6) NOT NULL auto_increment,
  tid datetime NOT NULL default '0000-00-00 00:00:00',
  query varchar(30) NOT NULL default '',
  search_bruker varchar(20) default NULL,
  treff int(5) unsigned NOT NULL default '0',
  search_tid decimal(12,10) unsigned NOT NULL default '0.0000000000',
  ip_adresse varchar(15) NOT NULL default '',
  side int(3) NOT NULL default '0',
  betaler_keywords_treff int(3) NOT NULL default '0',
  HTTP_ACCEPT_LANGUAGE varchar(255) default NULL,
  HTTP_USER_AGENT varchar(255) default NULL,
  HTTP_REFERER varchar(255) default NULL,
  GeoIPLang char(2) default NULL,
  PRIMARY KEY  (id)
) TYPE=MyISAM;

--
-- Table structure for table `search_logg_store`
--

CREATE TABLE search_logg_store (
  id int(6) NOT NULL auto_increment,
  tid date NOT NULL default '0000-00-00',
  query char(30) NOT NULL default '',
  PRIMARY KEY  (id)
) TYPE=MyISAM;

--
-- Table structure for table `session_id`
--

CREATE TABLE session_id (
  Id int(9) unsigned NOT NULL default '0',
  User_Name char(100) NOT NULL default '',
  Log_in_Date datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (Id)
) TYPE=MyISAM;

--
-- Table structure for table `sider`
--

--
-- Table structure for table `transactions`
--

CREATE TABLE transactions (
  transactions_id int(5) unsigned NOT NULL auto_increment,
  user_Name char(20) NOT NULL default '',
  date date NOT NULL default '0000-00-00',
  coment char(100) default NULL,
  amount float default NULL,
  service_account_id int(5) default NULL,
  status char(20) default NULL,
  KEY transactions_id (transactions_id),
  KEY user_Name (user_Name)
) TYPE=MyISAM;


-- tabell over utstette anonser
CREATE TABLE issuedadds (
  id int(7) unsigned NOT NULL auto_increment,
  keyword char(20) NOT NULL,
  bid float NOT NULL,
  uri BLOB NOT NULL,
  issuetime DATETIME NOT NULL,
  clickfrequency int(7) NOT NULL default '0',
  ppcuser char(20) NOT NULL,
  affuser char(20) NOT NULL,
  ipadress char(15) NOT NULL,
  HTTP_ACCEPT_LANGUAGE varchar(12),
  HTTP_USER_AGENT varchar(64),
  HTTP_REFERER varchar(255),
  betaler_keyword_id int(7) NOT NULL default '0',
  betaler_side_id int(7) NOT NULL default '0',
  PRIMARY KEY  (id)
);

