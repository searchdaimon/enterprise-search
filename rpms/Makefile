boitho-configfiles_name = boitho-configfiles
boitho-configfiles_version = 0.0.3

daemonize_name = daemonize
daemonize_version = 1.5.2

tlcd-gt14_name = tlcd-gt14
tlcd-gt14_version = 0.0.2

awskeys_name = awskeys
awskeys_version = 1.0.0

boitho-ad_name = boitho-ad
boitho-ad_version = 0.3.34

boitho-perldummy_name = boitho-perldummy
boitho-perldummy_version = 0.0.1

boitho-searchdbb_name = boitho-searchdbb
boitho-searchdbb_version = 5.5.141

boitho-crawlManager_name = boitho-crawlManager
boitho-crawlManager_version = 1.4.117

boitho-boitho-bbdn_name = boitho-bbdn
boitho-boitho-bbdn_version = 1.8.46

boitho-fileFilter_name = boitho-fileFilter
boitho-fileFilter_version = 1.4.0

boitho-base_name = boitho-base
boitho-base_version = 1.3.1

boitho-sdesuser_name = boitho-sdesuser
boitho-sdesuser_version = 1.0.0

boitho-apaceconf_name = boitho-apaceconf
boitho-apaceconf_version = 1.4.0

boitho-syslogconf_name = boitho-syslogconf
boitho-syslogconf_version = 1.0.3

boitho-bbadmin_name = boitho-bbadmin
boitho-bbadmin_version = 2.8.98

boitho-everrun_name = boitho-everrun
boitho-everrun_version = 2.0.5

boitho-infoquery_name = boitho-infoquery
boitho-infoquery_version = 2.0.30

boitho-iindex_name = boitho-iindex
boitho-iindex_version = 2.0.38

boitho-webclient_name = boitho-webclient
boitho-webclient_version = 7.0.27

httplib-ext_name = httplib-ext
httplib-ext_version = 3.0.0

boithobb-setuid_name = boithobb-setuid
boithobb-setuid_version = 1.0.19

boithobb-crawl_watch_name = boithobb-crawl_watch
boithobb-crawl_watch_version = 1.0.22

boithobb-bbAutoUpdate_name = bbAutoUpdate
boithobb-bbAutoUpdate_version = 1.0.3

boithobb-sqlUpdate_name = boithobb-sqlUpdate
boithobb-sqlUpdate_version = 1.7.35

boithobb-unlimitedlicense_name = boithobb-unlimitedlicense
boithobb-unlimitedlicense_version = 1.0.0

boithobb-licensekey_name = boithobb-licensekey
boithobb-licensekey_version = 1.0.0

boithobb-filepermissions_name = boithobb-filepermissions
boithobb-filepermissions_version = 0.5.9

boithobb-database_name = boitho-database
boithobb-database_version = 0.2

boitho-meta_name = boitho-meta
boitho-meta_version = 0.3.2

boitho-phonehome_name = boitho-phonehome
boitho-phonehome_version = 0.13

boitho-dummy_name = boitho-dummy
boitho-dummy_version = 0.1

boitho-local-network_name = boitho-local-network
boitho-local-network_version = 0.1

boitho-auth_name = mod_auth_boitho
boitho-auth_version = 1.7.3

boitho-scripts_name = boitho-scripts
boitho-scripts_version = 1.2.2

boitho-dictionary_name = boitho-dictionary
boitho-dictionary_version = 1.4.4

boitho-list_collections_name = boitho-list_collections
boitho-list_collections_version = 0.0.1

boitho-repomod_name= boitho-repomod
boitho-repomod_version = 1.2

boitho-suggest_name = boitho-suggest
boitho-suggest_version = 1.6.5

boitho-gc_name = boitho-garbage-collection
boitho-gc_version = 1.5.8

boitho-perlxs-sdcrawl_name = boitho-perlxs-sdcrawl
boitho-perlxs-sdcrawl_version = 0.5

boitho-webadd_name = boitho-bbdocumentwebadd
boitho-webadd_version = 0.7.10

boitho-setup_login_name = boitho-setup_login
boitho-setup_login_version = 0.1.1

boitho-vmware_name = boitho-vmware
boitho-vmware_version = 0.1.1

boitho-virtualbox_name = boitho-virtualbox
boitho-virtualbox_version = 0.1.1

boitho-devel_name = boitho-devel
boitho-devel_version = 0.1.2

boitho-hp-tg3_name = boitho-hp-tg3
boitho-hp-tg3_version = 0.1.1

boitho-PageInfobb_name = boitho-PageInfobb
boitho-PageInfobb_version = 0.1.3

boitho-public-htaccess_name = boitho-public-htaccess
boitho-public-htaccess_version = 0.1.0

