#!/bin/bash
# -*- coding: utf-8 -*-
runner=""
while [ "$runner" == "" ]; do
	sleep 0.5
	runner="$(ps -e -o pid= -o ppid= -o cmd= | grep -E "([0-9]+\s+){2}\S*gawm-dbg" | sed 's/^\s*//' | grep -v -e "^$$" -e "[0-9]\+\s\+$$" | cut -d' ' -f1)"
done
kdbg -p ${runner} ./gawm-dbg

# konec souboru debug-kdbg.sh
