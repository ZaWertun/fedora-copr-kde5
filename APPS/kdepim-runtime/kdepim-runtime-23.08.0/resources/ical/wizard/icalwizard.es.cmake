/*
    SPDX-FileCopyrightText: 2010 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var page = Dialog.addPage( "icalwizard.ui", qsTr("Settings") );

page.widget().lineEdit.text = "${ICAL_FILE_DEFAULT_PATH}";

function validateInput()
{
  if ( page.widget().lineEdit.text == "" ) {
    page.setValid( false );
  } else {
    page.setValid( true );
  }
}

function setup()
{
  var icalRes = SetupManager.createResource( "akonadi_ical_resource" );
  icalRes.setOption( "Path", page.widget().lineEdit.text );
  icalRes.setName( qsTr("Default Calendar") );
  SetupManager.execute();
}

page.widget().lineEdit.textChanged.connect( validateInput );
page.pageLeftNext.connect( setup );
validateInput();
