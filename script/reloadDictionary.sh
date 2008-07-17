#!/bin/sh

kill -USR1 `cat $BOITHOHOME/var/searchd.pid`
