Resource.setType( "akonadi_ical_resource" );
Resource.setPathOption( "Path", "newical.ics" );
Resource.create();

XmlOperations.setXmlFile( "ical-empty.xml" );
XmlOperations.setRootCollections( Resource.identifier() );
XmlOperations.setCollectionKey( "None" );
XmlOperations.ignoreCollectionField( "Name" ); // name is the resource identifier and thus unpredictable
XmlOperations.setNormalizeRemoteIds( true );
XmlOperations.assertEqual();

// item creation
var i1 = ItemTest.newInstance();
i1.setParentCollection( Resource.identifier() );
i1.setMimeType( "text/calendar" );
i1.setPayloadFromFile( "event.ical" );
i1.create();

var i2 = ItemTest.newInstance();
i2.setParentCollection( Resource.identifier() );
i2.setMimeType( "text/calendar" );
i2.setPayloadFromFile( "task.ical" );
i2.create();

Resource.recreate();

XmlOperations.setXmlFile( "ical-step1.xml" );
XmlOperations.setRootCollections( Resource.identifier() );
XmlOperations.ignoreCollectionField( "None" );
XmlOperations.assertEqual();

