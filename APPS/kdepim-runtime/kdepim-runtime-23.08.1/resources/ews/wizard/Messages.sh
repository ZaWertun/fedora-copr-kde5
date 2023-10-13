#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/accountwizard_ews.pot
touch $podir/accountwizard_ews.pot
$XGETTEXT -kqsTr *.es -j -o $podir/accountwizard_ews.pot
