#include <omniORB2/CORBA.h>
#include <giopObjectInfo.h>

// Strip the object reference of any Firewall profile components.
// No effect if the object reference is a nil or local object.
void
stripFireWallComponents(CORBA::Object_ptr obj)
{
  if (CORBA::is_nil(obj)) return;

  omniObject* op = obj->PR_getobj();

  if (!op->is_proxy()) return;

  CORBA::Boolean fwd = 0;
  GIOPObjectInfo_var objInfo = op->getInvokeInfo(fwd);

  // XXX We deliberately recast the return value to allow us to modify
  //     in place. Only do so because we are sure this obj is not
  //     used by any other thread.
  IOP::TaggedProfileList* orig_iop = (IOP::TaggedProfileList*) 
                                          objInfo->iopProfiles();

  for (CORBA::ULong index = 0; index < orig_iop->length(); index++) {

    if ((*orig_iop)[index].tag == IOP::TAG_INTERNET_IOP) {
      
      cdrEncapsulationStream orig_s((*orig_iop)[index].profile_data.get_buffer(),
				    (*orig_iop)[index].profile_data.length(),
				    1);
      cdrEncapsulationStream new_s((*orig_iop)[index].profile_data.length());

      IIOP::ProfileBody body;
      
      // Extract version, host, port and object_key from the original and
      // insert into the new encapsulation stream. Notice that the original
      // and the new encapsulation stream may be of different endian.
      body.version.major <<= orig_s; 
      body.version.major >>= new_s; 
      body.version.minor <<= orig_s;
      body.version.minor >>= new_s;
      body.host <<= orig_s;
      body.host >>= new_s;
      body.port <<= orig_s;
      body.port >>= new_s;
      body.object_key <<= orig_s;
      body.object_key >>= new_s;

      if (body.version.minor) {

	// Now go through the optional tagged components and store
	// only those that are not TAG_FIREWALL_TRANS.

	CORBA::ULong total, newtotal = 0;
	total <<= orig_s;

	IOP::MultipleComponentProfile newComp(total);

	for (CORBA::ULong i=0; i < total; i++) {
	  IOP::ComponentId v;

	  v <<= orig_s;
	  if (v != IOP::TAG_FIREWALL_TRANS) {
	    newtotal++;
	    newComp.length(newtotal);
	    newComp[newtotal-1].tag = v;
	    newComp[newtotal-1].component_data <<= orig_s;
	  }
	}

	// Insert the optional tagged components which have been stripped
	// of any TAG_FIREWALL_TRANS into the new encapsulation stream
	newComp >>= new_s;
      }

      // Now replace the original profile data with the new encapsulation
      // stream.
      CORBA::Octet* p;
      CORBA::ULong max;
      CORBA::ULong len;
      new_s.getOctetStream(p,max,len);
      (*orig_iop)[index].profile_data.replace(max,len,p,1);
    }
  }

  // Finally, set the proxy object to use the modified GIOPObjectInfo to
  // marshal the IOR onto the wire.
  op->setInvokeInfo(objInfo._retn(),0);
}
