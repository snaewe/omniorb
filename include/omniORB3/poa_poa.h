// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa_poa.h                  Created on: 8/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

/*
  $Log$
  Revision 1.1.2.1  1999/09/24 09:51:51  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIPOA_H__
#error poa_poa.h should only be included by poa.h
#endif

#ifndef __OMNI_POA_POA_H__
#define __OMNI_POA_POA_H__


_CORBA_MODULE POA_PortableServer
_CORBA_MODULE_BEG


class AdapterActivator :
    public virtual PortableServer::_impl_AdapterActivator,
    public virtual PortableServer::ServantBase
{
public:
  virtual ~AdapterActivator();

  inline PortableServer::AdapterActivator_ptr _this() {
    return (PortableServer::AdapterActivator_ptr) _do_this(PortableServer::AdapterActivator::_PD_repoId);
  }
};


class ServantManager :
    public virtual PortableServer::_impl_ServantManager,
    public virtual PortableServer::ServantBase
{
public:
  virtual ~ServantManager();

  inline PortableServer::ServantManager_ptr _this() {
    return (PortableServer::ServantManager_ptr) _do_this(PortableServer::ServantManager::_PD_repoId);
  }
};


class ServantActivator :
    public virtual PortableServer::_impl_ServantActivator,
    public virtual PortableServer::ServantBase
{
public:
  virtual ~ServantActivator();

  inline PortableServer::ServantActivator_ptr _this() {
    return (PortableServer::ServantActivator_ptr) _do_this(PortableServer::ServantActivator::_PD_repoId);
  }
};


class ServantLocator :
    public virtual PortableServer::_impl_ServantLocator,
    public virtual PortableServer::ServantBase
{
public:
  virtual ~ServantLocator();

  inline PortableServer::ServantLocator_ptr _this() {
    return (PortableServer::ServantLocator_ptr) _do_this(PortableServer::ServantLocator::_PD_repoId);
  }
};


_CORBA_MODULE_END  // POA_PortableServer


#endif  // __OMNI_POA_POA_H__
