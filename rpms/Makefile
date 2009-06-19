boitho-configfiles_name = boitho-configfiles
boitho-configfiles_version = 0.0.3

daemonize_name = daemonize
daemonize_version = 1.5.2

tlcd-gt14_name = tlcd-gt14
tlcd-gt14_version = 0.0.2

boitho-ad_name = boitho-ad
boitho-ad_version = 0.3.31

boitho-searchdbb_name = boitho-searchdbb
boitho-searchdbb_version = 5.5.113

boitho-crawlManager_name = boitho-crawlManager
boitho-crawlManager_version = 1.4.93

boitho-boitho-bbdn_name = boitho-bbdn
boitho-boitho-bbdn_version = 1.8.37

boitho-fileFilter_name = boitho-fileFilter
boitho-fileFilter_version = 1.3.19

boitho-base_name = boitho-base
boitho-base_version = 1.3.0

boitho-apaceconf_name = boitho-apaceconf
boitho-apaceconf_version = 1.3.0

boitho-bbadmin_name = boitho-bbadmin
boitho-bbadmin_version = 2.8.73

boitho-everrun_name = boitho-everrun
boitho-everrun_version = 2.0.4

boitho-infoquery_name = boitho-infoquery
boitho-infoquery_version = 2.0.23

boitho-iindex_name = boitho-iindex
boitho-iindex_version = 2.0.30

boitho-webclient_name = boitho-webclient
boitho-webclient_version = 6.0.73

boithobb-setuid_name = boithobb-setuid
boithobb-setuid_version = 1.0.19

boithobb-crawl_watch_name = boithobb-crawl_watch
boithobb-crawl_watch_version = 1.0.19

boithobb-bbAutoUpdate_name = bbAutoUpdate
boithobb-bbAutoUpdate_version = 1.0.3

boithobb-sqlUpdate_name = boithobb-sqlUpdate
boithobb-sqlUpdate_version = 1.7.23

boithobb-licensekey_name = boithobb-licensekey
boithobb-licensekey_version = 1.0.0

boithobb-filepermissions_name = boithobb-filepermissions
boithobb-filepermissions_version = 0.5.9

boithobb-database_name = boitho-database
boithobb-database_version = 0.2

boitho-meta_name = boitho-meta
boitho-meta_version = 0.3.2

boitho-phonehome_name = boitho-phonehome
boitho-phonehome_version = 0.9

boitho-dummy_name = boitho-dummy
boitho-dummy_version = 0.1

boitho-auth_name = mod_auth_boitho
boitho-auth_version = 1.13

boitho-scripts_name = boitho-scripts
boitho-scripts_version = 1.2

boitho-dictionary_name = boitho-dictionary
boitho-dictionary_version = 1.4.3

boitho-list_collections_name = boitho-list_collections
boitho-list_collections_version = 0.0.1

boitho-repomod_name= boitho-repomod
boitho-repomod_version = 1.2

boitho-suggest_name = boitho-suggest
boitho-suggest_version = 1.6.2

boitho-gc_name = boitho-garbage-collection
boitho-gc_version = 1.5.3

boitho-perlxs-sdcrawl_name = boitho-perlxs-sdcrawl
boitho-perlxs-sdcrawl_version = 0.3

boitho-webadd_name = boitho-bbdocumentwebadd
boitho-webadd_version = 0.7.4

boitho-setup_login_name = boitho-setup_login
boitho-setup_login_version = 0.1.1

boitho-PageInfobb_name = boitho-PageInfobb
boitho-PageInfobb_version = 0.1.3


all: webclient iindex bbadmin boithoad everrun infoquery searchdbb crawlManager2 boitho-bbdn base setuidbb bbAutoUpdate sqldatabase mod_boitho_auth meta phonehome daemonize tlcd-gt14 fileFilter filepermissions configfiles crawl_watch scripts repomod suggest perlxs-sdcrawl webadd list_collections

sqlUpdate:
	perl rpmmagic.pl --sql sql/update-cur.sql --requires "boitho-database" $(boithobb-sqlUpdate_name) $(boithobb-sqlUpdate_version) $(BOITHOHOME) /home/boitho/boithoTools 

	rpmbuild -bb $(boithobb-sqlUpdate_name).spec

licensekey:
	perl rpmmagic.pl --sql sql/licensekey.sql --requires "boitho-database" $(boithobb-licensekey_name) $(boithobb-licensekey_version) $(BOITHOHOME) /home/boitho/boithoTools 

	rpmbuild -bb $(boithobb-licensekey_name).spec

