// -*- Mode: C++; -*-
//                            Package   : omniORB2
// any.cc                     Created on: 31/07/97
//                            Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
//      Implementation of type any


/* $Log$
/* Revision 1.1  1998/01/27 15:43:47  ewc
/* Initial revision
/*
 */

#include <iostream.h>
#include <string.h>

#include <omniORB2/CORBA.h>
#include "tcParseEngine.h"


CORBA::Any::Any(CORBA::TypeCode_ptr tc, void* value, CORBA::Boolean release,
 		   CORBA::Boolean nocheck) : pd_data(0)
{
  if (nocheck) 
    {
      pd_tc = tc->NP_aliasExpand();
      CORBA::release(tc);
    }
  else pd_tc = CORBA::TypeCode::_duplicate(tc);

  CORBA::Char* _val = (CORBA::Char*) value; 

  if (_val != 0)
    {
      MemBufferedStream mbuf(_val);
      CORBA::ULong fillerLen = (pd_tc->pd_param).alreadyWritten() % 4;
      
      if (fillerLen > 0)
	{
	  if (!nocheck) mbuf.skip(fillerLen);  
	  CORBA::Char* filler = new CORBA::Char[fillerLen];

	  try
	    {
	      pd_mbuf.put_char_array(filler,fillerLen);
	    }
	  catch(...)
	    {
	      delete[] filler;
	      throw;
	    }

	  delete[] filler;
	}
      
      tcParseEngine tcEngine(pd_tc,&mbuf);
      tcEngine.parse(pd_mbuf);

      if (release) delete[] _val;
    }
  else if (!tc->NP_expandEqual(CORBA::_tc_null,1) && 
	   !tc->NP_expandEqual(CORBA::_tc_void,1))
    {
      CORBA::release(pd_tc);
      pd_tc = 0;
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    }
}  
	  

/**************************************************************************/

void 
CORBA::Any::NP_memAlignMarshal(MemBufferedStream& m)
{
  *pd_tc >>= m;

  if (pd_tc->pd_tck  <= CORBA::tk_void) return;
  else if (pd_mbuf.alreadyWritten() == 0)
    {
      if (pd_tc->pd_tck != CORBA::tk_alias && pd_tc->pd_tck != CORBA::tk_any &&
	  pd_tc->pd_tck != CORBA::tk_except)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);  
      return;
    }

  CORBA::UShort fillerLen = 0;

  if (pd_tc->pd_tck > CORBA::tk_Principal && pd_tc->pd_tck != 
                                                            CORBA::tk_string) 
    fillerLen = (pd_tc->pd_param).alreadyWritten() % 4;
  
#if !defined(NO_FLOAT)
  if (pd_tc->pd_tck == CORBA::tk_double || pd_tc->pd_tck == CORBA::tk_any || 
      pd_tc->pd_tck == CORBA::tk_struct || pd_tc->pd_tck == CORBA::tk_union ||
      pd_tc->pd_tck > CORBA::tk_string)
    {
      MemBufferedStream tmpInBuf(pd_mbuf,1);
      
      tmpInBuf.skip(fillerLen);

      tcParseEngine tcEngine(pd_tc,&tmpInBuf);
      tcEngine.parse(m,1);
    }
  else
#endif
    {
      m.put_char_array((_CORBA_Char*) (omni::ptr_arith_t) pd_mbuf.data() +
		       (omni::ptr_arith_t) fillerLen, 
		       pd_mbuf.alreadyWritten() - fillerLen);
    }
}


void
CORBA::Any::operator>>= (NetBufferedStream& s) const
{
  *pd_tc >>= s;

  if (pd_tc->pd_tck  <= CORBA::tk_void) return;
  else if (pd_mbuf.alreadyWritten() == 0)
    {
      if (pd_tc->pd_tck != CORBA::tk_alias && pd_tc->pd_tck != CORBA::tk_any &&
	  pd_tc->pd_tck != CORBA::tk_except)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);  
      return;
    }

  CORBA::UShort fillerLen = 0;

  if (pd_tc->pd_tck > CORBA::tk_Principal && pd_tc->pd_tck != 
                                                            CORBA::tk_string) 
    fillerLen = (pd_tc->pd_param).alreadyWritten() % 4;

