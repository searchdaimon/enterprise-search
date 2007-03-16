
boitho-ad_name = boitho-ad
boitho-ad_version = 0.2.2

boitho-startscripts_name = boitho-startscripts
boitho-startscripts_version = 0.2.2

boitho-searchdbb_name = boitho-searchdbb
boitho-searchdbb_version = 5.4.9

boitho-crawlManager_name = boitho-crawlManager
boitho-crawlManager_version = 1.3.2

boitho-dispatcher_allbb_name = boitho-dispatcher_allbb
boitho-dispatcher_allbb_version = 8.7.6

boitho-boitho-bbdn_name = boitho-boitho-bbdn
boitho-boitho-bbdn_version = 1.7.0

boitho-base_name = boitho-base
boitho-base_version = 1.3.0

all: boithoad startscripts searchdbb crawlManager dispatcher_allbb boitho-bbdn ShowThumb base

boithoad:
	perl rpmmagic.pl $(boitho-ad_name) $(boitho-ad_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/boithoad

	rpm -bb $(boitho-ad_name).spec


startscripts:
	perl rpmmagic.pl $(boitho-startscripts_name) $(boitho-startscripts_version) /home/boitho/boitho/websearch /home/boitho/boithoTools script/start.sh script/stop.sh

	rpm -bb $(boitho-startscripts_name).spec


searchdbb:
	#perl rpmmagic.pl $(boitho-searchdbb_name) $(boitho-searchdbb_version) /home/boitho/boithoTools/bin ../bin/searchdbb
	perl rpmmagic.pl $(boitho-searchdbb_name) $(boitho-searchdbb_version) /home/boitho/boitho/websearch /home/boitho/boithoTools config/searchd.cfg bin/searchdbb

	rpm -bb $(boitho-searchdbb_name).spec


crawlManager:
	perl rpmmagic.pl $(boitho-crawlManager_name) $(boitho-crawlManager_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/crawlManager

	rpm -bb $(boitho-crawlManager_name).spec


dispatcher_allbb:
	perl rpmmagic.pl $(boitho-dispatcher_allbb_name) $(boitho-dispatcher_allbb_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/dispatcher_allbb

	rpm -bb $(boitho-dispatcher_allbb_name).spec


boitho-bbdn:
	perl rpmmagic.pl $(boitho-boitho-bbdn_name) $(boitho-boitho-bbdn_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/boitho-bbdn

	rpm -bb $(boitho-boitho-bbdn_name).spec


ShowThumb:
	perl rpmmagic.pl $(boitho-ShowThumb_name) $(boitho-ShowThumb_version) /home/boitho/boitho/websearch /home/boitho/boithoTools bin/ShowThumb

	rpm -bb $(boitho-ShowThumb_name).spec

base:
	rm -rf $(boitho-base_name)-$(boitho-base_version)
	mkdir $(boitho-base_name)-$(boitho-base_version)

	cp ../blackbox/bbdemo.boitho.com.conf $(boitho-base_name)-$(boitho-base_version)

	tar -z -c -f $(boitho-base_name)-$(boitho-base_version).tar.gz $(boitho-base_name)-$(boitho-base_version)
	mv $(boitho-base_name)-$(boitho-base_version).tar.gz /home/boitho/redhat/SOURCES/	

	rpm -bb $(boitho-base_name).spec

clean:
	rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
