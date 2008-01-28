

setuidcaller: bb-phone-home-wrapper.c
	gcc -o setuidcaller -O2 -W -Wall bb-phone-home-wrapper.c -DEXEC_USER=\"phonehome\" -DWEB_USER=\"apache\"
	#chown root setuidcaller
	#chmod +s setuidcaller

clean:
	rm -f setuidcaller