all: webclient iindex bbadmin boithoad everrun infoquery searchdbb crawlManager2 boitho-bbdn base setuidbb bbAutoUpdate sqldatabase mod_boitho_auth meta daemonize tlcd-gt14 fileFilter filepermissions configfiles crawl_watch scripts repomod suggest perlxs-sdcrawl webadd list_collections virtualbox hp-tg3 devel syslogconf perldummy dictionary setup_login gc sqlUpdate apaceconf httplib-ext

sqlUpdate:
	perl rpmmagic.pl --sql sql/update-cur.sql --requires "boitho-database" $(boithobb-sqlUpdate_name) $(boithobb-sqlUpdate_version) $(BOITHOHOME) /home/boitho/boithoTools 

	rpmbuild -bb $(boithobb-sqlUpdate_name).spec

unlimitedlicense:
	perl rpmmagic.pl --sql sql/unlimitedlicense.sql --requires "boitho-database" $(boithobb-unlimitedlicense_name) $(boithobb-unlimitedlicense_version) $(BOITHOHOME) /home/boitho/boithoTools 

	rpmbuild -bb $(boithobb-unlimitedlicense_name).spec

licensekey:
	perl rpmmagic.pl --sql sql/licensekey.sql --requires "boitho-database" $(boithobb-licensekey_name) $(boithobb-licensekey_version) $(BOITHOHOME) /home/boitho/boithoTools 

	rpmbuild -bb $(boithobb-licensekey_name).spec

filepermissions:
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/blackbox/filepermissions_fix.sh" $(boithobb-filepermissions_name) $(boithobb-filepermissions_version) $(BOITHOHOME) /home/boitho/boithoTools blackbox/filepermissions_fix.sh

	rpmbuild -bb $(boithobb-filepermissions_name).spec


setuidbb:
	perl rpmmagic.pl --DEfattr "%defattr(4755, root, -)" $(boithobb-setuid_name) $(boithobb-setuid_version) $(BOITHOHOME)/blackbox/boithoTools /home/boitho/boithoTools setuid/initwrapper setuid/configwrite setuid/yumwrapper bin/setuidcaller setuid/repomodwrap
	rpmbuild -bb $(boithobb-setuid_name).spec
	echo ""
	echo "***********************************************************"
	echo "Warn: Network interface is hardcoded to eth0. Som need eth1"
	echo "***********************************************************"

htaccess:
	perl rpmmagic.pl --config "public_html/.htaccess" $(boitho-public-htaccess_name) $(boitho-public-htaccess_version) $(BOITHOHOME) /home/boitho/boithoTools 
	rpmbuild -bb $(boitho-public-htaccess_name).spec

webclient:
ifeq ($(arc), i386)
	perl rpmmagic.pl --config "public_html/webclient2/config.pm public_html/robots.txt public_html/webclient2/img/common/logo_smaller.gif public_html/webclient2/img/common/logo_top.jpg public_html/webclient2/favicon.ico" --post "install -o apache -d /home/boitho/boithoTools/var/webclient2_tpl\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/tpl\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/locale/\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/config/\n" --requires "perl-CGI-State perl-Locale-Maketext-Lexicon" $(boitho-webclient_name) $(boitho-webclient_version) $(BOITHOHOME) /home/boitho/boithoTools public_html/webclient2 public_html/amcharts public_html/noaccess.html
else
	perl rpmmagic.pl --config "public_html/webclient2/config.pm public_html/robots.txt public_html/webclient2/img/common/logo_smaller.gif public_html/webclient2/img/common/logo_top.jpg public_html/webclient2/favicon.ico" --post "install -o apache -d /home/boitho/boithoTools/var/webclient2_tpl\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/tpl\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/locale/\n chmod 777 /home/boitho/boithoTools/public_html/webclient2/config/\n" --requires "perl(Time::HiRes) perl(Locale::Maketext::Lexicon)" $(boitho-webclient_name) $(boitho-webclient_version) $(BOITHOHOME) /home/boitho/boithoTools public_html/webclient2 public_html/amcharts public_html/noaccess.html
endif
	rpmbuild -bb $(boitho-webclient_name).spec

httplib-ext:
	perl rpmmagic.pl $(httplib-ext_name) $(httplib-ext_version) $(BOITHOHOME) /home/boitho/boithoTools public_html/lib/ext-3.0-rc2
	rpmbuild -bb $(httplib-ext_name).spec
	
