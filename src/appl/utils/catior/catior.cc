// -*- Mode: C++; -*-
//                          Package   : catior
// catior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of catior.
//
//  Catior is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
//
// Lists contents of an IOR.

#include <iostream.h>
#include <stdlib.h>
#ifndef __WIN32__
#include <unistd.h>
#endif

#include <omniORB2/CORBA.h>

#ifndef Swap16
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#else
#error "Swap16 has already been defined"
#endif

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

static int hexflag = 0;

//////////////////////////////////////////////////////////////////////////
static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] <stringified IOR>" << endl;
  cerr << "         flag:" << endl;
  cerr << "               -x print object key as a hexadecimal value." << endl;
}


#if defined(__WIN32__) || defined(__VMS) && __CRTL_VER < 60200000

// WIN32 doesn't have an implementation of getopt() - 
// supply a getopt() for this program:

char* optarg;
int optind = 1;


//////////////////////////////////////////////////////////////////////////
int
getopt(int num_args, char* const* args, const char* optstring)
{
  if (optind == num_args) return EOF;
  char* buf_left = *(args+optind);

  if ((*buf_left != '-' && *buf_left != '/') || buf_left == NULL ) return EOF;
  else if ((optind < (num_args-1)) && strcmp(buf_left,"-") == 0 && 
	   strcmp(*(args+optind+1),"-") == 0)
    {
      optind+=2;
      return EOF;
    }
  else if (strcmp(buf_left,"-") == 0)
    {
      optind++;
      return '?';
    }

  for(int count = 0; count < strlen(optstring); count++)
    {
      if (optstring[count] == ':') continue;
      if (buf_left[1] == optstring[count])
	{
	  if(optstring[count+1] == ':')
	    {
	      if (strlen(buf_left) > 2)
		{
		  optarg = (buf_left+2);
		  optind++;
		}
	      else if (optind < (num_args-1))
		{
		  optarg = *(args+optind+1);
		  optind+=2;
		}
	      else
		{
		  optind++;
		  return '?';
		}
	    }
	  else optind++;
	  
	  return buf_left[1];
	}
    }
  optind++;
  return '?';
}

#endif


//////////////////////////////////////////////////////////////////////////
static
void
write_char(char val,CORBA::Boolean inhex) {
  if (inhex) {
    int v = ((val & 0xf0) >> 4);
    if (v < 10)
      cout << (char)('0' + v);
    else
      cout << (char)('a' + (v - 10));
    
    v = (val & 0xf);
    if (v < 10)
      cout << (char)('0' + v);
    else
      cout << (char)('a' + (v - 10));
  }
  else {
    if (val >= ' ' && val <= '~')
      cout << val;
    else
      cout << ".";
  }
}



#if !defined(__WIN32__)
extern char* optarg;
extern int optind;
#endif

static void decodeIIOPprofile(const _CORBA_Unbounded_Sequence_Octet&);

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  if (argc < 2) 
    {
      usage(argv[0]);
      return 1;
    }	


  // Get options:

  int c;

  while((c = getopt(argc,argv,"x")) != EOF)
    {
      switch(c)
	{
	case 'x':
	  hexflag = 1;
	  break;
	case '?':
	case 'h':
	  {
	    usage(argv[0]);
	    return 1;
	  }
	}
    }
	  
  if (optind >= argc)
    {
      usage(argv[0]);
      return 1;
    }

  
  char* str_ior = argv[optind];

  char* repoID;
  IOP::TaggedProfileList* profiles;
  CORBA::ORB_ptr orb;

  try
    {
      orb = CORBA::ORB_init(argc,argv,"omniORB2");
      IOP::EncapStrToIor(str_ior, repoID, profiles);
      if (*repoID == '\0' && profiles->length() == 0)
	{
	  cerr << "IOR is a nil object reference." << endl;
	}
      else
	{
	  cerr << "Type ID: \"" << (char*) repoID << "\"" << endl;
	  cerr << "Profiles:" << endl;
	  
	  for (long count=0; count < profiles->length(); count++) {

	    cout << count+1 << ". ";
	      
	    if ((*profiles)[count].tag == IOP::TAG_INTERNET_IOP) {
	      decodeIIOPprofile((*profiles)[count].profile_data);
	    }
	    else if ((*profiles)[count].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	      cout << "Multiple Component Tag" << endl;
	    }
	    else {
	      cout << "Unrecognised profile tag: " 
		   << (int) ((*profiles)[count].tag) 
		   << endl;
	    }
	  }
	}
    }
  catch(CORBA::MARSHAL& ex)
    {
      cerr << "Invalid stringified IOR supplied." << endl;
      cerr << "(Minor = " << ex.minor() << ")" << endl;
      return -1;
    }
#if 0
  catch(...)
    {
      cerr << "Exception while processing stringified IOR." << endl;
      return -1;
    }
#endif
  orb->NP_destroy();

  delete[] repoID;
  delete profiles;
  return 1;
}

