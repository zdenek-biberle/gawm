#!/bin/bash
# -*- coding: utf-8 -*-

runner=$(bash ./get_pid.sh 'gawm-dbg')
kdbg -p ${runner} ./gawm-dbg

# konec souboru debug-kdbg.sh
