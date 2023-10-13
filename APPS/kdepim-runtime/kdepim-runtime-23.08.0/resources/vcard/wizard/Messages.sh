#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/accountwizard_vcard.pot
$XGETTEXT -kqsTr *.es.cmake -j -o $podir/accountwizard_vcard.pot
