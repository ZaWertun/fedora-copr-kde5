#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/akonadi_birthdays_resource.pot
