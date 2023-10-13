/*
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var page = Dialog.addPage( "contactswizard.ui", qsTr("Settings") );

page.widget().lineEdit.text = "${CONTACTS_DIRECTORY_DEFAULT_PATH}";

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
  var contactsResource = SetupManager.createResource( "akonadi_contacts_resource" );
  contactsResource.setOption( "Path", page.widget().lineEdit.text );
  contactsResource.setOption( "IsConfigured", "true" );
  contactsResource.setName( qsTr("Local Contacts") );
  SetupManager.execute();
}

page.widget().lineEdit.textChanged.connect( validateInput );
page.pageLeftNext.connect( setup );
validateInput();
