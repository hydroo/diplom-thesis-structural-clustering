#! /usr/bin/env bash

for a in $(ls -d */)
do

	cd $a

	if ls *.pro 1> /dev/null 2>&1; then
		echo "$a : qmake"
		if ! qmake; then
			echo "  Error. Aborting."
			cd ..
			break
		fi
	fi

	if [ -e Makefile ]; then
		echo "$a : make"
		if ! make -j4; then
			echo "  Error. Aborting"
			cd ..
			break
		fi
	fi

	cd ..
done
