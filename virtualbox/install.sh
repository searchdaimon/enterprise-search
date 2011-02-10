#if service exist, stop 
if [ -f /etc/init.d/vboxadd ] ; then
        sh /etc/init.d/vboxadd stop
	#ikke testet !!!
	/home/boitho/boithoTools/virtualbox/VBoxLinuxAdditions-amd64.run uninstall
fi


#Pakker ut filer
/home/boitho/boithoTools/virtualbox/VBoxLinuxAdditions-amd64.run no_setup

#bygger kjernemoduler
/usr/lib64/VBoxGuestAdditions/vboxadd setup


#fix bug in /etc/init.d/vboxadd where we have "# chkconfig: 357 30 70", and 357 meens runlevel 3,5 and 7. Linux only have 6 runlecels.
sed -i -e 's/^\(#\s*chkconfig:\s*[0-6]*\)7/\1/' /etc/init.d/vboxadd

#run chkconfig to add it to rc
chkconfig --add vboxadd

