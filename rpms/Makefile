
boitho-ad_name = boitho-ad
boitho-ad_version = 0.2.2

all: boithoad

boithoad:
	perl rpmmagic.pl $(boitho-ad_name) $(boitho-ad_version) /home/boitho/bin ../bin/boithoad

	rpm -bb $(boitho-ad_name).spec


clean:
	rm /home/boitho/redhat/SOURCES/boitho-ad-0.2.2.tar.gz $(boitho-ad_name).spec
