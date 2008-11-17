#!/bin/bash

echo "Searchdaimon ES running on Fedora release 8 (Werewolf)" > /etc/issue
echo "Kernel \r on an \m" >> /etc/issue
echo -n "IP-" >> /etc/issue
ifconfig eth1|sed -ne "2p"|awk '{print $2}' >> /etc/issue
echo >> /etc/issue