static void decodeIIOPComponents(IOP::TaggedComponent&);
static void EncapStreamToProfile(const _CORBA_Unbounded_Sequence_Octet&,
				 IIOP::ProfileBody&);

//////////////////////////////////////////////////////////////////////////
static
void
decodeIIOPprofile(const _CORBA_Unbounded_Sequence_Octet &profile)
{
  IIOP::ProfileBody pBody;
  EncapStreamToProfile(profile,pBody);

  cout << "IIOP " << (int) pBody.version.major << "."
       << (int) pBody.version.minor << " ";

  cout << (char*) pBody.host << " " << pBody.port << " ";

  unsigned long j;
  if (hexflag)
    {
      // Output key in hexadecimal form.
      
      cout << "0x";
      for (j=0; j < (pBody.object_key).length(); j++) {
	write_char((pBody.object_key)[j],1);
      }

      cout << "  (" << (pBody.object_key).length() 
	   << " bytes)" << endl;
    }
  else
    {
      // Output key as text
      
      cout << "\"";
      for(j=0; j < (pBody.object_key).length(); j++) {
	write_char((pBody.object_key)[j],0);
      }
      cout << "\"" << endl;
    }

  for (j=0; j < pBody.components.length(); j++) {
    decodeIIOPComponents(pBody.components[j]);
  }
}

