

#if 0

// From giopServer.cc -- this is how the BOA activates objects
// which have 

    if( !servant && pd_response_expected && MapKeyToObjectFunction ) {
      // Cannot find the object in the object table. If the application
      // has registered a loader, do an upcall to locate the object.
      // If the returned value is not a nil object reference, send a
      // LOCATION_FORWARD message to the client to instruct it to retry
      // with the new object reference.
      //
      // Limitation: if this invocation is oneway, one cannot reply with
      //             a LOCATION_FORWARD message, in that case, just
      //             treat this as a normal OBJECT_NOT_EXIST and let
      //             the code below to deal with it.
      //
      CORBA::Object_var newDestination(MapKeyToObjectFunction(pd_objkey));
      if (!CORBA::is_nil(newDestination)) {
	// Note that we have completed the object request
	RequestReceived(1); 

	// Build and send the location forward message...
	size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
	msgsize = CORBA::Object::NP_alignedSize(newDestination, msgsize);
	InitialiseReply(GIOP::LOCATION_FORWARD,(CORBA::ULong)msgsize);
	CORBA::Object::marshalObjRef(newDestination, *this);

	// All done...
	ReplyCompleted();
	return;
      }
    }

#endif
