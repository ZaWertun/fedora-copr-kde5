/*
    SPDX-FileCopyrightText: 2010 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var page = Dialog.addPage( "mailboxwizard.ui", qsTr("Personal Settings") );

function validateInput()
{
  if ( page.widget().mailboxPath.text == "" ) {
    page.setValid( false );
  } else {
    page.setValid( true );
  }
}

function setup()
{
  var mboxRes = SetupManager.createResource( "akonadi_mbox_resource" );
  mboxRes.setOption( "Path", page.widget().mailboxPath.text );

  SetupManager.execute();
}

page.widget().mailboxPath.textChanged.connect( validateInput );
page.pageLeftNext.connect( setup );
validateInput();
