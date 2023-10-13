#! /bin/sh
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/akonadi_newmailnotifier_agent.pot
rm -f rc.cpp
