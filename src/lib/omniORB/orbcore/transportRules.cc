// -*- Mode: C++; -*-
//                            Package   : omniORB
// transportRule.cc           Created on: 21/08/2001
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
  Revision 1.1.2.3  2001/08/31 11:56:52  sll
  Change the default preference to unix,tcp,ssl.
  Minor fix to extractIPv4.

  Revision 1.1.2.2  2001/08/29 17:54:15  sll
  New method dumpRule.

  Revision 1.1.2.1  2001/08/23 16:00:35  sll
  Added method in giopTransportImpl to return the addresses of the host
  interfaces.

*/

#include <omniORB4/CORBA.h>
#include <orbOptions.h>
#include <transportRules.h>
#include <initialiser.h>
#include <SocketCollection.h>
#include <libcWrapper.h>
#include <omniORB4/giopEndpoint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tcp/tcpConnection.h>

OMNI_NAMESPACE_BEGIN(omni)

static transportRules serverRules_;
static transportRules clientRules_;

static char* dumpRuleString(transportRules::RuleActionPair* ra);

/////////////////////////////////////////////////////////////////////////////
transportRules::transportRules() {
}

/////////////////////////////////////////////////////////////////////////////
transportRules::~transportRules() {
  reset();
}

/////////////////////////////////////////////////////////////////////////////
void
transportRules::reset() {
  omnivector<RuleActionPair*>::iterator i = pd_rules.begin();
  omnivector<RuleActionPair*>::iterator last = pd_rules.end();

  for (; i != last; i++) {
    delete (*i);
  }
  pd_rules.erase(pd_rules.begin(),last);
}

/////////////////////////////////////////////////////////////////////////////
transportRules&
transportRules::serverRules() {
  return serverRules_;
}

/////////////////////////////////////////////////////////////////////////////
transportRules&
transportRules::clientRules() {
  return clientRules_;
}

