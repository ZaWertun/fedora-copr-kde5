#!/bin/sh
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/akonadi_openxchange_resource.pot
