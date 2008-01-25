
boitho-ad_name = boitho-ad
boitho-ad_version = 0.3.1

boitho-searchdbb_name = boitho-searchdbb
boitho-searchdbb_version = 5.5.2

boitho-crawlManager_name = boitho-crawlManager
boitho-crawlManager_version = 1.4.4

boitho-boitho-bbdn_name = boitho-bbdn
boitho-boitho-bbdn_version = 1.8.0

boitho-fileFilter_name = boitho-fileFilter
boitho-fileFilter_version = 1.3.1

boitho-base_name = boitho-base
boitho-base_version = 1.3.0

boitho-bbadmin_name = boitho-bbadmin
boitho-bbadmin_version = 2.8.4

boitho-everrun_name = boitho-everrun
boitho-everrun_version = 2.0.0

boitho-infoquery_name = boitho-infoquery
boitho-infoquery_version = 2.0.8

boitho-iindex_name = boitho-iindex
boitho-iindex_version = 2.0.9

boitho-webclient_name = boitho-webclient
boitho-webclient_version = 6.0.18

boithobb-setuid_name = boithobb-setuid
boithobb-setuid_version = 1.0.8

boithobb-bbadminhtaccess_name = boithobb-bbadminhtaccess
boithobb-bbadminhtaccess_version = 1.0.0

boithobb-crawl_watch_name = boithobb-crawl_watch
boithobb-crawl_watch_version = 1.0.3

boithobb-bbAutoUpdate_name = bbAutoUpdate
boithobb-bbAutoUpdate_version = 1.0.1

boithobb-call-home_name = bb-call-home
boithobb-call-home_version = 0.2

boithobb-database_name = boitho-database
boithobb-database_version = 0.1

boitho-meta_name = boitho-meta
boitho-meta_version = 0.1


all: webclient iindex bbadmin boithoad everrun infoquery searchdbb crawlManager boitho-bbdn base setuidbb bbAutoUpdate callHome sqldatabase mod_boitho_auth meta

setuidbb:
	perl rpmmagic.pl --defattr "%defattr(4755, root, -) " --post "chown root \$${DESTDIR}/setuid/initwrapper; chmod +s \$${DESTDIR}/setuid/initwrapper; chown root \$${DESTDIR}/setuid/yumwrapper; chmod +s \$${DESTDIR}/setuid/yumwrapper;chown root \$${DESTDIR}/setuid/configwrite; chmod +s \$${DESTDIR}/setuid/configwrite;" $(boithobb-setuid_name) $(boithobb-setuid_version) $(BOITHOHOME) /home/boitho/boithoTools setuid/initwrapper setuid/configwrite setuid/yumwrapper

	rpm -bb $(boithobb-setuid_name).spec

webclient:

	perl rpmmagic.pl $(boitho-webclient_name) $(boitho-webclient_version) $(BOITHOHOME) /home/boitho/boithoTools public_html/webclient

	rpm -bb $(boitho-webclient_name).spec

iindex:
	perl rpmmagic.pl $(boitho-iindex_name) $(boitho-iindex_version) $(BOITHOHOME) /home/boitho/boithoTools bin/IndexerLotbb bin/LotInvertetIndexMaker2 bin/mergeUserToSubname bin/mergeIIndex bin/readIIndex perl/cleanLots.pl

	rpm -bb $(boitho-iindex_name).spec