/////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
transportRules::match(const char* endpoint,
		      transportRules::sequenceString& actions,
		      CORBA::ULong& priority) {

  omnivector<RuleActionPair*>::iterator i = pd_rules.begin();
  omnivector<RuleActionPair*>::iterator last = pd_rules.end();

  while (i != last) {
    if ((*i)->rule_->match(endpoint)) {
      CORBA::ULong max = (*i)->action_.maximum();
      CORBA::ULong len = (*i)->action_.length();
      actions.replace(max,len,(*i)->action_.get_buffer(),0);
      priority = i - pd_rules.begin();
      return 1;
    }
    i++;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
char*
transportRules::dumpRule(CORBA::ULong index) {
  omnivector<RuleActionPair*>::iterator i = pd_rules.begin();
  omnivector<RuleActionPair*>::iterator last = pd_rules.end();

  if ( (i+index) >= last ) return 0;

  return dumpRuleString((*(i+index)));
}


/////////////////////////////////////////////////////////////////////////////
static 
omnivector<transportRules::RuleType*>*&
ruleTypes() {
  static omnivector<transportRules::RuleType*>* ruletypes_ = 0;
  if (!ruletypes_) {
    ruletypes_ = new omnivector<transportRules::RuleType*>;
  }
  return ruletypes_;
}

/////////////////////////////////////////////////////////////////////////////
void
transportRules::addRuleType(transportRules::RuleType* rt) {
  ruleTypes()->push_back(rt);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class builtinMatchAllRule : public transportRules::Rule {
public:
  builtinMatchAllRule(const char* address_mask) : 
    transportRules::Rule(address_mask) {}

  ~builtinMatchAllRule() {}

  CORBA::Boolean match(const char*) { return 1; }
};

/////////////////////////////////////////////////////////////////////////////
static char* extractIPv4(const char* endpoint) {
  // returns the ipv4 address if there is one in the endpoint string.
  // To maximise the kind of endpoint we understand,
  // we assume that any endpoint which has an IPv4 address would be
  // of this form:   giop:*:w.x.y.z:*
  // This is certainly true for giop:tcp and giop:ssl

  const char* p = strchr(endpoint,':');
  if (p) p = strchr(p+1,':');
  if (p) {
    p++;
    const char* q = strchr(p,':');
    if (!q) return 0;
    CORBA::ULong l = q - p;
    if (l) {
      CORBA::String_var ipv4(CORBA::string_alloc(l));
      strncpy(ipv4,p,l);
      *((char*)ipv4+l) = '\0';
      if ( LibcWrapper::isipaddr(ipv4) ) 
	return ipv4._retn();
    }
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
class builtinLocalHostRule : public transportRules::Rule {
public:
  builtinLocalHostRule(const char* address_mask) : 
    transportRules::Rule(address_mask) {}

  ~builtinLocalHostRule() {}

  CORBA::Boolean match(const char* endpoint) { 

    if (strncmp(endpoint,"giop:unix",9) == 0) return 1;

    // Otherwise, we want to check if this endpoint is one of those
    // with an IPv4 address.
    CORBA::String_var ipv4;
    ipv4 = extractIPv4(endpoint);
    if ( (const char*)ipv4 )  {
      // Get this host's IP addresses and look for a match
      const omnivector<const char*>* ifaddrs;
      ifaddrs = giopTransportImpl::getInterfaceAddress("giop:tcp");
      if (!ifaddrs) return 0;
      {
	omnivector<const char*>::const_iterator i    = ifaddrs->begin();
	omnivector<const char*>::const_iterator last = ifaddrs->end();
	while ( i != last ) {
	  if ( strcmp((*i),ipv4) == 0 ) return 1;
	  i++;
	}
      }
    }

    return 0; 
  }
};

/////////////////////////////////////////////////////////////////////////////
class builtinIPv4Rule : public transportRules::Rule {
public:
  builtinIPv4Rule(const char* address_mask,
		  CORBA::ULong n, CORBA::ULong m) : 
    transportRules::Rule(address_mask), network_(n), netmask_(m) {}

  ~builtinIPv4Rule() {}

  CORBA::Boolean match(const char* endpoint) { 

    CORBA::String_var ipv4;
    ipv4 = extractIPv4(endpoint);
    if ((const char*)ipv4) {
      CORBA::ULong address = inet_addr(ipv4);
      return (network_ == (address & netmask_));
    }

    if (strncmp(endpoint,"giop:unix",9) == 0) {
      // local transport. Does this rule applies to this  host's 
      // IP address(es)? 
      const omnivector<const char*>* ifaddrs;
      ifaddrs = giopTransportImpl::getInterfaceAddress("giop:tcp");
      if (!ifaddrs) return 0;
      {
	omnivector<const char*>::const_iterator i    = ifaddrs->begin();
	omnivector<const char*>::const_iterator last = ifaddrs->end();
	while ( i != last ) {
	  CORBA::ULong address = inet_addr((*i));
	  if ( network_ == (address & netmask_) ) return 1;
	  i++;
	}
	return 0;
      }
    }
    return 0;
  }

private:
  CORBA::ULong network_;
  CORBA::ULong netmask_;
  CORBA::Boolean apply_to_localhost_;
};

/////////////////////////////////////////////////////////////////////////////
class builtinRuleType : public transportRules::RuleType {
public:
  builtinRuleType() {
    transportRules::addRuleType((transportRules::RuleType*)this);
  }
  virtual ~builtinRuleType() {}

  transportRules::Rule* createRule(const char* address_mask) {
    
    CORBA::ULong network, netmask;

    if ( strcmp(address_mask,"*" ) == 0 ) {
      return (transportRules::Rule*) new builtinMatchAllRule(address_mask);
    }
    else if ( strcmp(address_mask,"localhost") == 0 ) {
      return (transportRules::Rule*) new builtinLocalHostRule(address_mask);
    }
    else if ( parseIPv4AddressMask(address_mask,network,netmask) ) {
      return (transportRules::Rule*) new builtinIPv4Rule(address_mask,
							 network,
							 netmask);
    }
    else {
      return 0;
    }
  }

  static CORBA::Boolean parseIPv4AddressMask(const char* address,
					     CORBA::ULong& network,
					     CORBA::ULong& netmask) {
    CORBA::String_var cp(address);
    char* mask = strchr(cp,'/');
    if (mask) {
      *mask = '\0';
      mask++;
    }
    else {
      mask = (char*) "255.255.255.255";
    }

    if ( ! LibcWrapper::isipaddr(cp) ) return 0;
    network = inet_addr(cp);

    if ( ! LibcWrapper::isipaddr(mask) ) return 0;
    netmask = inet_addr(mask);

    return 1;
  }
};

static builtinRuleType builtinRuleType_;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
parseAndAddRuleString(omnivector<transportRules::RuleActionPair*>& ruleStore,
		      const char* rule_string) {

  transportRules::sequenceString action(4);
  CORBA::String_var address_mask;

  CORBA::String_var rs(rule_string);  // make a copy

  // Extract address mask
  char* p = rs;
  while ( isspace(*p) )
    p++;

  char* q = p;
  while ( !isspace(*p) && *p != '\0' )
    p++;

  if (*p == '\0')
    return 0;

  *p = '\0';
  address_mask = CORBA::string_dup(q);
  p++;

  // Extract action list, one or more comma separated action.
  // There may also be white spaces between the actions and comma separators.
  while ( isspace(*p) )
    p++;
  
  q = p;
  
  p = strchr(q,',');
  while ( p && p != q ) {
    *p = '\0';
    char* t = q;
    while ( !isspace(*t) && *t != '\0' )
      t++;
    *t = '\0';
    action.length(action.length()+1);
    action[action.length()-1] = (const char*) q;

    p++;
    while ( isspace(*p) )
      p++;
    q = p;
    p = strchr(q,',');
  }
  if (*q == ',')
    return 0;
  if (*q != '\0') {
    p = q;
    while ( !isspace(*p) && *p != '\0' )
      p++;
    if (*p != '\0')
      *p = '\0';
    action.length(action.length()+1);
    action[action.length()-1] = (const char*) q;
  }

  omnivector<transportRules::RuleType*>& ruletypes = *ruleTypes();
  omnivector<transportRules::RuleType*>::iterator i = ruletypes.begin();
  omnivector<transportRules::RuleType*>::iterator last = ruletypes.end();

  while (i != last) {
    transportRules::Rule* rule = (*i)->createRule(address_mask);
    if (rule) {
      transportRules::RuleActionPair* ra;
      ra = new transportRules::RuleActionPair(rule,action);
      ruleStore.push_back(ra);
      return 1;
    }
    i++;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
static
char*
dumpRuleString(transportRules::RuleActionPair* ra) {

  transportRules::sequenceString& ss = ra->action_;
  CORBA::ULong len = strlen(ra->rule_->addressMask()) + 1;

  CORBA::ULong i = 0;
  for (; i < ss.length(); i++) {
    len += strlen(ss[i]) + 1;
  }
  
  CORBA::String_var v(CORBA::string_alloc(len));
  sprintf(v,"%s ",ra->rule_->addressMask());

  i = 0;
  while ( i < ss.length() ) {
    strcat(v,ss[i]);
    i++;
    if (i != ss.length()) strcat(v,",");
  }
  return v._retn();
}


/////////////////////////////////////////////////////////////////////////////
class clientTransportRuleHandler : public orbOptions::Handler {
public:

  clientTransportRuleHandler() : 
    orbOptions::Handler("clientTransportRule",
			"clientTransportRule = <address mask>  [action]+",
			0,
			0) {}

  void visit(const char* value,
	     orbOptions::Source)  throw (orbOptions::BadParam) {

    if (!parseAndAddRuleString(clientRules_.pd_rules,value)) {
      throw orbOptions::BadParam(key(),value,"Unrecognised address mask");
    }
  }

  void dump(orbOptions::sequenceString& result) {
    omnivector<transportRules::RuleActionPair*>
      ::iterator i = clientRules_.pd_rules.begin();
    omnivector<transportRules::RuleActionPair*>
      ::iterator last = clientRules_.pd_rules.end();

    while ( i != last ) {
      CORBA::String_var v;
      v = dumpRuleString(*i);
      orbOptions::addKVString(key(),v,result);
      i++;
    }
  }
};

static clientTransportRuleHandler clientTransportRuleHandler_;

/////////////////////////////////////////////////////////////////////////////
class serverTransportRuleHandler : public orbOptions::Handler {
public:

  serverTransportRuleHandler() : 
    orbOptions::Handler("serverTransportRule",
			"serverTransportRule = <address mask>  [action]+",
			0,
			0) {}

  void visit(const char* value,
	     orbOptions::Source) throw (orbOptions::BadParam) {

    if (!parseAndAddRuleString(serverRules_.pd_rules,value)) {
      throw orbOptions::BadParam(key(),value,"Unrecognised address mask");
    }
  }

  void dump(orbOptions::sequenceString& result) {
    omnivector<transportRules::RuleActionPair*>
      ::iterator i = serverRules_.pd_rules.begin();
    omnivector<transportRules::RuleActionPair*>
      ::iterator last = serverRules_.pd_rules.end();

    while ( i != last ) {
      CORBA::String_var v;
      v = dumpRuleString(*i);
      orbOptions::addKVString(key(),v,result);
      i++;
    }
  }
};

static serverTransportRuleHandler serverTransportRuleHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
class omni_transportRules_initialiser : public omniInitialiser {
public:

  omni_transportRules_initialiser() {
    orbOptions::singleton().registerHandler(clientTransportRuleHandler_);
    orbOptions::singleton().registerHandler(serverTransportRuleHandler_);
  }
  virtual ~omni_transportRules_initialiser() {
    omnivector<transportRules::RuleType*>*& ruletypes = ruleTypes();
    if (ruletypes) {
      delete ruletypes;
      ruletypes = 0;
    }
  }
  void attach() { 
    if (clientRules_.pd_rules.size() == 0) {
      // Add a default rule
      parseAndAddRuleString(clientRules_.pd_rules,
                            "*  unix,ssl,tcp");
    }
    if (serverRules_.pd_rules.size() == 0) {
      // Add a default rule
      parseAndAddRuleString(serverRules_.pd_rules,
			    "* unix,ssl,tcp");
    }
  }
  void detach() { 
    serverRules_.reset();
    clientRules_.reset();
  }
};


static omni_transportRules_initialiser initialiser;

omniInitialiser& omni_transportRules_initialiser_ = initialiser;



OMNI_NAMESPACE_END(omni)