#if !defined(NO_FLOAT)
  if ((pd_tc->pd_tck == CORBA::tk_double || pd_tc->pd_tck == CORBA::tk_any || 
       pd_tc->pd_tck == CORBA::tk_struct || pd_tc->pd_tck == CORBA::tk_union ||
       pd_tc->pd_tck > CORBA::tk_string) && 
      s.WrMessageAlreadyWritten()%8 > 0 && s.WrMessageAlreadyWritten()%8 <= 4)
    {
      MemBufferedStream outBuf;
      MemBufferedStream tmpInBuf(pd_mbuf,1);

      PR_fill(fillerLen,outBuf);
      tmpInBuf.skip(fillerLen);

      tcParseEngine tcEngine(pd_tc,&tmpInBuf);
      tcEngine.parse(outBuf,1);

      s.put_char_array((_CORBA_Char*) (omni::ptr_arith_t) outBuf.data() +
		       (omni::ptr_arith_t) fillerLen,
		       outBuf.alreadyWritten() - fillerLen );
    }
  else
#endif
    {
      s.put_char_array((_CORBA_Char*) (omni::ptr_arith_t) pd_mbuf.data() +
		       (omni::ptr_arith_t) fillerLen, 
		       pd_mbuf.alreadyWritten() - fillerLen);
    }
}


void
CORBA::Any::operator>>= (MemBufferedStream& m) const
{
  *pd_tc >>= m;
  
  if (pd_tc->pd_tck <= CORBA::tk_void) return;
  else if (pd_mbuf.alreadyWritten() == 0)
    {
      if (pd_tc->pd_tck != CORBA::tk_alias && pd_tc->pd_tck != CORBA::tk_any &&
	  pd_tc->pd_tck != CORBA::tk_except)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      return;
    }

  CORBA::UShort fillerLen = 0;

  if (pd_tc->pd_tck > CORBA::tk_Principal && pd_tc->pd_tck != CORBA::tk_string)
    fillerLen = (pd_tc->pd_param).alreadyWritten() % 4;


#if !defined(NO_FLOAT)
  if ((pd_tc->pd_tck == CORBA::tk_double || pd_tc->pd_tck == CORBA::tk_any || 
       pd_tc->pd_tck == CORBA::tk_struct || pd_tc->pd_tck == CORBA::tk_union ||
       pd_tc->pd_tck > CORBA::tk_string) && 
      m.alreadyWritten()%8 > 0 && m.alreadyWritten()%8 <= 4)
   {
     MemBufferedStream tmpInBuf(pd_mbuf,1);
     tmpInBuf.skip(fillerLen);

     tcParseEngine tcEngine(pd_tc,&tmpInBuf);
     tcEngine.parse(m);
   }
  else
#endif  
    {
      m.put_char_array((_CORBA_Char*) (omni::ptr_arith_t) pd_mbuf.data() +
		       (omni::ptr_arith_t) fillerLen, 
		       pd_mbuf.alreadyWritten() - fillerLen);
    }
}


void 
CORBA::Any::operator<<=(NetBufferedStream& s)
{
  *pd_tc <<= s;

  pd_mbuf.rewind_inout_mkr();
  if (pd_tc->pd_tck <= CORBA::tk_void) return;
  else if (pd_tc->pd_tck > CORBA::tk_Principal && 
	   pd_tc->pd_tck != CORBA::tk_string &&
	   (pd_tc->pd_param).alreadyWritten() % 4 > 0)
    PR_fill((pd_tc->pd_param).alreadyWritten() % 4,pd_mbuf);
    
  tcParseEngine tcEngine(pd_tc,&s);
  tcEngine.parse(pd_mbuf);
  PR_deleteData();
}  


void 
CORBA::Any::operator<<=(MemBufferedStream& m)
{
  *pd_tc <<= m;

  pd_mbuf.rewind_inout_mkr();
  if (pd_tc->pd_tck <= CORBA::tk_void) return;
  else if (pd_tc->pd_tck > CORBA::tk_Principal && 
	   pd_tc->pd_tck != CORBA::tk_string &&
	   (pd_tc->pd_param).alreadyWritten() % 4)
    PR_fill((pd_tc->pd_param).alreadyWritten() % 4, pd_mbuf);

  tcParseEngine tcEngine(pd_tc,&m);
  tcEngine.parse(pd_mbuf);
  PR_deleteData();
}  



