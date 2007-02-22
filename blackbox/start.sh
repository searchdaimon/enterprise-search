echo "starting boithoad"
/usr/local/sbin/daemonize /home/boitho/boithoTools/bin/everrun /home/boitho/boithoTools/bin/boithoad

echo "starting crawlManager"
/usr/local/sbin/daemonize /home/boitho/boithoTools/bin/everrun /home/boitho/boithoTools/bin/crawlManager

echo "starting boitho-bbdn"
/usr/local/sbin/daemonize /home/boitho/boithoTools/bin/everrun /home/boitho/boithoTools/bin/boitho-bbdn

echo "starting searchdbb"
/usr/local/sbin/daemonize /home/boitho/boithoTools/bin/everrun /home/boitho/boithoTools/bin/searchdbb localhost