bbadminhtaccess:
	perl rpmmagic.pl $(boithobb-bbadminhtaccess_name) $(boithobb-bbadminhtaccess_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/webadmin/.htaccess cgi-bin/webadmin/.htpasswd

	rpm -bb $(boithobb-bbadminhtaccess_name).spec

bbadmin:
	echo BOITHOHOME
	echo $(BOITHOHOME)
	perl rpmmagic.pl --requires "boithobb-setuid" $(boitho-bbadmin_name) $(boitho-bbadmin_version) $(BOITHOHOME) /home/boitho/boithoTools cgi-bin/webadmin Modules

	rpm -bb $(boitho-bbadmin_name).spec

boithoad:
	perl rpmmagic.pl --initd boithoad $(boitho-ad_name) $(boitho-ad_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boithoad

	rpm -bb $(boitho-ad_name).spec

crawl_watch:
	perl rpmmagic.pl --initd crawl_watch $(boithobb-crawl_watch_name) $(boithobb-crawl_watch_version) $(BOITHOHOME) /home/boitho/boithoTools perl/crawl_watch.pl

	rpm -bb $(boithobb-crawl_watch_name).spec

bbAutoUpdate:
	perl rpmmagic.pl --initd bbAutoUpdate $(boithobb-bbAutoUpdate_name) $(boithobb-bbAutoUpdate_version) $(BOITHOHOME) /home/boitho/boithoTools perl/bbAutoUpdate.pl

	rpm -bb $(boithobb-bbAutoUpdate_name).spec

everrun:
	perl rpmmagic.pl --post "install -d -o boitho /coredumps/saved" --initd everrun $(boitho-everrun_name) $(boitho-everrun_version) $(BOITHOHOME) /home/boitho/boithoTools bin/everrun

	rpm -bb $(boitho-everrun_name).spec

infoquery:
	perl rpmmagic.pl $(boitho-infoquery_name) $(boitho-infoquery_version) $(BOITHOHOME) /home/boitho/boithoTools bin/infoquery

	rpm -bb $(boitho-infoquery_name).spec

searchdbb:
	perl rpmmagic.pl --initd searchdbb $(boitho-searchdbb_name) $(boitho-searchdbb_version) $(BOITHOHOME) /home/boitho/boithoTools config/searchd.conf cgi-bin/dispatcher_allbb bin/searchdbb cgi-bin/ShowThumbbb cgi-bin/ShowCache config/filetypes.eng.conf config/filetypes.nbo.conf config/dispatcher.conf

	rpm -bb $(boitho-searchdbb_name).spec


crawlManager:
	perl rpmmagic.pl --requires boithobb-crawl_watch --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) $(BOITHOHOME) /home/boitho/boithoTools bin/crawlManager crawlers config/crawlmanager.conf

	rpm -bb $(boitho-crawlManager_name).spec


boitho-bbdn:
	perl rpmmagic.pl --initd boitho-bbdn $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) $(BOITHOHOME) /home/boitho/boithoTools bin/boitho-bbdn

	rpm -bb $(boitho-boitho-bbdn_name).spec

fileFilter:
	perl rpmmagic.pl $(boitho-fileFilter_name) $(boitho-fileFilter_version) $(BOITHOHOME) /home/boitho/boithoTools fileFilter

	rpm -bb $(boitho-fileFilter_name).spec


callHome:
	perl rpmmagic.pl $(boithobb-call-home_name) $(boithobb-call-home_version) $(BOITHOHOME) /home/boitho/boithoTools \
	config/bb-phone-home-client.conf

	rpm -bb $(boithobb-call-home_name).spec
 

sqldatabase:
	perl rpmmagic.pl -initd boithodbsetup --initdnostart --post "chkconfig mysqld on" $(boithobb-database_name) $(boithobb-database_version) $(BOITHOHOME) /home/boitho/boithoTools \
		blackbox/boithobb.sql blackbox/creatuser.sql blackbox/boithodbsetup
	rpm -bb $(boithobb-database_name).spec


base:
	rm -rf $(boitho-base_name)-$(boitho-base_version)
	mkdir $(boitho-base_name)-$(boitho-base_version)

	cp ../blackbox/bbdemo.boitho.com.conf $(boitho-base_name)-$(boitho-base_version)

	tar -z -c -f $(boitho-base_name)-$(boitho-base_version).tar.gz $(boitho-base_name)-$(boitho-base_version)
	mv $(boitho-base_name)-$(boitho-base_version).tar.gz $(HOME)/redhat/SOURCES/	

	rpm -bb $(boitho-base_name).spec

meta:
	rm -rf $(boitho-meta_name)-$(boitho-meta_version)
	mkdir $(boitho-meta_name)-$(boitho-meta_version)
	echo boitho-meta > $(boitho-meta_name)-$(boitho-meta_version)/boitho-meta
	tar -z -c -f $(boitho-meta_name)-$(boitho-meta_version).tar.gz $(boitho-meta_name)-$(boitho-meta_version)
	mv $(boitho-meta_name)-$(boitho-meta_version).tar.gz $(HOME)/redhat/SOURCES/
	rpm -bb $(boitho-meta_name).spec

mod_boitho_auth:
	rpm -bb boitho_mod_auth.spec

clean:
	#rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
	#sletter buggde pakker
	rm -f $(HOME)/redhat/RPMS/i386/boitho*.rpm

install:
	cp --verbose /home/boitho/redhat/RPMS/i386/boitho*.rpm ~/FedoraRepository/boitho/updates/4/i386/
