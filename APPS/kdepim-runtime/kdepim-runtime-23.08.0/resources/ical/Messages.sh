#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name \*.kcfg -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/akonadi_ical_resource.pot
rm -f rc.cpp