size_t
CORBA::Any::NP_alignedSize(size_t _initialoffset) const
{
  CORBA::ULong _msgsize = _initialoffset;      
  _msgsize = pd_tc->NP_alignedSize(_msgsize);

  UShort fillerLen = 0;

  if (pd_tc->pd_tck <= CORBA::tk_void) return _msgsize;
  else if (pd_mbuf.alreadyWritten() == 0)
    {
      if (pd_tc->pd_tck != CORBA::tk_alias && pd_tc->pd_tck != CORBA::tk_any &&
	  pd_tc->pd_tck != CORBA::tk_except)
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      return _msgsize;
    }
  else if (pd_tc->pd_tck > CORBA::tk_Principal && 
	   pd_tc->pd_tck != CORBA::tk_string)
    fillerLen = (pd_tc->pd_param).alreadyWritten()%4;
   
#if !defined(NO_FLOAT)
  if ((pd_tc->pd_tck == CORBA::tk_double || pd_tc->pd_tck == CORBA::tk_any || 
       pd_tc->pd_tck == CORBA::tk_struct || pd_tc->pd_tck == CORBA::tk_union ||
       pd_tc->pd_tck > CORBA::tk_string) && _msgsize%8 > 0 && _msgsize%8 <= 4)
	{
	  MemBufferedStream outBuf;
	  MemBufferedStream tmpInBuf(pd_mbuf,1);

	  PR_fill(fillerLen,outBuf);
	  tmpInBuf.skip(fillerLen);

	  tcParseEngine tcEngine(pd_tc,&tmpInBuf);
	  tcEngine.parse(outBuf,1);

	  _msgsize += (outBuf.alreadyWritten() - fillerLen);
	}
      else
#endif
	{
	  _msgsize += (pd_mbuf.alreadyWritten() - fillerLen);
	}
  
  return _msgsize;
}


void 
CORBA::Any::NP_holdData(void* data, void (*del)(void*)) const 
{
  void** tmp_data = (void**) &pd_data; 
  void (**tmp_del)(void*);
  tmp_del = (void (**)(void*)) &deleteData;
      
  (*tmp_data) = data;
  (*tmp_del) = del;
}


void
CORBA::Any::NP_getBuffer(MemBufferedStream& mbuf) const
{
  mbuf.shallowCopy(pd_mbuf);
 if (pd_tc->pd_tck > CORBA::tk_Principal && pd_tc->pd_tck != CORBA::tk_string) 
    mbuf.skip((pd_tc->pd_param).alreadyWritten() % 4);
}

void 
CORBA::Any::NP_replaceData(CORBA::TypeCode_ptr tcp, 
			                        const MemBufferedStream& mb) 
{
  CORBA::release(pd_tc);
  PR_deleteData();
    
  if (omniORB::tcAliasExpand) pd_tc = tcp->NP_completeExpand();
  else pd_tc = tcp->NP_aliasExpand();
  pd_mbuf = mb;
}

/**************************************************************************/
      


void
CORBA::Any::operator<<=(CORBA::Any::from_string s)
{
  CORBA::ULong _len = strlen(s.val);

  if (_len > s.bound && s.bound != 0)
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  else
    {
      MemBufferedStream mbuf;
      _len++;
      _len >>= mbuf;
      mbuf.put_char_array((Char*) s.val,_len);

      CORBA::TypeCode_var boundedTC(
			       new CORBA::TypeCode(CORBA::tk_string,s.bound));
      NP_replaceData(boundedTC, mbuf);
      
      if (s.nc) delete[] s.val;
    }
}



CORBA::Boolean 
CORBA::Any::operator>>=(CORBA::Any& a) const 
{
  if (!pd_tc->NP_expandEqual(_tc_any,1)) return 0;
  else
    {
      if (this == &a) 
	{
	  MemBufferedStream tmp_mbuf(pd_mbuf);

	  CORBA::release(a.pd_tc);
	  a.PR_deleteData();
	  a.pd_tc = new CORBA::TypeCode(CORBA::tk_null);
	  MemBufferedStream mbuf;
	  a.pd_mbuf = mbuf;
       
	  a <<= tmp_mbuf;
	}
      else
	{
	  CORBA::release(a.pd_tc);
	  a.PR_deleteData();
	  a.pd_tc = new CORBA::TypeCode(CORBA::tk_null);
	  MemBufferedStream mbuf;
	  a.pd_mbuf = mbuf;
       
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  a <<= tmp_mbuf;
	}
      return 1;
    }
}


