chown boitho:boitho /home/boitho/boithoTools/
chown boitho:boitho /home/boitho/boithoTools/bin
chown boitho:boitho /home/boitho/boithoTools/var
chown boitho:boitho /home/boitho/boithoTools/var/*
chown boitho:boitho /home/boitho/boithoTools/logs
chown boitho:boitho /home/boitho/boithoTools/logs/*
if [ ! -f /home/boitho/boithoTools/logs/bbdocumentWebAdd.log ]; then
	touch /home/boitho/boithoTools/logs/bbdocumentWebAdd.log
fi
chown apache /home/boitho/boithoTools/logs/bbdocumentWebAdd.log

chown -R boitho:boitho /boithoData/lot/

chown boitho:boitho /home/boitho/boithoTools/data/dict/
chown apache:apache /home/boitho/boithoTools/cgi-bin/webadmin/.htpasswd
chown apache:apache /home/boitho/boithoTools/crawlers
