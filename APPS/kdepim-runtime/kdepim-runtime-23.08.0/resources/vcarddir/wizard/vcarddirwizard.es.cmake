/*
    SPDX-FileCopyrightText: 2010 Till Adam <adam@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var page = Dialog.addPage( "vcarddirwizard.ui", qsTr("Settings") );

page.widget().lineEdit.text = "${VCARDDIR_FILE_DEFAULT_PATH}";

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
  var vcardRes = SetupManager.createResource( "akonadi_vcarddir_resource" );
  vcardRes.setOption( "Path", page.widget().lineEdit.text );
  vcardRes.setName( qsTr("Default Contact") );
  SetupManager.execute();
}

page.widget().lineEdit.textChanged.connect( validateInput );
page.pageLeftNext.connect( setup );
validateInput();
