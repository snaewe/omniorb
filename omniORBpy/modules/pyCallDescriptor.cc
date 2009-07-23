// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyCallDescriptor.cc        Created on: 2000/02/02
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
//
// Description:
//    Implementation of Python call descriptor object

// $Id$
// $Log$
// Revision 1.1.4.3  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.2  2003/07/10 22:17:02  dgrisby
// Track orb core changes, fix bugs.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.11  2003/01/27 11:56:57  dgrisby
// Correctly handle invalid returns from application code.
//
// Revision 1.1.2.10  2002/01/18 15:49:44  dpg1
// Context support. New system exception construction. Fix None call problem.
//
// Revision 1.1.2.9  2001/09/24 10:48:25  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.8  2001/09/20 14:51:24  dpg1
// Allow ORB reinitialisation after destroy(). Clean up use of omni namespace.
//
// Revision 1.1.2.7  2001/08/15 10:37:14  dpg1
// Track ORB core object table changes.
//
// Revision 1.1.2.6  2001/06/29 15:11:12  dpg1
// Fix for clients using GIOP 1.0.
//
// Revision 1.1.2.5  2001/06/29 09:53:56  dpg1
// Fix for clients using GIOP 1.0.
//
// Revision 1.1.2.4  2001/06/01 11:09:26  dpg1
// Make use of new omni::ptrStrCmp() and omni::strCmp().
//
// Revision 1.1.2.3  2001/05/10 15:16:02  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.1.2.2  2001/01/10 12:00:07  dpg1
// Release the Python interpreter lock when doing potentially blocking
// stream calls.
//
// Revision 1.1.2.1  2000/10/13 13:55:24  dpg1
// Initial support for omniORB 4.
//

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORB4/IOP_C.h>

#ifdef HAS_Cplusplus_Namespace
namespace {
#endif
  class cdLockHolder {
  public:
    inline cdLockHolder(omniPy::Py_omniCallDescriptor* cd) : cd_(cd) {
      cd->reacquireInterpreterLock();
    }
    inline ~cdLockHolder() {
      cd_->releaseInterpreterLock();
    }
  private:
    omniPy::Py_omniCallDescriptor* cd_;
  };
#ifdef HAS_Cplusplus_Namespace
};
#endif


OMNI_USING_NAMESPACE(omni)


omniPy::Py_omniCallDescriptor::~Py_omniCallDescriptor()
{
  OMNIORB_ASSERT(!tstate_);
  Py_XDECREF(args_);
  Py_XDECREF(result_);
}


void
omniPy::Py_omniCallDescriptor::initialiseCall(cdrStream&)
{
  // initialiseCall() is called with the interpreter lock
  // released. Reacquire it so we can touch the descriptor objects
  // safely
  cdLockHolder _l(this);

  for (int i=0; i < in_l_; i++)
    omniPy::validateType(PyTuple_GET_ITEM(in_d_,i),
			 PyTuple_GET_ITEM(args_,i),
			 CORBA::COMPLETED_NO);
}


