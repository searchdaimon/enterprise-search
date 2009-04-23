#!/bin/bash

LIBS="src/base64 src/newspelling src/getdate src/common src/3pLibs/keyValueHash src/perlembed"

case $1 in 
	deps)
		for i in $LIBS; do (echo $i; cd $i; make clean; make); done
		;;
	search)
		(cd src/searchkernel; make clean; make)
		;;
	*)
		echo "I don't understand" >&2
		exit 1
		;;
esac
