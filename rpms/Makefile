
boitho-ad_name = boitho-ad
boitho-ad_version = 0.2.3

boitho-searchdbb_name = boitho-searchdbb
boitho-searchdbb_version = 5.4.10

boitho-crawlManager_name = boitho-crawlManager
boitho-crawlManager_version = 1.3.2

boitho-boitho-bbdn_name = boitho-bbdn
boitho-boitho-bbdn_version = 1.7.0

boitho-base_name = boitho-base
boitho-base_version = 1.3.0

boitho-bbadmin_name = boitho-bbadmin
boitho-bbadmin_version = 2.7.9

boitho-everrun_name = boitho-everrun
boitho-everrun_version = 1.0.0

boitho-infoquery_name = boitho-infoquery
boitho-infoquery_version = 2.0.1

boitho-iindex_name = boitho-iindex
boitho-iindex_version = 2.0.1

boitho-webclient_name = boitho-webclient
boitho-webclient_version = 6.0.1


all: webclient iindex bbadmin boithoad everrun infoquery searchdbb crawlManager boitho-bbdn base


webclient:

	perl rpmmagic.pl $(boitho-webclient_name) $(boitho-webclient_version) /home/boitho/boitho/websearch /home/boitho/boithoTools public_html/webclient

	rpm -bb $(boitho-webclient_name).spec

iindex:
	perl rpmmagic.pl $(boitho-iindex_name) $(boitho-iindex_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/IndexerLotbb bin/LotInvertetIndexMaker2 bin/mergeUserToSubname bin/mergeIIndex perl/cleanLots.pl

	rpm -bb $(boitho-iindex_name).spec

bbadmin:
	perl rpmmagic.pl $(boitho-bbadmin_name) $(boitho-bbadmin_version) /home/boitho/boitho/websearch /home/boitho/boithoTools cgi-bin/webadmin Modules

	rpm -bb $(boitho-bbadmin_name).spec

boithoad:
	perl rpmmagic.pl --initd boithoad $(boitho-ad_name) $(boitho-ad_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/boithoad

	rpm -bb $(boitho-ad_name).spec


everrun:
	perl rpmmagic.pl $(boitho-everrun_name) $(boitho-everrun_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/everrun

	rpm -bb $(boitho-everrun_name).spec

infoquery:
	perl rpmmagic.pl $(boitho-infoquery_name) $(boitho-infoquery_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/infoquery

	rpm -bb $(boitho-infoquery_name).spec

searchdbb:
	perl rpmmagic.pl --initd searchdbb $(boitho-searchdbb_name) $(boitho-searchdbb_version) /home/boitho/boitho/websearch /home/boitho/boithoTools config/searchd.conf cgi-bin/dispatcher_allbb bin/searchdbb cgi-bin/ShowThumb cgi-bin/ShowCache

	rpm -bb $(boitho-searchdbb_name).spec


crawlManager:
	perl rpmmagic.pl --initd crawlManager $(boitho-crawlManager_name) $(boitho-crawlManager_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/crawlManager crawlers

	rpm -bb $(boitho-crawlManager_name).spec


boitho-bbdn:
	perl rpmmagic.pl --initd boitho-bbdn $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/boitho-bbdn fileFilter

	rpm -bb $(boitho-boitho-bbdn_name).spec


base:
	rm -rf $(boitho-base_name)-$(boitho-base_version)
	mkdir $(boitho-base_name)-$(boitho-base_version)

	cp ../blackbox/bbdemo.boitho.com.conf $(boitho-base_name)-$(boitho-base_version)

	tar -z -c -f $(boitho-base_name)-$(boitho-base_version).tar.gz $(boitho-base_name)-$(boitho-base_version)
	mv $(boitho-base_name)-$(boitho-base_version).tar.gz /home/boitho/redhat/SOURCES/	

	rpm -bb $(boitho-base_name).spec

clean:
	#rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
	#sletter buggde pakker
	rm -f /home/boitho/redhat/RPMS/i386/boitho-*.rpm

install:
	cp /home/boitho/redhat/RPMS/i386/boitho-*.rpm ~/FedoraRepository/boitho/updates/4/i386/