void
omniPy::Py_omniCallDescriptor::marshalArguments(cdrStream& stream)
{
  int i;
  if (in_marshal_) {
    omniORB::logs(25, "Python marshalArguments re-entered.");

    // marshalArguments can be re-entered when using GIOP 1.0, to
    // calculate the message size if the message is too big for a
    // single buffer. In that case, the interpreter lock has been
    // released by the PyUnlockingCdrStream, meaning the call
    // descriptor does not have the lock details. We have to use the
    // thread cache lock.

    omnipyThreadCache::lock _t;

    for (i=0; i < in_l_; i++)
      omniPy::marshalPyObject(stream,
			      PyTuple_GET_ITEM(in_d_,i),
			      PyTuple_GET_ITEM(args_,i));
    if (ctxt_d_)
      omniPy::marshalContext(stream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
  }
  else {
    cdLockHolder _l(this);

    in_marshal_ = 1;
    PyUnlockingCdrStream pystream(stream);

    try {
      for (i=0; i < in_l_; i++)
	omniPy::marshalPyObject(pystream,
				PyTuple_GET_ITEM(in_d_,i),
				PyTuple_GET_ITEM(args_,i));
      if (ctxt_d_)
	omniPy::marshalContext(pystream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
    }
    catch (...) {
      in_marshal_ = 0;
      throw;
    }
    in_marshal_ = 0;
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalReturnedValues(cdrStream& stream)
{
  if (out_l_ == -1) return;  // Oneway operation

  cdLockHolder _l(this);

  if (out_l_ == 0) {
    Py_INCREF(Py_None);
    result_ = Py_None;
  }
  else {
    PyUnlockingCdrStream pystream(stream);

    if (out_l_ == 1)
      result_ = omniPy::unmarshalPyObject(pystream,
					  PyTuple_GET_ITEM(out_d_, 0));
    else {
      result_ = PyTuple_New(out_l_);
      if (!result_)
	OMNIORB_THROW(NO_MEMORY, 0,
		      (CORBA::CompletionStatus)stream.completion());

      for (int i=0; i < out_l_; i++) {
	PyTuple_SET_ITEM(result_, i,
			 omniPy::unmarshalPyObject(pystream,
						   PyTuple_GET_ITEM(out_d_,
								    i)));
      }
    }
  }
}


void
omniPy::Py_omniCallDescriptor::userException(cdrStream& stream,
					     IOP_C* iop_client,
					     const char* repoId)
{
  reacquireInterpreterLock();

  PyObject* d_o = PyDict_GetItemString(exc_d_, (char*)repoId);

  if (d_o) { // class, repoId, exc name, name, descriptor, ...

    try {
      PyUserException ex(d_o);
      
      ex <<= stream;
      ex._raise();
    }
    catch (...) {
      releaseInterpreterLock();
      if (iop_client) iop_client->RequestCompleted();
      throw;
    }
    OMNIORB_ASSERT(0); // Never reach here
  }
  else {
    releaseInterpreterLock();
    if (iop_client) iop_client->RequestCompleted(1);
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException,
		  (CORBA::CompletionStatus)stream.completion());
  }
}


void
omniPy::Py_localCallBackFunction(omniCallDescriptor* cd, omniServant* svnt)
{
  Py_omniCallDescriptor* pycd = (Py_omniCallDescriptor*)cd;
  Py_omniServant*        pyos =
    (Py_omniServant*)svnt->_ptrToInterface(omniPy::string_Py_omniServant);

  // Unfortunately, we can't use the call descriptor's
  // reacquireInterpreterLock() function, since this call-back may be
  // running in a different thread to the creator of the call
  // descriptor.

  if (cd->is_upcall()) {
    omnipyThreadCache::lock _t;
    pyos->remote_dispatch(pycd);
  }
  else {
    omnipyThreadCache::lock _t;
    pyos->local_dispatch(pycd);
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalArguments(cdrStream& stream)
{
  OMNIORB_ASSERT(args_ == 0);

  omnipyThreadCache::lock _t;

  if (ctxt_d_)
    args_ = PyTuple_New(in_l_ + 1);
  else
    args_ = PyTuple_New(in_l_);


  PyUnlockingCdrStream pystream(stream);

  int i;
  for (i=0; i < in_l_; i++) {
    PyTuple_SET_ITEM(args_, i,
		     omniPy::unmarshalPyObject(pystream,
					       PyTuple_GET_ITEM(in_d_, i)));
  }
  if (ctxt_d_)
    PyTuple_SET_ITEM(args_, i, omniPy::unmarshalContext(pystream));
}

void
omniPy::Py_omniCallDescriptor::setAndValidateReturnedValues(PyObject* result)
{
  OMNIORB_ASSERT(result_ == 0);
  result_ = result;

  if (out_l_ == -1 || out_l_ == 0) {
    if (result_ != Py_None)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_WrongPythonType,
		    CORBA::COMPLETED_MAYBE);
  }
  else if (out_l_ == 1) {
    omniPy::validateType(PyTuple_GET_ITEM(out_d_,0),
			 result,
			 CORBA::COMPLETED_MAYBE);
  }
  else {
    if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != out_l_)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_WrongPythonType,
		    CORBA::COMPLETED_MAYBE);

    for (int i=0; i < out_l_; i++) {
      omniPy::validateType(PyTuple_GET_ITEM(out_d_,i),
			   PyTuple_GET_ITEM(result,i),
			   CORBA::COMPLETED_MAYBE);
    }
  }
}

void
omniPy::Py_omniCallDescriptor::marshalReturnedValues(cdrStream& stream)
{
  omnipyThreadCache::lock _t;
  PyUnlockingCdrStream pystream(stream);

  if (out_l_ == 1) {
    omniPy::marshalPyObject(pystream,
			    PyTuple_GET_ITEM(out_d_, 0),
			    result_);
  }
  else {
    for (int i=0; i < out_l_; i++) {
      omniPy::marshalPyObject(pystream,
			      PyTuple_GET_ITEM(out_d_,i),
			      PyTuple_GET_ITEM(result_,i));
    }
  }
}
