#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/akonadi_ews_resource.pot
