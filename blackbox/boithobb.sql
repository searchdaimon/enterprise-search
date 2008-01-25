drop table if exists brukere;
drop table if exists session_id;
drop table if exists connectors;
drop table if exists connectorsCustomField;
drop table if exists connectorsFor;
drop table if exists dataSources;
drop table if exists groups;
drop table if exists groupMembership;
drop table if exists groupPermissions;
drop table if exists dataSourcesCustomField;
--
-- Table structure for table `brukere`
--

CREATE TABLE brukere (
  bruker_navn char(20) NOT NULL default '',
  passord char(10) NOT NULL default '',
  PRIMARY KEY  (bruker_navn)
) TYPE=MyISAM;


insert into brukere values("admin","12345");


CREATE TABLE session_id (
  Id int(9) unsigned NOT NULL default '0',
  User_Name char(100) NOT NULL default '',
  Log_in_Date datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (Id)
) TYPE=MyISAM;


CREATE TABLE connectors (
	id int(7) unsigned NOT NULL auto_increment,
	name varchar(255) NOT NULL,
	comment text,
	commandFirst varchar(255),
	commandUpdate varchar(255),
	commandTest varchar(255),
 	PRIMARY KEY  (id)	
);


insert into connectors values(NULL,"SMB","SMB (Server Message Block). Protocol used in Windows networks to allow resources such as files on one machine to be shared on other machines as if they were local.","smbcrawler.pl #host #user%#password 0 #dataSourcesname","smbcrawler.pl #host #user%#password #lastrununixtime #dataSourcesname","");

insert into connectors values(NULL,"ODBC","ODBC (Open DataBase Connectivity). A standard application programming interface (API) for accessing data in both relational and non-relational database management systems.","odbcclient $server $user $password \"select * from table\" $tempdir","","");

insert into connectors values(NULL,"LDAP","LDAP (Lightweight Directory Access Protocol). It is a protocol for accessing information directories such as organizations, individuals, phone numbers, and addresses.","ldapcclient $server $user $password \"select * from table\" $tempdir","","");
insert into connectors values(NULL,"MySQL","MySQL is the world's most popular open source database. This is for the native api. One can also use odbc.","mysqlcclient $server $user $password \"select * from table\" $tempdir","","");


CREATE TABLE connectorsCustomField (
        id int(7) unsigned NOT NULL auto_increment,
	connectorsId int(7) unsigned NOT NULL,
        name varchar(255) NOT NULL,
        prefix varchar(255),
        comment varchar(255),
        PRIMARY KEY  (id)
);

insert into connectorsCustomField values(NULL,1,"user","","");
insert into connectorsCustomField values(NULL,1,"password","","");



CREATE TABLE connectorsFor (
	id int(7) unsigned NOT NULL auto_increment,
	connectorsId int(7) unsigned NOT NULL,
	name varchar(255) NOT NULL,
 	PRIMARY KEY  (id)	
);

insert into connectorsFor values(NULL,1,"Boitho Personnal Space");
insert into connectorsFor values(NULL,1,"Shared files from Microsoft Windows");

insert into connectorsFor values(NULL,2,"Microsoft SQL server");
insert into connectorsFor values(NULL,2,"Oracle database server");
insert into connectorsFor values(NULL,2,"Sybase");
insert into connectorsFor values(NULL,2,"DB2");
insert into connectorsFor values(NULL,2,"IBM AS/400");
insert into connectorsFor values(NULL,2,"MySQL SQL server odbc");

insert into connectorsFor values(NULL,3,"Microsoft Exchange server");
insert into connectorsFor values(NULL,3,"Eudora Internet Mail Server");

insert into connectorsFor values(NULL,4,"MySQL SQL server native api (faster and easyer the odbc)");


CREATE TABLE dataSources (
	id int(7) unsigned NOT NULL auto_increment,
	name char(20) not null,
	connectorsId int(7) unsigned NOT NULL,
	host varchar(255),
	nrofPages int(7) NOT NULL,
	diskSize int (7) NOT NULL,
	lastRun datetime NOT NULL default '0000-00-00 00:00:00',
	indexInterval int(7) NOT NULL default 0, 
 	PRIMARY KEY  (id)	
);

insert into dataSources values(NULL,"Norsk_wikopedia",1,"//129.241.50.224/wikipedia_no",67518,263,0,0);


CREATE TABLE dataSourcesCustomField (
        id int(7) unsigned NOT NULL auto_increment,
	connectorsCustomField int(7) unsigned NOT NULL,
        dataSourcesId int(7) unsigned NOT NULL,
        value varchar(255) NOT NULL,
        PRIMARY KEY  (id)
);



CREATE TABLE groups (
	groupname varchar(20) NOT NULL,
 	PRIMARY KEY  (groupname)	
);

insert into groups values("users");

CREATE TABLE groupMembership (        
	groupname varchar(255) NOT NULL,
	bruker_navn char(20) NOT NULL,
	FOREIGN KEY (groupname) REFERENCES groups(groupname),
	FOREIGN KEY (bruker_navn) REFERENCES brukere(bruker_navn),
        PRIMARY KEY  (groupname,bruker_navn)
);

insert into groupMembership values("users","admin");


CREATE TABLE groupPermissions (        
	id int(7) unsigned NOT NULL auto_increment,
	groupname varchar(255) NOT NULL,
	dataSourcesName char(20) NOT NULL,
	FOREIGN KEY (groupname) REFERENCES groups(groupname),
	FOREIGN KEY (dataSourcesName) REFERENCES dataSources(name),
        PRIMARY KEY  (id)
);


