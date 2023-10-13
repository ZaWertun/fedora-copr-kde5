#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp *.h -o $podir/akonadi_singlefile_resource.pot
rm -f rc.cpp
