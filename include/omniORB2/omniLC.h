// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniLifeCycle.cc           Created on: 1997/09/20
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//      

/* 
   $Log$
   Revision 1.2  1997/12/10 13:45:10  sll
   Cleanup to remove constructs that trigger the MSVC nested class bug.

 * Revision 1.1  1997/12/10 11:52:39  sll
 * Initial revision
 *
 * Revision 1.1  1997/09/20  17:41:47  dpg1
 * Initial revision
 *
 */

#ifndef _omniLC_h_
#define _omniLC_h_


#include <omnithread.h>
#include <omniORB2/omniLifeCycle.hh>


// Everything lives inside class omniLC:

class omniLC {
public:

  //
  // _wrap_home class -- base abstract class for home location wrappers:

  class _wrap_home {
  protected:
    omniObject *_dispatcher;	// Object to handle dispatch() calls

  public:
    //
    // Functions needed by omniLifeCycleInfo:

    virtual void _move(CORBA::Object_ptr to) = 0;
    virtual void _remove() = 0;
  };


  //
  // _wrap_proxy class -- base class for proxy object wrappers. Calls
  // to a _wrap_proxy object normally resolve to calls to an _lc_proxy
  // object. On receiving a LOCATION_FORWARD message, the object being
  // wrapped is changed.

  class _wrap_proxy {
  public:
    _wrap_proxy() {
      _next_wrap_proxy = 0;
      _fwd  = 0;
    };

    CORBA::Boolean _forwarded() {
      return _fwd;
    };

    // Register that this _wrap_proxy is wrapping the given local object
    void _register_wrap(omniObject *obj);

    // Register that this _wrap_proxy is no longer wrapping a local object
    void _unregister_wrap();

    // Reset all _wrap_proxies which are wrapping the given local object
    static void _reset_wraps(omniObject *obj);

    // Reset the wrapper to use its original _proxy object (when the
    // object moves back out of our address space).
    virtual void _reset_proxy() = 0;

  protected:
    CORBA::Boolean _fwd;		// Have we been forwarded?
    _wrap_proxy *_next_wrap_proxy;

  private:
    omniObjectKey _wrapped_key;
  };


  //
  // Implementation class for LifeCycleInfo:

  class LifeCycleInfo_i
    : public virtual _sk_omniLifeCycleInfo
  {
  private:
    _wrap_home *wrap;
    CORBA::Object_var  home;

  public:
    LifeCycleInfo_i(_wrap_home *w, CORBA::Object_ptr h)
      : wrap(w)
    {
      home = CORBA::Object::_duplicate(h);
    };

    virtual ~LifeCycleInfo_i() { };

    void reportMove(CORBA::Object_ptr obj) {
      wrap->_move(obj);
    };

    void reportRemove() {
      wrap->_remove();
      CORBA::BOA::getBOA()->dispose(this);
    };

    CORBA::Object_ptr homeObject() {
      return CORBA::Object::_duplicate(home);
    };
  };


  //
  // _lc_sk class defines things common to all _lc_sk skeleton classes:

  class _lc_sk {
  private:
    omniLifeCycleInfo_var _linfo;

  protected:
    void _set_linfo(omniLifeCycleInfo_ptr li) {
      _linfo = omniLifeCycleInfo::_duplicate(li);
    };
    omniLifeCycleInfo_ptr _get_linfo() {
      return _linfo;
    };

  public:
    virtual void _move(CORBA::Object_ptr to) = 0;
    virtual void _remove() = 0;
  };


  //
  // reDirect class creates an omniObject which sends LOCATION_FORWARD
  // messages:

  class reDirect : public virtual omniObject, public virtual CORBA::Object {
  public:

    reDirect(CORBA::Object_ptr fwdref); 
    reDirect(CORBA::Object_ptr fwdref,const omniORB::objectKey& mykey);

    virtual ~reDirect() { }

    CORBA::Object_ptr forwardReference() const;

    CORBA::Object_ptr _this();
    void _obj_is_ready(CORBA::BOA_ptr boa);
    CORBA::BOA_ptr _boa();
    void _dispose();
    omniORB::objectKey _key();
    virtual CORBA::Boolean dispatch(GIOP_S &s, const char *,CORBA::Boolean);

  private:
    CORBA::Object_var pd_fwdref;
    reDirect();
  };


  //
  // _threadControl class helps look after concurrency in LifeCycle object
  // implementations:

  class _threadControl {
  public:

    _threadControl() {
      _mu      = new omni_mutex;
      _moveGo  = new omni_condition(_mu);
      _running = 0;
      _moving  = 0;
    };

    virtual ~_threadControl() {
      delete _moveGo;
      delete _mu;
    };

    //
    // _beginOp() must be called at the start of all operations except
    // ones which affect object existence:

    void _beginOp() {
      _mu->lock();

      if (_moving) {
	_mu->unlock();
	throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
      }

      _running++;
      _mu->unlock();
    };

    //
    // _endOp() must be called at the end of operations started with
    // _beginOp():

    void _endOp() {
      _mu->lock();

      _running--;
      if (_moving && (_running == 0))
	_moveGo->signal();

      _mu->unlock();
    };

    //
    // _beginLC() must be called before all LifeCycle operations instead
    // of _beginOp():

    void _beginLC() {
      _mu->lock();

      if (_moving) {
	_mu->unlock();
	throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
      }

      _moving = 1;

      if (_running > 0)
	_moveGo->wait();

      _mu->unlock();
    };

    //
    // _endLC() must be called after operations started with _beginLC():

    void _endLC() {
      _moving = 0;		// No concurrency control needed
    };

  private:
    omni_mutex     *_mu;
    omni_condition *_moveGo;
    CORBA::Long    _running;
    CORBA::Boolean _moving;
  };


  //
  // Two helper classes to make exception handling with the _threadControl
  // class easier:

// Create a TheadOp object in all normal operations:

  class ThreadOp {
  public:
    ThreadOp(_threadControl *t)
      : tc(t)
    {
      t->_beginOp();
    };

    ~ThreadOp() {
      tc->_endOp();
    };

  private:
    _threadControl *tc;
  };

  // Create a ThreadLC object in all LifeCycle operations:

  class ThreadLC {
  public:
    ThreadLC(_threadControl *t)
      : tc(t)
    {
      t->_beginLC();
    };

    ~ThreadLC() {
      tc->_endLC();
    };

  private:
    _threadControl *tc;
  };

};


#endif // _omniLC_h_