webadd:
	perl rpmmagic.pl --requires "/usr/lib/libxml2.so.2 boithobb-filepermissions" $(boitho-webadd_name) $(boitho-webadd_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/bbdocumentWebAdd
	rpmbuild -bb $(boitho-webadd_name).spec

perlxs-sdcrawl:
	perl rpmmagic.pl --post "chown boitho:boitho /home/boitho/boithoTools/perlxs/ /home/boitho/boithoTools/perlxs/SD-Crawl/" $(boitho-perlxs-sdcrawl_name) $(boitho-perlxs-sdcrawl_version) $(BOITHOHOME) /home/boitho/boithoTools perlxs/SD-Crawl/blib
	rpmbuild -bb $(boitho-perlxs-sdcrawl_name).spec

iindex:
	perl rpmmagic.pl $(boitho-iindex_name) $(boitho-iindex_version) $(BOITHOHOME) /home/boitho/boithoTools bin/IndexerLotbb bin/LotInvertetIndexMaker3bb bin/mergeUserToSubname bin/mergeIIndex bin/readIIndex bin/PageInfobb bin/rreadbb bin/readDocumentIndexbb perl/cleanLots.pl Modules/Boitho/Lot.pm bin/readIIndex bin/sortCrc32attrMap

	rpmbuild -bb $(boitho-iindex_name).spec


bbadmin:
	echo BOITHOHOME
	echo $(BOITHOHOME)
ifeq ($(arc), i386)
	perl rpmmagic.pl --config "public_html/favicon.ico public_html/infopage.html" --pre "mkdir --parents --mode=777 /home/boitho/boithoTools/var/webadmin_tmp/; mkdir --parents --mode=777 /home/boitho/boithoTools/crawlers/; chmod 777 /home/boitho/boithoTools/crawlers/; mkdir --parents --mode=777 /home/boitho/boithoTools/usersystems/; chmod 777 /home/boitho/boithoTools/usersystems/; install -d -o boitho /coredumps/saved; chown -R boitho /coredumps; mv /home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd /home/boitho/boithoTools/config/.htpasswd; chmod 777 /home/boitho/boithoTools/public_html/.htaccess; true" --requires "boitho-configfiles httplib-ext boitho-list_collections perl-JSON-XS perl-Config-Tiny perl-XML-Simple boitho-repomod perl-Apache-Htpasswd perl-File-Copy-Recursive perl-IO-String perl-Archive-Tar perl mysql mysql-server boitho-infoquery boithobb-setuid boithobb-sqlUpdate==1.6.2 bbAutoUpdate perl-SQL-Abstract perl-Readonly perl-Params-Validate perl-DateTime boitho-perldummy" $(boitho-bbadmin_name) $(boitho-bbadmin_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/webadmin Modules 
else
	perl rpmmagic.pl --config "public_html/favicon.ico public_html/infopage.html" --pre "mkdir --parents --mode=777 /home/boitho/boithoTools/var/webadmin_tmp/; mkdir --parents --mode=777 /home/boitho/boithoTools/crawlers/; chmod 777 /home/boitho/boithoTools/crawlers/; mkdir --parents --mode=777 /home/boitho/boithoTools/usersystems/; chmod 777 /home/boitho/boithoTools/usersystems/; install -d -o boitho /coredumps/saved; chown -R boitho /coredumps; mv /home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd /home/boitho/boithoTools/config/.htpasswd; chmod 777 /home/boitho/boithoTools/public_html/.htaccess; true" --requires "boitho-configfiles httplib-ext boitho-list_collections perl(JSON::XS) perl(Config::Tiny) perl(XML::Simple) boitho-repomod perl(Apache::Htpasswd) perl(File::Copy::Recursive) perl(IO::String) perl(Archive::Tar) perl mysql mysql-server boitho-infoquery boithobb-setuid boithobb-sqlUpdate==1.6.2 bbAutoUpdate perl(SQL::Abstract) perl-Readonly perl(Params::Validate) perl-DateTime boitho-perldummy perl(XML::SimpleObject)" $(boitho-bbadmin_name) $(boitho-bbadmin_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/webadmin Modules
endif
	rpmbuild -bb $(boitho-bbadmin_name).spec

boithoad:
	perl rpmmagic.pl --pre "rm -f /home/boitho/boithoTools/logs/boithoad_*" --initd boithoad --requires "boithobb-sqlUpdate>=1.7.10 boithobb-filepermissions" $(boitho-ad_name) $(boitho-ad_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boithoad bin/slicense_info

	rpmbuild -bb $(boitho-ad_name).spec

gc:
	perl rpmmagic.pl $(boitho-gc_name) $(boitho-gc_version) $(BOITHOHOME) /home/boitho/boithoTools bin/gcAuthoritybb

	rpmbuild -bb $(boitho-gc_name).spec

crawl_watch:
	perl rpmmagic.pl --pre "rm -f /home/boitho/boithoTools/logs/crawl_watch.log" --requires "boitho-garbage-collection boithobb-sqlUpdate>=0.5.3-1 boitho-crawlManager>=1.4.12-1 boithobb-filepermissions" --initd crawl_watch $(boithobb-crawl_watch_name) $(boithobb-crawl_watch_version) $(BOITHOHOME) /home/boitho/boithoTools perl/crawl_watch.pl perl/CrawlWatch 

	rpmbuild -bb $(boithobb-crawl_watch_name).spec

bbAutoUpdate:
	perl rpmmagic.pl --requires "boitho-everrun boithobb-filepermissions" --initd bbAutoUpdate $(boithobb-bbAutoUpdate_name) $(boithobb-bbAutoUpdate_version) $(BOITHOHOME) /home/boitho/boithoTools perl/bbAutoUpdate.pl

	rpmbuild -bb $(boithobb-bbAutoUpdate_name).spec

everrun:
	#perl rpmmagic.pl --post "install -d -o boitho /coredumps/saved; chown -R boitho /coredumps" --requires "boithobb-filepermissions gdb" --initd "everrun" $(boitho-everrun_name) $(boitho-everrun_version) $(BOITHOHOME) /home/boitho/boithoTools bin/everrun
	perl rpmmagic.pl $(boitho-everrun_name) $(boitho-everrun_version) $(BOITHOHOME) /home/boitho/boithoTools bin/everrun

	rpmbuild -bb $(boitho-everrun_name).spec

infoquery:
	perl rpmmagic.pl --nostrip $(boitho-infoquery_name) $(boitho-infoquery_version) $(BOITHOHOME) /home/boitho/boithoTools bin/infoquery

	rpmbuild -bb $(boitho-infoquery_name).spec

configfiles:
	perl rpmmagic.pl --post "chown apache:apache /home/boitho/boithoTools/config/.htpasswd" --restartsw $(boitho-configfiles_name) $(boitho-configfiles_version) $(BOITHOHOME)/blackbox/boithoTools /home/boitho/boithoTools config/main.conf config/maplist.conf config/lotlist.conf config/setup.txt config/.htpasswd

	rpmbuild -bb $(boitho-configfiles_name).spec

searchdbb:
	perl rpmmagic.pl --nostrip --pre "mkdir --parents /home/boitho/boithoTools/var/cache/; chown boitho:boitho /home/boitho/boithoTools/var/cache/" --post "sh /home/boitho/boithoTools/blackbox/limitsTuning.sh; chmod -v 6755 /home/boitho/boithoTools/cgi-bin/dispatcher_allbb" --requires "libconfig boitho-syslogconf boithobb-sqlUpdate>=1.7.10 boitho-suggest perl-Text-Iconv boitho-dictionary boitho-configfiles boitho-everrun boithobb-filepermissions" --initd searchdbb $(boitho-searchdbb_name) $(boitho-searchdbb_version) $(BOITHOHOME) /home/boitho/boithoTools config/searchd.conf cgi-bin/dispatcher_allbb bin/searchdbb cgi-bin/ShowThumbbb cgi-bin/ShowCache2bb config/filetypes.eng.conf config/filetypes.nbo.conf config/dispatcher.conf  data/thesaurus  config/file_extensions.conf config/show_attributes.conf config/attribute_descriptions.conf blackbox/limitsTuning.sh

	rpmbuild -bb $(boitho-searchdbb_name).spec


#crawlManager:
#	perl rpmmagic.pl --requires "boitho-everrun boitho-bbdn boithobb-filepermissions /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/auto/XML/Parser/Expat/Expat.so /usr/lib/mysql/libmysqlclient.so.15" --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager crawlers/crawlSMB crawlers/crawlExchange config/crawlmanager.conf 
#
#	rpmbuild -bb $(boitho-crawlManager_name).spec

crawlManager2:

ifeq ($(arc), i386)
	perl rpmmagic.pl --nostrip --requires "/usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/Crypt/SSLeay.pm boitho-syslogconf perl-SOAP-Lite>=0.710.08 perl-Authen-NTLM perl-version perl-HTML-Strip /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/XML/Parser.pm perl-XML-XPath boitho-bbdocumentwebadd boitho-infoquery>=2.0.19-1 boitho-bbdn>=1.8.16-1 boithobb-sqlUpdate>=1.7.10 /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/auto/HTML/Parser/Parser.so boitho-perlxs-sdcrawl boitho-everrun boitho-bbdn boithobb-filepermissions /usr/lib/mysql/libmysqlclient.so.15 perl-Devel-Symdump /usr/lib/perl5/site_perl/5.8.8/i386-linux-thread-multi/auto/XML/LibXML/LibXML.so /usr/lib/perl5/site_perl/5.8.8/i386-linux-thread-multi/auto/XML/LibXML/Common/Common.so" --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager2 crawlers/crawlSMB crawlers/crawlExchange crawlers/Modules crawlers/Intranet crawlers/crawlSO crawlers/crawlExchangePublic crawlers/Sharepoint usersystems/us_ad/us_ad.so usersystems/us_sqlbb/us_sqlbb.so config/crawlmanager.conf perl/persistent.pl crawlers/crawlPush usersystems/us_mapback crawlers/crawlLocal crawlers/RandomDoc/main.pm
else
	perl rpmmagic.pl --nostrip --requires "libsmbclient libconfig perl(Crypt::SSLeay) boitho-syslogconf perl(SOAP::Lite) perl(Authen::NTLM) perl-version perl(HTML::Strip) perl(XML::Parser) perl(XML::XPath) boitho-bbdocumentwebadd boitho-infoquery>=2.0.19-1 boitho-bbdn>=1.8.16-1 boithobb-sqlUpdate>=1.7.10 perl(HTML::Parser) boitho-perlxs-sdcrawl boitho-everrun boitho-bbdn boithobb-filepermissions mysql-libs perl(Devel::Symdump) perl(XML::LibXML) perl(XML::LibXML::Common)" --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager2 crawlers/crawlSMB crawlers/crawlExchange crawlers/Modules crawlers/Intranet crawlers/crawlSO crawlers/crawlExchangePublic crawlers/Sharepoint usersystems/us_ad/us_ad.so usersystems/us_sqlbb/us_sqlbb.so config/crawlmanager.conf perl/persistent.pl crawlers/crawlPush usersystems/us_mapback crawlers/crawlLocal crawlers/RandomDoc/main.pm
endif
	rpmbuild -bb $(boitho-crawlManager_name).spec

scripts:
	perl rpmmagic.pl $(boitho-scripts_name) $(boitho-scripts_version) $(BOITHOHOME) /home/boitho/boithoTools script/eachNight.sh script/reloadDictionary.sh script/makebbwordlist.sh perl/createaspellwordlist.pl script/log_rotation_script
	rpmbuild -bb $(boitho-scripts_name).spec

dictionary:
	perl rpmmagic.pl --requires "boitho-scripts" $(boitho-dictionary_name) $(boitho-dictionary_version) $(BOITHOHOME) /home/boitho/boithoTools bin/dictionarywordsLot data/dict/ data/dict/bb.dat data/dict/bb.multi data/dict/iso-8859-1.cset data/dict/iso-8859-1.cmap
	rpmbuild -bb $(boitho-dictionary_name).spec

list_collections:
	perl rpmmagic.pl $(boitho-list_collections_name) $(boitho-list_collections_version) $(BOITHOHOME) /home/boitho/boithoTools bin/list_collections
	rpmbuild -bb $(boitho-list_collections_name).spec


boitho-bbdn:
ifeq ($(arc), i386)
	perl rpmmagic.pl --nostrip --requires "boitho-everrun boithobb-filepermissions libextractor==0.5.22-1" --initd boitho-bbdn $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boitho-bbdn
else
	perl rpmmagic.pl --nostrip --requires "unzip libconfig boitho-everrun boithobb-filepermissions libextractor>=0.5.22-1" --initd boitho-bbdn $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boitho-bbdn
endif
	rpmbuild -bb $(boitho-boitho-bbdn_name).spec

fileFilter:
	perl rpmmagic.pl --nostrip --initdrestart "boitho-bbdn" --requires "perl-Email-MIME perl-TimeDate" $(boitho-fileFilter_name) $(boitho-fileFilter_version) $(BOITHOHOME) /home/boitho/boithoTools fileFilter

	rpmbuild -bb $(boitho-fileFilter_name).spec

setup_login:
	cd $(BOITHOHOME)/vmware/; make run_setup; cd -
ifeq ($(arc), i386)
	perl rpmmagic.pl --post "sh /home/root/post_install.sh" $(boitho-setup_login_name) $(boitho-setup_login_version) $(BOITHOHOME)/vmware / etc/hosts etc/sysconfig/network home/setup/run_setup home/root/post_install.sh sbin/ifup-local
else
	perl rpmmagic.pl --post "sh /home/root/post_install.sh" $(boitho-setup_login_name) $(boitho-setup_login_version) $(BOITHOHOME)/vmware / home/setup/run_setup home/root/post_install.sh sbin/ifup-local
endif
	rpmbuild -bb $(boitho-setup_login_name).spec

vmware:
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/script/disable_dhcpd.sh; vmware-config-tools.pl -d; perl -ni -le 'if(\$$_ =~ /vmhgfs/) {print qq{#} . \$$_} else {print \$$_}' /etc/fstab" --requires "boitho-setup_login kernel-devel kernel-headers gcc VMwareTools" $(boitho-vmware_name) $(boitho-vmware_version) $(BOITHOHOME) /home/boitho/boithoTools script/disable_dhcpd.sh
	rpmbuild -bb $(boitho-vmware_name).spec

virtualbox:
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/virtualbox/install.sh" --requires "boitho-setup_login kernel-devel kernel-headers gcc dkms" $(boitho-virtualbox_name) $(boitho-virtualbox_version) $(BOITHOHOME) /home/boitho/boithoTools virtualbox/VBoxLinuxAdditions-amd64.run virtualbox/install.sh
	rpmbuild -bb $(boitho-virtualbox_name).spec

hp-tg3:
	perl rpmmagic.pl --requires "hp-tg3-doc>=3.110g-2 kmod-hp-tg3>=3.110g-2" $(boitho-hp-tg3_name) $(boitho-hp-tg3_version) $(BOITHOHOME) /home/boitho/boithoTools
	rpmbuild -bb $(boitho-hp-tg3_name).spec

devel:
ifeq ($(arc), i386)
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/script/setup-dev-environment.sh; install -o boitho -g boitho -d /home/boitho/redhat/; install -o boitho -g boitho -d /home/boitho/redhat/SOURCES/ /home/boitho/redhat/BUILD /home/boitho/redhat/RPMS/x86_64 /home/boitho/redhat/SPECS /home/boitho/redhat/SRPMS /home/boitho/redhat/BUILDROOT; cp /home/boitho/boithoTools/blackbox/rpmmacros /home/boitho/.rpmmacros" --config "config.mk" --requires "nano httpd-devel rpm-build valgrind gdb gcc-c++ /usr/include/gnu/stubs-64.h /usr/lib/perl5/vendor_perl/5.8.8/Error.pm perl-JSON kernel-devel kernel-headers gcc bison db4-devel glibc-devel libconfig libconfig-devel mysql-devel perl-devel libsmbclient-devel libxml2-devel popt-devel git flex /usr/include/gnu/stubs-32.h openldap-devel /usr/include/openssl/opensslconf-x86_64.h libtool curl-devel zlib-devel openssl-devel /usr/lib/libssl.a" $(boitho-devel_name) $(boitho-devel_version) $(BOITHOHOME) /home/boitho/boithoTools script/setup-dev-environment.sh script/enable-root-login.sh script/setup-dev-github.sh blackbox/rpmmacros
else
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/script/setup-dev-environment.sh; install -o boitho -g boitho -d /home/boitho/redhat/; install -o boitho -g boitho -d /home/boitho/redhat/SOURCES/ /home/boitho/redhat/BUILD /home/boitho/redhat/RPMS/x86_64 /home/boitho/redhat/SPECS /home/boitho/redhat/SRPMS /home/boitho/redhat/BUILDROOT; cp /home/boitho/boithoTools/blackbox/rpmmacros /home/boitho/.rpmmacros" --config "config.mk" --requires "nano httpd-devel rpm-build valgrind gdb gcc-c++ /usr/include/gnu/stubs-64.h perl(ExtUtils::Embed) perl(Error) perl(JSON) kernel-devel kernel-headers gcc bison db4-devel glibc-devel libconfig libconfig-devel mysql-devel perl-devel libsmbclient-devel libxml2-devel popt-devel git flex /usr/include/gnu/stubs-32.h openldap-devel /usr/include/openssl/opensslconf-x86_64.h libtool curl-devel zlib-devel openssl-devel" $(boitho-devel_name) $(boitho-devel_version) $(BOITHOHOME) /home/boitho/boithoTools script/setup-dev-environment.sh script/enable-root-login.sh script/setup-dev-github.sh blackbox/rpmmacros
endif
	rpmbuild -bb $(boitho-devel_name).spec

perldummy:
ifeq ($(arc), i386)
	perl rpmmagic.pl --requires "/usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/JSON/XS.pm /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/Crypt/SSLeay.pm" $(boitho-perldummy_name) $(boitho-perldummy_version) $(BOITHOHOME) /home/boitho/boithoTools
else
	perl rpmmagic.pl --requires "perl(JSON::XS) perl(Crypt::SSLeay)" $(boitho-perldummy_name) $(boitho-perldummy_version) $(BOITHOHOME) /home/boitho/boithoTools
endif
	rpmbuild -bb $(boitho-perldummy_name).spec

#	mkdir hp-tg3-kmod-common-3.110g
#	echo dummy > hp-tg3-kmod-common-3.110g/DUMMY
#	tar -z -c -f hp-tg3-kmod-common-3.110g.tar.gz hp-tg3-kmod-common-3.110g
#	mv hp-tg3-kmod-common-3.110g.tar.gz $(HOME)/redhat/SOURCES/
#	rpmbuild -bb hp-tg3-kmod-common-3.110g.spec
	

awskeys:
	rm -rf $(awskeys_name)-$(awskeys_version)
	mkdir $(awskeys_name)-$(awskeys_version)

	mkdir $(awskeys_name)-$(awskeys_version)/init.d/
	cp -v ../init.d/awskeys $(awskeys_name)-$(awskeys_version)/init.d/

	tar -z -c -f $(awskeys_name)-$(awskeys_version).tar.gz $(awskeys_name)-$(awskeys_version)
	mv $(awskeys_name)-$(awskeys_version).tar.gz $(HOME)/redhat/SOURCES/

	rpmbuild -bb $(awskeys_name).spec


tlcd-gt14:
	rm -rf $(tlcd-gt14_name)-$(tlcd-gt14_version)
	mkdir $(tlcd-gt14_name)-$(tlcd-gt14_version)
	
	cp -r ../src/tlcd-gt14/* $(tlcd-gt14_name)-$(tlcd-gt14_version)/

	tar -z -c -f $(tlcd-gt14_name)-$(tlcd-gt14_version).tar.gz $(tlcd-gt14_name)-$(tlcd-gt14_version)
	mv $(tlcd-gt14_name)-$(tlcd-gt14_version).tar.gz $(HOME)/redhat/SOURCES/

	rpmbuild -bb tlcd-gt14.spec


daemonize:
	rm -rf $(daemonize_name)-$(daemonize_version)
	mkdir $(daemonize_name)-$(daemonize_version)
	
	cp -r ../src/daemonize/* $(daemonize_name)-$(daemonize_version)/

	tar -z -c -f $(daemonize_name)-$(daemonize_version).tar.gz $(daemonize_name)-$(daemonize_version)
	mv $(daemonize_name)-$(daemonize_version).tar.gz $(HOME)/redhat/SOURCES/

	rpmbuild -bb boitho-daemonize.spec


sqldatabase:
	perl rpmmagic.pl --pre "/etc/init.d/mysqld start" --post "chkconfig mysqld on; sh /home/boitho/boithoTools/blackbox/boithodbsetup" $(boithobb-database_name) $(boithobb-database_version) $(BOITHOHOME) /home/boitho/boithoTools \
		blackbox/boithobb.sql blackbox/creatuser.sql blackbox/boithodbsetup
	rpmbuild -bb $(boithobb-database_name).spec

repomod:
	perl rpmmagic.pl --requires "boithobb-setuid" $(boitho-repomod_name) $(boitho-repomod_version) $(BOITHOHOME) /home/boitho/boithoTools perl/repomod.pl

	rpmbuild -bb $(boitho-repomod_name).spec
 
base:
	rm -rf $(boitho-base_name)-$(boitho-base_version)
	mkdir $(boitho-base_name)-$(boitho-base_version)

	cp ../blackbox/bbdemo.boitho.com.conf $(boitho-base_name)-$(boitho-base_version)

	tar -z -c -f $(boitho-base_name)-$(boitho-base_version).tar.gz $(boitho-base_name)-$(boitho-base_version)
	mv $(boitho-base_name)-$(boitho-base_version).tar.gz $(HOME)/redhat/SOURCES/	

	rpmbuild -bb $(boitho-base_name).spec

sdesuser:
	rm -rf $(boitho-sdesuser_name)-$(boitho-sdesuser_version)
	mkdir $(boitho-sdesuser_name)-$(boitho-sdesuser_version)

	cp ../blackbox/sdes-authorized_keys $(boitho-sdesuser_name)-$(boitho-sdesuser_version)/authorized_keys

	tar -z -c -f $(boitho-sdesuser_name)-$(boitho-sdesuser_version).tar.gz $(boitho-sdesuser_name)-$(boitho-sdesuser_version)
	mv $(boitho-sdesuser_name)-$(boitho-sdesuser_version).tar.gz $(HOME)/redhat/SOURCES/

	rpmbuild -bb $(boitho-sdesuser_name).spec

syslogconf:

	rm -rf $(boitho-syslogconf_name)-$(boitho-syslogconf_version)
	mkdir $(boitho-syslogconf_name)-$(boitho-syslogconf_version)

	cp ../blackbox/bb.rsyslog.conf $(boitho-syslogconf_name)-$(boitho-syslogconf_version)

	tar -z -c -f $(boitho-syslogconf_name)-$(boitho-syslogconf_version).tar.gz $(boitho-syslogconf_name)-$(boitho-syslogconf_version)
	mv $(boitho-syslogconf_name)-$(boitho-syslogconf_version).tar.gz $(HOME)/redhat/SOURCES/	

	rpmbuild -bb $(boitho-syslogconf_name).spec


apaceconf:

	rm -rf $(boitho-apaceconf_name)-$(boitho-apaceconf_version)
	mkdir $(boitho-apaceconf_name)-$(boitho-apaceconf_version)

	cp ../blackbox/bbdemo.boitho.com.conf $(boitho-apaceconf_name)-$(boitho-apaceconf_version)

	tar -z -c -f $(boitho-apaceconf_name)-$(boitho-apaceconf_version).tar.gz $(boitho-apaceconf_name)-$(boitho-apaceconf_version)
	mv $(boitho-apaceconf_name)-$(boitho-apaceconf_version).tar.gz $(HOME)/redhat/SOURCES/	

	rpmbuild -bb $(boitho-apaceconf_name).spec

meta:
	rm -rf $(boitho-meta_name)-$(boitho-meta_version)
	mkdir $(boitho-meta_name)-$(boitho-meta_version)
	echo boitho-meta > $(boitho-meta_name)-$(boitho-meta_version)/boitho-meta
	tar -z -c -f $(boitho-meta_name)-$(boitho-meta_version).tar.gz $(boitho-meta_name)-$(boitho-meta_version)
	mv $(boitho-meta_name)-$(boitho-meta_version).tar.gz $(HOME)/redhat/SOURCES/
	rpmbuild -bb $(boitho-meta_name).spec


phonehome:
	rm -rf $(boitho-phonehome_name)-$(boitho-phonehome_version)
	mkdir $(boitho-phonehome_name)-$(boitho-phonehome_version)
	cp $(BOITHOHOME)/bin/bb-client.pl $(boitho-phonehome_name)-$(boitho-phonehome_version)
	cp $(BOITHOHOME)/bin/bbph-keep-alive.pl $(boitho-phonehome_name)-$(boitho-phonehome_version)
	cp $(BOITHOHOME)/config/bb-phone-home-client.conf $(boitho-phonehome_name)-$(boitho-phonehome_version)
	cp $(BOITHOHOME)/phonehome/id_rsa $(boitho-phonehome_name)-$(boitho-phonehome_version)
	cp $(BOITHOHOME)/phonehome/ssh_config $(boitho-phonehome_name)-$(boitho-phonehome_version)
	mkdir $(boitho-phonehome_name)-$(boitho-phonehome_version)/init.d
	cp $(BOITHOHOME)/init.d/phonehome $(boitho-phonehome_name)-$(boitho-phonehome_version)/init.d
	tar -z -c -f $(boitho-phonehome_name)-$(boitho-phonehome_version).tar.gz $(boitho-phonehome_name)-$(boitho-phonehome_version)
	mv $(boitho-phonehome_name)-$(boitho-phonehome_version).tar.gz $(HOME)/redhat/SOURCES/
	rpmbuild -bb $(boitho-phonehome_name).spec


dummy:
	rm -rf $(boitho-dummy_name)-$(boitho-dummy_version)
	mkdir $(boitho-dummy_name)-$(boitho-dummy_version)
	echo dummy > $(boitho-dummy_name)-$(boitho-dummy_version)/DUMMY
	tar -z -c -f $(boitho-dummy_name)-$(boitho-dummy_version).tar.gz $(boitho-dummy_name)-$(boitho-dummy_version)
	mv $(boitho-dummy_name)-$(boitho-dummy_version).tar.gz $(HOME)/redhat/SOURCES/
	rpmbuild -bb $(boitho-dummy_name).spec



mod_boitho_auth:
	rm -rf $(boitho-auth_name)-$(boitho-auth_version)
	mkdir $(boitho-auth_name)-$(boitho-auth_version)
	cp $(BOITHOHOME)/src/mod_auth_boitho/.libs/mod_auth_boitho.so $(boitho-auth_name)-$(boitho-auth_version)/
	tar -z -c -f $(boitho-auth_name)-$(boitho-auth_version).tar.gz $(boitho-auth_name)-$(boitho-auth_version)
	mv $(boitho-auth_name)-$(boitho-auth_version).tar.gz $(HOME)/redhat/SOURCES/
	rpmbuild -bb $(boitho-auth_name).spec

suggest:
	perl rpmmagic.pl --sysconfig "rpcbind" --post "/etc/init.d/rpcbind restart" --requires "boitho-apaceconf>=1.0.0 boitho-everrun boitho-dictionary" --initd suggest $(boitho-suggest_name) $(boitho-suggest_version) $(BOITHOHOME) /home/boitho/boithoTools bin/suggest_server cgi-bin/suggest_webclient bin/suggest_client_redict

	rpmbuild -bb $(boitho-suggest_name).spec

PageInfobb:
	perl rpmmagic.pl $(boitho-PageInfobb_name) $(boitho-PageInfobb_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/PageInfobb
	rpmbuild -bb $(boitho-PageInfobb_name).spec


local-network:
	rm -rf $(boitho-local-network_name)-$(boitho-local-network_version)
	mkdir $(boitho-local-network_name)-$(boitho-local-network_version)
	echo local-network > $(boitho-local-network_name)-$(boitho-local-network_version)/DUMMY
	tar -z -c -f $(boitho-local-network_name)-$(boitho-local-network_version).tar.gz $(boitho-local-network_name)-$(boitho-local-network_version)
	mv $(boitho-local-network_name)-$(boitho-local-network_version).tar.gz $(HOME)/redhat/SOURCES/
	rpmbuild -bb $(boitho-local-network_name).spec



clean:
	#rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
	#sletter buggde pakker
	rm -rf /home/boitho/redhat/*/*

install:
	cp --verbose /home/boitho/redhat/RPMS/i386/boitho*.rpm ~/FedoraRepository/boitho/updates/4/i386/




