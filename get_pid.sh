#!/bin/bash
# -*- coding: utf-8 -*-

process=${1}

runner=""
while [ "$runner" == "" ]; do
	sleep 0.5
	runner="$(ps -e -o pid= -o ppid= -o cmd= | grep -E "([0-9]+\s+){2}\S*${process}$" | sed 's/^\s*//' | grep -v -e "^$$" -e "[0-9]\+\s\+$$" | cut -d' ' -f1)"
done

echo ${runner}

# konec souboru debug-kdbg.sh
