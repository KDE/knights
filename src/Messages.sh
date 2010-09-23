#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT proto/*.cpp *.cpp -o $podir/knights.pot
rm -f rc.cpp
