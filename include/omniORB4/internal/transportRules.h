// -*- Mode: C++; -*-
//                            Package   : omniORB
// transportRule.h            Created on: 21/08/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//

/*
  $Log$
  Revision 1.1.2.3  2001/09/03 13:27:12  sll
  Minor update to comments.

  Revision 1.1.2.2  2001/08/29 17:50:39  sll
  New method dumpRule.

  Revision 1.1.2.1  2001/08/23 16:00:35  sll
  Added method in giopTransportImpl to return the addresses of the host
  interfaces.

*/

#ifndef __TRANSPORTRULE_H__
#define __TRANSPORTRULE_H__

OMNI_NAMESPACE_BEGIN(omni)

class transportRules {
 public:

  ////////////////////////////////////////////////////////////////////////
  static transportRules& serverRules();
  static transportRules& clientRules();

  ////////////////////////////////////////////////////////////////////////
  class sequenceString;
  
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  class Rule {
  public:
    Rule(const char* address_mask) : addressMask_(address_mask) {}
    virtual ~Rule() {}

    virtual CORBA::Boolean match(const char* endpoint) = 0;

    const char* addressMask() { return addressMask_; }

  private:
    CORBA::String_var addressMask_;
    Rule();
    Rule(const Rule&);
    Rule& operator=(const Rule&);
  };

  void addRule(Rule*,sequenceString*);

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  class RuleType {
  public:
    virtual Rule* createRule(const char* address_mask) = 0;
    // If <address_mask> is recognised by this RuleType instance, return
    // a Rule instance to handle this <address_mask>. Otherwise return 0.

    RuleType() {}
    virtual ~RuleType() {}

  private:
    RuleType(const RuleType&);
    RuleType& operator=(const RuleType&);
  };

  static void addRuleType(RuleType*);


  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean match(const char* endpoint,
		       sequenceString& actions/* return arg */,
		       CORBA::ULong& priority/* return arg */);
  // Return true if <endpoint> matches one of the transport rules.
  // The action list of the matched rule is returned in <actions>.
  // The index of the matched rule is returned in <priority>.
  // Return false if <endpoint> does not match any rule. In that case
  // <actions and <priority> are not initialised.

  ////////////////////////////////////////////////////////////////////////
  char* dumpRule(CORBA::ULong index);
  // Return the string representation of the rule at <index>. Returns 0
  // if the index is out of range. If the value of <priority> returned
  // by match() is used as <index> in this function, the string representation
  // of the rule that match() matches is returned.

  ////////////////////////////////////////////////////////////////////////
  class sequenceString_var;

  class sequenceString : public _CORBA_Unbounded_Sequence_String {
  public:
    typedef sequenceString_var _var_type;
    inline sequenceString() {}
    inline sequenceString(const sequenceString& s)
      : _CORBA_Unbounded_Sequence_String(s) {}

    inline sequenceString(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence_String(_max) {}
    inline sequenceString(_CORBA_ULong _max, _CORBA_ULong _len, char** _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence_String(_max, _len, _val, _rel) {}



    inline sequenceString& operator = (const sequenceString& s) {
      _CORBA_Unbounded_Sequence_String::operator=(s);
      return *this;
    }
  };

  ////////////////////////////////////////////////////////////////////////
  class sequenceString_var {
  public:
    typedef sequenceString T;
    typedef sequenceString_var T_var;

    inline sequenceString_var() : _pd_seq(0) {}
    inline sequenceString_var(T* s) : _pd_seq(s) {}
    inline sequenceString_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~sequenceString_var() { if( _pd_seq )  delete _pd_seq; }

    inline T_var& operator = (T* s) {
      if( _pd_seq )  delete _pd_seq;
      _pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s._pd_seq ) {
	if( !_pd_seq )  _pd_seq = new T;
	*_pd_seq = *s._pd_seq;
      } else if( _pd_seq ) {
	delete _pd_seq;
	_pd_seq = 0;
      }
      return *this;
    }

    inline _CORBA_String_element operator [] (_CORBA_ULong s) {
      return (*_pd_seq)[s];
    }

    inline T* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
    inline operator T& () const { return *_pd_seq; }
#else
    inline operator const T& () const { return *_pd_seq; }
    inline operator T& () { return *_pd_seq; }
#endif

    inline const T& in() const { return *_pd_seq; }
    inline T&       inout()    { return *_pd_seq; }
    inline T*&      out() {
      if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
      return _pd_seq;
    }
    inline T* _retn() { T* tmp = _pd_seq; _pd_seq = 0; return tmp; }

  private:
    T* _pd_seq;
  };

#ifdef __GNUG__
  friend class _keep_gcc_quiet_;
#endif

  friend class omni_transportRules_initialiser;
  friend class clientTransportRuleHandler;
  friend class serverTransportRuleHandler;

  struct RuleActionPair {

    RuleActionPair(Rule* r,sequenceString& a) : rule_(r) {
      CORBA::ULong max = a.maximum();
      CORBA::ULong len = a.length();
      action_.replace(max,len,a.get_buffer(1),1);
    }
    ~RuleActionPair() { 
      if (rule_) delete rule_; 
    }
    Rule*               rule_;
    sequenceString      action_;
  };

  transportRules();
  ~transportRules();

 private:

  omnivector<RuleType*>       pd_ruletypes;
  omnivector<RuleActionPair*> pd_rules;

  void reset();


  transportRules(const transportRules&);
  transportRules& operator=(const transportRules&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TRANSPORTRULE_H__
