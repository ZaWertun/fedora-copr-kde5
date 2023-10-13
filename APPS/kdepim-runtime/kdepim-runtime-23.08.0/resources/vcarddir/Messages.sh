#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name \*.ui` `find . -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/akonadi_vcarddir_resource.pot