-- Boitho bb test data

-- boitho
--insert into brukere values("runarb","bb12345");
--insert into brukere values("magnusga","bb12345");
--insert into brukere values("martibre","bb12345");

-- 24so
--insert into brukere values("sr","bb12345");
--insert into brukere values("eo","bb12345");

-- forskningsparken
--insert into brukere values("lmkrohn","bb12345");
--insert into brukere values("lillekjendlie","bb12345");
--insert into brukere values("thomasryd","bb12345");

-- ig
--insert into brukere values("bih","bb12345");


--insert into groups values("boitho");
--insert into groups values("24sevenoffice");
--insert into groups values("forskningsparken");
--insert into groups values("ig ntnu");

--insert into groupMembership values("users","runarb");
--insert into groupMembership values("users","magnusga");
--insert into groupMembership values("users","martibre");
--insert into groupMembership values("users","sr");
--insert into groupMembership values("users","eo");
--insert into groupMembership values("users","lmkrohn");
--insert into groupMembership values("users","lillekjendlie");
--insert into groupMembership values("users","thomasryd");
--insert into groupMembership values("users","bih");

--insert into groupMembership values("boitho","runarb");
--insert into groupMembership values("boitho","magnusga");
--insert into groupMembership values("boitho","martibre");
--insert into groupMembership values("24sevenoffice","sr");
--insert into groupMembership values("24sevenoffice","eo");
--insert into groupMembership values("forskningsparken","lmkrohn");
--insert into groupMembership values("forskningsparken","lillekjendlie");
--insert into groupMembership values("forskningsparken","thomasryd");
--insert into groupMembership values("ig ntnu","bih");

--insert into groups values("runarb");
--insert into groups values("magnusga");
--insert into groups values("martibre");
--insert into groups values("sr");
--insert into groups values("eo");
--insert into groups values("lmkrohn");
--insert into groups values("lillekjendlie");
--insert into groups values("thomasryd");
--insert into groups values("bih");

--insert into groupMembership values("runarb","runarb");
--insert into groupMembership values("magnusga","magnusga");
--insert into groupMembership values("martibre","martibre");
--insert into groupMembership values("sr","sr");
--insert into groupMembership values("eo","eo");
--insert into groupMembership values("lmkrohn","lmkrohn");
--insert into groupMembership values("lillekjendlie","lillekjendlie");
--insert into groupMembership values("thomasryd","thomasryd");
--insert into groupMembership values("bih","bih");


--insert into dataSources values(NULL,"Boitho_stuff",2,"//129.241.50.224/boitho",0,0,0,0);
--insert into dataSources values(NULL,"24so_stuff",2,"//129.241.50.224/24so",0,0,0,0);
--insert into dataSources values(NULL,"fp_stuff",2,"//129.241.50.224/fp",0,0,0,0);
--insert into dataSources values(NULL,"ig_stuff",2,"//129.241.50.224/ig",0,0,0,0);

--insert into groupPermissions values(NULL,"boitho","Boitho_stuff");
--insert into groupPermissions values(NULL,"24sevenoffice","24so_stuff");
--insert into groupPermissions values(NULL,"forskningsparken","fp_stuff");
--insert into groupPermissions values(NULL,"ig ntnu","ig_stuff");

--insert into groupPermissions values(NULL,"users","Norsk_wikopedia");


--
--insert into dataSources values(NULL,"runarb_ps",1,"//localhost/runarb",0,0,0,60);
--insert into dataSourcesCustomField values(NULL,1,6,"runarb");
--insert into dataSourcesCustomField values(NULL,2,6,"bb12345");
--insert into groupPermissions values(NULL,"runarb","runarb_ps");

CREATE TABLE fileq (
	id int(9) unsigned NOT NULL auto_increment,
        bruker_navn char(20) NOT NULL,
        uri varchar(255) NOT NULL,
        url varchar(255) NOT NULL,
        PRIMARY KEY  (id)
);


CREATE TABLE config (
        configkey varchar(255) NOT NULL,
        configvalue varchar(255) NOT NULL,
        PRIMARY KEY  (configkey)
);

insert into config values("authenticatmetod","msad");
insert into config values("msad_domain","ad.boitho.com");
insert into config values("msad_user","Administrator");
insert into config values("msad_password","Ju13brz;");
insert into config values("msad_ip","127.0.0.1");
--insert into config values("msad_ip","129.241.50.208");
insert into config values("msad_port","3666");
--insert into config values("msad_port","389");
-- Builtin or Users
insert into config values("msad_group","Users");

insert into config values("msad_ldapstring","CN=Black Box,OU=Users,OU=EA-Forskningsparken,OU=Customers,DC=i04,DC=local");
insert into config values("msad_ldapbase","OU=Users,OU=EA-Forskningsparken,OU=Customers,DC=i04,DC=local");
insert into config values("sudo","water66");

UPDATE `config` SET `configvalue` = 'bbtest.searchdaimon.com' WHERE `configkey` = 'msad_domain';
UPDATE `config` SET `configvalue` = 'bbtest.searchdaimon.com\Boitho' WHERE `configkey` = 'msad_user';
UPDATE `config` SET `configvalue` = '1234Asd' WHERE `configkey` = 'msad_password';
UPDATE `config` SET `configvalue` = '213.179.58.120' WHERE `configkey` = 'msad_ip';
UPDATE `config` SET `configvalue` = '389' WHERE `configkey` = 'msad_port';
UPDATE `config` SET `configvalue` = 'Builtin' WHERE `configkey` = 'Builtin';

