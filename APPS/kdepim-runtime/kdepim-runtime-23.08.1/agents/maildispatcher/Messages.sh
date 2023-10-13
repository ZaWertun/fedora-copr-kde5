#! /bin/sh
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/akonadi_maildispatcher_agent.pot
rm -f rc.cpp
