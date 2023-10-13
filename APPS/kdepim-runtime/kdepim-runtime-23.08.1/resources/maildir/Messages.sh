#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC *.ui *.kcfg >> rc.cpp
$XGETTEXT libmaildir/*.cpp *.cpp -o $podir/akonadi_maildir_resource.pot
