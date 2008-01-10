#!/bin/sh

sh $BOITHOHOME/script/makebbwordlist.sh
kill -USR1 `cat $BOITHOHOME/var/searchd.pid`