CORBA::Boolean 
CORBA::Any::operator>>=(CORBA::TypeCode_ptr& tc) const 
{
  if (!pd_tc->NP_expandEqual(_tc_TypeCode,1)) return 0;
  else 
    {
      MemBufferedStream tmp_mbuf;
      NP_getBuffer(tmp_mbuf);
	  
      CORBA::TypeCode_ptr tmp_tc = new CORBA::TypeCode(CORBA::tk_null);
      *tmp_tc <<= tmp_mbuf;
      tc = tmp_tc;
      return 1;
    }
}

CORBA::Boolean 
CORBA::Any::operator>>=(char*& s) const 
{
  if (!pd_tc->NP_expandEqual(_tc_string,1)) 
    {
      s = 0;
      return 0;
    }
  else
    {
      MemBufferedStream tmp_mbuf(pd_mbuf,1);
      CORBA::ULong _len;
      _len <<= tmp_mbuf;
      if (tmp_mbuf.overrun(_len)) throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      // This check has already been performed by TC Parser.

      s = string_alloc(_len - 1);
      tmp_mbuf.get_char_array((Char*) s, _len);
      return 1;
    }
}

CORBA::Boolean 
CORBA::Any::operator>>=(CORBA::Any::to_string s) const
{
  CORBA::TypeCode_ptr boundedStrTC = new CORBA::TypeCode(CORBA::tk_string,
							 s.bound);

  if (!pd_tc->NP_expandEqual(boundedStrTC,1)) 
    {
      CORBA::release(boundedStrTC);
      s.val = 0;
      return 0;
    }
  else
    {
      CORBA::release(boundedStrTC);

      MemBufferedStream tmp_mbuf(pd_mbuf,1);
      
      CORBA::ULong _len;
      _len <<= tmp_mbuf;
      if (tmp_mbuf.overrun(_len)) throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      // This check has already been performed by TC Parser.

      s.val = string_alloc(_len - 1);
      tmp_mbuf.get_char_array((Char*) s.val, _len);
      return 1;
    }
}


CORBA::Boolean 
CORBA::Any::operator>>=(CORBA::Any::to_object o) const 
{
  CORBA::TypeCode_var expandedTC = pd_tc->NP_aliasExpand();
  if (expandedTC->kind() != CORBA::tk_objref) return 0;
  else 
    {
      MemBufferedStream tmp_mbuf;
      NP_getBuffer(tmp_mbuf);
      CORBA::Object_ptr tmpObjRef;
      tmpObjRef = CORBA::Object::unmarshalObjRef(tmp_mbuf);
      o.ref = tmpObjRef;
      return 1;
    }
}

void 
CORBA::Any::replace(CORBA::TypeCode_ptr tc, void* value, 
		    CORBA::Boolean release)
{
  CORBA::release(pd_tc);
  PR_deleteData();
  pd_tc = CORBA::TypeCode::_duplicate(tc);  

  MemBufferedStream dbuf;
  CORBA::Char* _val = (CORBA::Char*) value;  

  if (_val != 0)
    {
      MemBufferedStream mbuf(_val);
      CORBA::ULong fillerLen = (pd_tc->pd_param).alreadyWritten() % 4;
      
      if (fillerLen > 0)
	{
	  mbuf.skip(fillerLen);  
	  CORBA::Char* filler = new CORBA::Char[fillerLen];

	  try
	    {
	      dbuf.put_char_array(filler,fillerLen);
	    }
	  catch(...)
	    {
	      delete[] filler;
	      throw;
	    }
	  delete[] filler;

	} 

      tcParseEngine tcEngine(pd_tc,&mbuf);
      tcEngine.parse(dbuf);
      if (release) delete[] _val;
    }
  else if (!tc->NP_expandEqual(CORBA::_tc_null,1) && 
	   !tc->NP_expandEqual(CORBA::_tc_void,1))
    {
      CORBA::release(pd_tc);
      pd_tc = 0;
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    }

  pd_mbuf = dbuf;
}


