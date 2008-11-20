#!/bin/bash

rm -rf /etc/udev/rules.d/70-persistent-net.rules
rm -rf /etc/udev/rules.d/75-persistent-net-generator.rules
adduser --shell /home/setup/run_setup setup
echo -e "/^setup/,s/^setup:x/setup:/\nw\nq" | ed -s /etc/passwd
chown root.setup /home/setup/run_setup
chmod 750 /home/setup/run_setup
chmod +s /home/setup/run_setup
cp "/home/setup/make_issue.sh" /sbin/ifup-local
