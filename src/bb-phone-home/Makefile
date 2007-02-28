

setuidcaller: bb-phone-home-wrapper.c
	gcc -o setuidcaller -O2 -W -Wall bb-phone-home-wrapper.c -DBBCLIENT=\"/home/eirik/Boitho/boitho/websearch/src/bb-phone-home/bb-client.pl\" -DEXEC_USER=\"phonehome\" -DWEB_USER=\"apache\"
	chown root setuidcaller
	chmod +s setuidcaller


clean:
	rm -f setuidcaller
