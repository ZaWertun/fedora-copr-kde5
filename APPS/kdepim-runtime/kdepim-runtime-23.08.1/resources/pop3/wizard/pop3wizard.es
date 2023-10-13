/*
    SPDX-FileCopyrightText: 2009-2021 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// add this function to trim user input of whitespace when needed
String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g, ""); };

var page = Dialog.addPage( "pop3wizard.ui", qsTr("Personal Settings") );

var userChangedServerAddress = false;

function serverChanged( arg )
{
  validateInput();
  if ( arg == "" ) {
    userChangedServerAddress = false;
  } else {
    userChangedServerAddress = true;
  }
}

function validateInput()
{
  if ( page.widget().incommingAddress.text.trim() == "" ) {
    page.setValid( false );
  } else {
    page.setValid( true );
  }
}

var stage = 1;

function setup()
{
  if ( stage == 1 ) {
    ServerTest.test( page.widget().incommingAddress.text.trim(), "pop" );
  } else {
    ServerTest.test( page.widget().outgoingAddress.text.trim(), "smtp" );
  }
}

function testResultFail()
{
  testOk( -1 );
}

function testOk( arg )
{
  if (stage == 1) {
    SetupManager.openWallet();
    var pop3Res = SetupManager.createResource( "akonadi_pop3_resource" );
    var server = page.widget().incommingAddress.text.trim();
    pop3Res.setOption( "Host", server );
    pop3Res.setOption( "Login", page.widget().userName.text.trim() );
    pop3Res.setOption( "Password", SetupManager.password() );

    if ( arg == "ssl" ) {
      pop3Res.setOption( "Port", 995 );
      pop3Res.setOption( "UseTLS", true );
    } else if ( arg == "tls" ) { // tls is really STARTTLS
      pop3Res.setOption( "Port", 110 );
      pop3Res.setOption( "UseTLS", true );
    } else if ( arg == "none" ) {
      pop3Res.setOption( "Port", 110 );
    } else {
      pop3Res.setOption( "Port", 110 );
    }

    stage = 2;
    setup();
  } else {
    var smtp = SetupManager.createTransport( "smtp" );
    smtp.setName( page.widget().outgoingAddress.text.trim() );
    smtp.setHost( page.widget().outgoingAddress.text.trim() );
    if ( arg == "ssl" ) {
      smtp.setEncryption( "SSL" );
    } else if ( arg == "tls" ) {
      smtp.setEncryption( "TLS" );
    } else {
      smtp.setEncryption( "None" );
    }
    smtp.setUsername( page.widget().userName.text );
    smtp.setPassword( SetupManager.password() );
    SetupManager.execute();
  }
}

try {
  ServerTest.testFail.connect( testResultFail );
  ServerTest.testResult.connect( testOk );
  page.widget().incommingAddress.textChanged.connect( validateInput );
  page.pageLeftNext.connect( setup );
} catch ( e ) {
  print( e );
}

validateInput();
