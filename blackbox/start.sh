echo starting from $BOITHOHOME

echo "starting boithoad"
/usr/local/sbin/daemonize $BOITHOHOME/bin/everrun $BOITHOHOME/bin/boithoad

echo "starting crawlManager"
/usr/local/sbin/daemonize $BOITHOHOME/bin/everrun $BOITHOHOME/bin/crawlManager

echo "starting boitho-bbdn"
/usr/local/sbin/daemonize $BOITHOHOME/bin/everrun $BOITHOHOME/bin/boitho-bbdn

echo "starting searchdbb"
/usr/local/sbin/daemonize $BOITHOHOME/bin/everrun $BOITHOHOME/bin/searchdbb localhost