//////////////////////////////////////////////////////////////////////////
static
void
EncapStreamToProfile(const _CORBA_Unbounded_Sequence_Octet &profile,
			   IIOP::ProfileBody &body)
{
  cdrEncapsulationStream s(profile.get_buffer(),profile.length(),1);
  
  body.version.major <<= s;
  body.version.minor <<= s;
  
  if (body.version.major != 1) {
    cerr << "Cannot decode IIOP profile with version no. = " 
	 << body.version.major << endl;
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  }

  body.host <<= s;

  body.port <<= s;

  body.object_key <<= s;

  switch (body.version.minor) {
  case 0:
    // Check if the profile body ends here.
    if (s.checkInputOverrun(1,1))
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    break;
  default:
    {
      body.components <<= s;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
static void decode_TAG_FIREWALL_TRANS(const _CORBA_Unbounded_Sequence_Octet&);
static void decode_TAG_ORB_TYPE(const _CORBA_Unbounded_Sequence_Octet&);
static void decode_TAG_ALTERNATE_IIOP_ADDRESS(const _CORBA_Unbounded_Sequence_Octet&);
static void decode_TAG_SSL_SEC_TRANS(const _CORBA_Unbounded_Sequence_Octet&);

static struct {
  IOP::ComponentId id;
  void (*fn)(const _CORBA_Unbounded_Sequence_Octet& data);
} componentDecoders[] = {
  // This table must be arranged in ascending order of IOP::ComponentId
  { IOP::TAG_ORB_TYPE,  decode_TAG_ORB_TYPE },
  { IOP::TAG_CODE_SETS, 0 },
  { IOP::TAG_POLICIES, 0 },
  { IOP::TAG_ALTERNATE_IIOP_ADDRESS, decode_TAG_ALTERNATE_IIOP_ADDRESS },
  { IOP::TAG_COMPLETE_OBJECT_KEY, 0 },
  { IOP::TAG_ENDPOINT_ID_POSITION, 0 },
  { IOP::TAG_LOCATION_POLICY, 0 },
  { IOP::TAG_ASSOCIATION_OPTIONS, 0 },
  { IOP::TAG_SEC_NAME, 0 },
  { IOP::TAG_SPKM_1_SEC_MECH, 0 },
  { IOP::TAG_SPKM_2_SEC_MECH, 0 },
  { IOP::TAG_KERBEROSV5_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_SECRET_SEC_MECH, 0 },
  { IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH, 0 },
  { IOP::TAG_SSL_SEC_TRANS, decode_TAG_SSL_SEC_TRANS },
  { IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH, 0 },
  { IOP::TAG_GENERIC_SEC_MECH, 0 },
  { IOP::TAG_FIREWALL_TRANS, decode_TAG_FIREWALL_TRANS },
  { IOP::TAG_SCCP_CONTACT_INFO, 0 },
  { IOP::TAG_JAVA_CODEBASE, 0 },
  { IOP::TAG_DCE_STRING_BINDING, 0 },
  { IOP::TAG_DCE_BINDING_NAME, 0 },
  { IOP::TAG_DCE_NO_PIPES, 0 },
  { IOP::TAG_DCE_SEC_MECH, 0 },
  { IOP::TAG_INET_SEC_TRANS, 0 },
  { 0xffffffff, 0 }
};

//////////////////////////////////////////////////////////////////////////
static
void
decodeIIOPComponents(IOP::TaggedComponent& component) {

  static int tablesize = 0;

  if (!tablesize) {
    while (componentDecoders[tablesize].id != 0xffffffff) tablesize++;
  }

  int top = tablesize;
  int bottom = 0;

  cout << "        ";

  do {
    int index = (top + bottom) >> 1;
    IOP::ComponentId id = componentDecoders[index].id;
    if (id == component.tag) {
      if (componentDecoders[index].fn) {
	componentDecoders[index].fn(component.component_data);
	return;
      }
      break;
    }
    else if (id > component.tag) {
      top = index;
    }
    else {
      bottom = index + 1;
    }
  } while (top != bottom);

  // Default is to dump this tag uninterpreted
  {
    const char* tagname = IOP::ComponentIDtoName(component.tag);
    if (tagname)
      cout << tagname << " ";
    else
      cout << "UNKNOWN TAG(" << component.tag << ") ";

    cout << "0x";
    for (CORBA::ULong j=0; j < component.component_data.length(); j++) {
      write_char(component.component_data[j],1);
    }

    cout << "  (" << component.component_data.length() 
	 << " bytes)";
  }
  cout << endl;
}

//////////////////////////////////////////////////////////////////////////
static
void
decode_TAG_ORB_TYPE(const _CORBA_Unbounded_Sequence_Octet& data)
{
  cout << "TAG_ORB_TYPE ";
  cdrEncapsulationStream s(data.get_buffer(),data.length(),1);
  CORBA::ULong orb_type;
  orb_type <<= s;
  if (orb_type == omniORB_TAG_ORB_TYPE) {
    cout << "omniORB";
  }
  else {
    if (hexflag) {
      cout << "0x" << hex << orb_type << dec;
    }
    else {
      char v;
      v = ((orb_type & 0xff000000) >> 24);
      write_char(v,0); 
      v = ((orb_type & 0x00ff0000) >> 16); 
      write_char(v,0); 
      v = ((orb_type & 0x0000ff00) >> 8);
      write_char(v,0);
      v = (orb_type & 0x000000ff);
      write_char(v,0);
    }
  }
  cout << endl;

}

//////////////////////////////////////////////////////////////////////////
static
void
decode_TAG_ALTERNATE_IIOP_ADDRESS(const _CORBA_Unbounded_Sequence_Octet& data)
{
  cout << "TAG_ALTERNATE_IIOP_ADDRESS ";
  cdrEncapsulationStream s(data.get_buffer(),data.length(),1);
  CORBA::String_member host;
  CORBA::UShort        port;
  host <<= s;
  port <<= s;
  cout << (const char*) host << " " << port << endl;
}


//////////////////////////////////////////////////////////////////////////
static
void
decode_TAG_SSL_SEC_TRANS(const _CORBA_Unbounded_Sequence_Octet& data)
{
  cout << "TAG_SSL_SEC_TRANS ";
  cdrEncapsulationStream s(data.get_buffer(),data.length(),1);
  CORBA::UShort v;
  v <<= s;
  cout << "target_supports 0x" << hex << v << dec << " ";
  v <<= s;
  cout << "target_requires 0x" << hex << v << dec << " ";
  v <<= s;
  cout << "port " << v << endl;
}

//////////////////////////////////////////////////////////////////////////
static
void
decode_TAG_FIREWALL_TRANS(const _CORBA_Unbounded_Sequence_Octet& data)
{
  cout << "TAG_FIREWALL_TRANS ";
  cdrEncapsulationStream s(data.get_buffer(),data.length(),1);

  CORBA::ULong total;
  total <<= s;

  for (CORBA::ULong i=0; i < total; i++) {
    CORBA::ULong tag;
    tag <<= s;
    _CORBA_Unbounded_Sequence_Octet pdata;
    pdata <<= s;
    cdrEncapsulationStream profile(pdata.get_buffer(),pdata.length(),1);
    switch (tag) {
    case 0:  // FW_MECH_PROXY
      {
	cout << "| GIOP Proxy ";
	CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(profile);
	CORBA::String_var str = omni::objectToString(obj->PR_getobj());
	cout << (const char*) str;
      }
      break;
    case 1:  // FW_MECH_TCP
      {
	cout << "TCP ";
	CORBA::String_member host;
	CORBA::UShort        port;
	host <<= s;
	port <<= s;
	cout << (const char*) host << " " << port << " ";
	IOP::MultipleComponentProfile components;
	components <<= s;
	if (components.length() > 1) {
	  cout << "??spurious tagged components??";
	}
	for (CORBA::ULong index = 0; index < components.length(); index++) {
	  if (components[index].tag == IOP::TAG_SSL_SEC_TRANS) {
	    decodeIIOPComponents(components[index]);
	  }
	}
      }
      break;
    case 2:  // FW_MECH_SOCKSV5
      {
	cout << "SOCKS ";
	cout << "0x";
	for (CORBA::ULong j=0; j < pdata.length(); j++) {
	  write_char(pdata[j],1);
	}

	cout << "  (" << pdata.length() 
	     << " bytes)";
      }
      break;
    default:
      cout << "Unknown Firewall Profile (" << tag << ")";
      break;

    }
    cout << " |";
  }

  cout << endl;
}