filepermissions:
	perl rpmmagic.pl --post "sh /home/boitho/boithoTools/blackbox/filepermissions_fix.sh" $(boithobb-filepermissions_name) $(boithobb-filepermissions_version) $(BOITHOHOME) /home/boitho/boithoTools blackbox/filepermissions_fix.sh

	rpmbuild -bb $(boithobb-filepermissions_name).spec


setuidbb:
	perl rpmmagic.pl --DEfattr "%defattr(4755, root, -)" $(boithobb-setuid_name) $(boithobb-setuid_version) $(BOITHOHOME) /home/boitho/boithoTools setuid/initwrapper setuid/configwrite setuid/yumwrapper setuid/yumupdate bin/setuidcaller setuid/repomodwrap

	rpmbuild -bb $(boithobb-setuid_name).spec

webclient:
	perl rpmmagic.pl --post "install -o apache -d /home/boitho/boithoTools/var/webclient2_tpl" --requires "perl-CGI-State perl-Locale-Maketext-Lexicon" $(boitho-webclient_name) $(boitho-webclient_version) $(BOITHOHOME) /home/boitho/boithoTools public_html/webclient2 public_html/.htaccess public_html/amcharts public_html/noaccess.html
	rpmbuild -bb $(boitho-webclient_name).spec

