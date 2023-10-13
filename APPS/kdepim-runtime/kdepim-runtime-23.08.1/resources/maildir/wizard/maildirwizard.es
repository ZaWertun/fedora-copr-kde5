/*
    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var page = Dialog.addPage( "maildirwizard.ui", qsTr("Personal Settings") );

function validateInput()
{
  if ( page.widget().maildirPath.text == "" ) {
    page.setValid( false );
  } else {
    page.setValid( true );
  }
}

function setup()
{
  var maildirRes = SetupManager.createResource( "akonadi_maildir_resource" );
  maildirRes.setOption( "Path", page.widget().maildirPath.text );

  SetupManager.execute();
}

page.widget().maildirPath.textChanged.connect( validateInput );
page.pageLeftNext.connect( setup );
validateInput();