webadd:
	perl rpmmagic.pl --requires "/usr/lib/libxml2.so.2 boithobb-filepermissions" $(boitho-webadd_name) $(boitho-webadd_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/bbdocumentWebAdd
	rpmbuild -bb $(boitho-webadd_name).spec

perlxs-sdcrawl:
	perl rpmmagic.pl $(boitho-perlxs-sdcrawl_name) $(boitho-perlxs-sdcrawl_version) $(BOITHOHOME) /home/boitho/boithoTools perlxs/SD-Crawl/blib
	rpmbuild -bb $(boitho-perlxs-sdcrawl_name).spec

iindex:
	perl rpmmagic.pl $(boitho-iindex_name) $(boitho-iindex_version) $(BOITHOHOME) /home/boitho/boithoTools bin/IndexerLotbb bin/LotInvertetIndexMaker3bb bin/mergeUserToSubname bin/mergeIIndex bin/readIIndex bin/PageInfobb bin/rreadbb bin/readDocumentIndexbb perl/cleanLots.pl Modules/Boitho/Lot.pm

	rpmbuild -bb $(boitho-iindex_name).spec


bbadmin:
	echo BOITHOHOME
	echo $(BOITHOHOME)
	perl rpmmagic.pl --pre "mkdir --parents --mode=777 /home/boitho/boithoTools/var/webadmin_tmp/; chmod 777 /home/boitho/boithoTools/crawlers/" --requires "boitho-list_collections perl-JSON-XS perl-Config-Tiny perl-XML-Simple boitho-repomod perl-Apache-Htpasswd perl-File-Copy-Recursive perl-IO-String perl-Archive-Tar perl mysql mysql-server boitho-infoquery boithobb-setuid boithobb-sqlUpdate==1.6.2 bbAutoUpdate perl-SQL-Abstract perl-Readonly perl-Params-Validate perl-DateTime" --post "chown apache:apache /home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd" $(boitho-bbadmin_name) $(boitho-bbadmin_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/webadmin Modules config/setup.txt

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
	perl rpmmagic.pl --post "install -d -o boitho /coredumps/saved; chown -R boitho /coredumps" --requires "boithobb-filepermissions gdb" --initd "everrun" $(boitho-everrun_name) $(boitho-everrun_version) $(BOITHOHOME) /home/boitho/boithoTools bin/everrun

	rpmbuild -bb $(boitho-everrun_name).spec

infoquery:
	perl rpmmagic.pl $(boitho-infoquery_name) $(boitho-infoquery_version) $(BOITHOHOME) /home/boitho/boithoTools bin/infoquery

	rpmbuild -bb $(boitho-infoquery_name).spec

configfiles:
	perl rpmmagic.pl --restartsw $(boitho-configfiles_name) $(boitho-configfiles_version) $(BOITHOHOME)/blackbox/boithoTools /home/boitho/boithoTools config/main.conf config/maplist.conf config/lotlist.conf config/setup.txt

	rpmbuild -bb $(boitho-configfiles_name).spec

searchdbb:
	perl rpmmagic.pl --nostrip --post "sh /home/boitho/boithoTools/blackbox/limitsTuning.sh" --requires "boithobb-sqlUpdate>=1.7.10 boitho-suggest perl-Text-Iconv boitho-dictionary boitho-configfiles boitho-everrun boithobb-filepermissions" --initd searchdbb $(boitho-searchdbb_name) $(boitho-searchdbb_version) $(BOITHOHOME) /home/boitho/boithoTools config/searchd.conf cgi-bin/dispatcher_allbb bin/searchdbb cgi-bin/ShowThumbbb cgi-bin/ShowCache2bb config/filetypes.eng.conf config/filetypes.nbo.conf config/dispatcher.conf  data/thesaurus.id data/thesaurus.text config/file_extensions.conf config/show_attributes.conf config/attribute_descriptions.conf blackbox/limitsTuning.sh

	rpmbuild -bb $(boitho-searchdbb_name).spec


crawlManager:
	perl rpmmagic.pl --requires "boitho-everrun boitho-bbdn boithobb-filepermissions /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/auto/XML/Parser/Expat/Expat.so /usr/lib/mysql/libmysqlclient.so.15" --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager crawlers/crawlSMB crawlers/crawlExchange config/crawlmanager.conf 

	rpmbuild -bb $(boitho-crawlManager_name).spec

crawlManager2:
	perl rpmmagic.pl --nostrip --requires "/usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/XML/Parser.pm perl-SOAP-Lite perl-XML-XPath boitho-bbdocumentwebadd boitho-infoquery>=2.0.19-1 boitho-bbdn>=1.8.16-1 boithobb-sqlUpdate>=1.7.10 /usr/lib/perl5/vendor_perl/5.8.8/i386-linux-thread-multi/auto/HTML/Parser/Parser.so boitho-perlxs-sdcrawl boitho-everrun boitho-bbdn boithobb-filepermissions /usr/lib/mysql/libmysqlclient.so.15 perl-Devel-Symdump /usr/lib/perl5/site_perl/5.8.8/i386-linux-thread-multi/auto/XML/LibXML/LibXML.so /usr/lib/perl5/site_perl/5.8.8/i386-linux-thread-multi/auto/XML/LibXML/Common/Common.so" --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager2 crawlers/crawlSMB crawlers/crawlExchange crawlers/Modules crawlers/Intranet crawlers/crawlSO crawlers/crawlExchangePublic crawlers/Sharepoint usersystems/us_ad/us_ad.so usersystems/us_sqlbb/us_sqlbb.so config/crawlmanager.conf perl/persistent.pl

	rpmbuild -bb $(boitho-crawlManager_name).spec

scripts:
	perl rpmmagic.pl $(boitho-scripts_name) $(boitho-scripts_version) $(BOITHOHOME) /home/boitho/boithoTools script/eachNight.sh script/reloadDictionary.sh script/makebbwordlist.sh perl/createaspellwordlist.pl
	rpmbuild -bb $(boitho-scripts_name).spec

dictionary:
	perl rpmmagic.pl --requires "boitho-scripts" $(boitho-dictionary_name) $(boitho-dictionary_version) $(BOITHOHOME) /home/boitho/boithoTools bin/dictionarywordsLot data/dict/ data/dict/bb.dat data/dict/bb.multi data/dict/iso-8859-1.cset data/dict/iso-8859-1.cmap
	rpmbuild -bb $(boitho-dictionary_name).spec

list_collections:
	perl rpmmagic.pl $(boitho-list_collections_name) $(boitho-list_collections_version) $(BOITHOHOME) /home/boitho/boithoTools bin/list_collections
	rpmbuild -bb $(boitho-list_collections_name).spec


boitho-bbdn:
	perl rpmmagic.pl --nostrip --requires "boitho-everrun boithobb-filepermissions /usr/lib/libextractor.so.1" --initd boitho-bbdn $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boitho-bbdn

	rpmbuild -bb $(boitho-boitho-bbdn_name).spec

fileFilter:
	perl rpmmagic.pl --nostrip --initdrestart "boitho-bbdn" --requires "perl-Email-MIME perl-TimeDate" $(boitho-fileFilter_name) $(boitho-fileFilter_version) $(BOITHOHOME) /home/boitho/boithoTools fileFilter

	rpmbuild -bb $(boitho-fileFilter_name).spec

setup_login:
	make $(BOITHOHOME)/vmware/run_setup
	perl rpmmagic.pl --post "sh /home/setup/post_install.sh" $(boitho-setup_login_name) $(boitho-setup_login_version) $(BOITHOHOME)/vmware / home/setup/run_setup home/root/post_install.sh sbin/ifup-local
	rpmbuild -bb $(boitho-setup_login_name).spec

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


clean:
	#rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
	#sletter buggde pakker
	rm -f $(HOME)/redhat/RPMS/i386/boitho*.rpm

install:
	cp --verbose /home/boitho/redhat/RPMS/i386/boitho*.rpm ~/FedoraRepository/boitho/updates/4/i386/


