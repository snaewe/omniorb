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
#include <iomanip.h>
#include <stdlib.h>

#include <omniORB4/CORBA.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

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



static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] [-o] <stringified IOR>" << endl;
  cerr << "  flag:" << endl;
  cerr << "     -x print object key as a hexadecimal value." << endl;
  cerr << "     -o print extra info for omniORB IOR." << endl;
}


#ifdef HAVE_GETOPT

extern char* optarg;
extern int optind;

#else

// WIN32 doesn't have an implementation of getopt() -
// supply a getopt() for this program:

char* optarg;
int optind = 1;

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

#endif  // !HAVE_GETOPT


#define POA_NAME_SEP            '\xff'
#define TRANSIENT_SUFFIX_SEP    '\xfe'
#define TRANSIENT_SUFFIX_SIZE   8


static void
print_key(_CORBA_Unbounded_Sequence_Octet& key, int hexflag)
{
  if (hexflag) {
    // Output key in hexadecimal form.

    cout << "0x";

    for( unsigned j = 0; j < key.length(); j++ ) {
      int v = (key[j] & 0xf0) >> 4;

      if (v < 10)
	cout << (char)('0' + v);
      else
	cout << (char)('a' + (v - 10));

      v = key[j] & 0xf;

      if (v < 10)
	cout << (char)('0' + v);
      else
	cout << (char)('a' + (v - 10));
    }

    cout << "  (" << key.length()
	 << " bytes)" << endl;
  }
  else {
    // Output key as text

    cout << "\"";

    for( unsigned j = 0; j < key.length(); j++ ) {
      if( (char) key[j] >= ' ' &&
	  (char) key[j] <= '~')
	cout << (char) key[j];
      else
	cout << ".";
    }
    cout << "\"" << endl;
  }
}


static int
get_poa_info(_CORBA_Unbounded_Sequence_Octet& key,
	     _CORBA_Unbounded_Sequence_String& poas_out,
	     int& transient_out,
	     _CORBA_Unbounded_Sequence_Octet& id_out)
{
  const char* k = (const char*) key.NP_data();
  int len = key.length();
  const char* kend = k + len;

  poas_out.length(1);
  poas_out[0] = CORBA::string_dup("root");

  if( *k != TRANSIENT_SUFFIX_SEP && *k != POA_NAME_SEP )  return 0;

  while( k < kend && *k == POA_NAME_SEP ) {

    k++;
    const char* name = k;

    while( k < kend && *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      k++;

    if( k == kend )  return 0;

    char* nm = new char[k - name + 1];
    memcpy(nm, name, k - name);
    nm[k - name] = '\0';
    poas_out.length(poas_out.length() + 1);
    poas_out[poas_out.length() - 1] = nm;
  }

  if( k == kend )  return 0;

  transient_out = 0;
  if( *k == TRANSIENT_SUFFIX_SEP ) {
    transient_out = 1;
    k += TRANSIENT_SUFFIX_SIZE + 1;
  }
  if( k >= kend || *k )  return 0;
  k++;

  id_out.length(kend - k);
  memcpy(id_out.NP_data(), k, kend - k);

  return 1;
}


static void
print_omni_key(_CORBA_Unbounded_Sequence_Octet& key, int hexflag)
{
  _CORBA_Unbounded_Sequence_String poas;
  int is_transient;
  _CORBA_Unbounded_Sequence_Octet id;

  if( get_poa_info(key, poas, is_transient, id) ) {
    cout << "POA(" << (char*)poas[0];
    for( unsigned i = 1; i < poas.length(); i++ )
      cout << '/' << (char*)poas[i];
    cout << ") ";
  }
  else {
    if( key.length() != sizeof(omniOrbBoaKey) ) {
      cout << "Object key not recognised as from omniORB." << endl;
      exit(-1);
    }
    cout << "BOA ";
  }

  print_key(id, hexflag);
}

static
void
print_tagged_components(IOP::MultipleComponentProfile& components)
{
  CORBA::ULong total = components.length();
  
  for (CORBA::ULong index=0; index < total; index++) {
    CORBA::String_var content;
    content = IOP::dumpComponent(components[index]);
    char* p = content;
    char* q;
    do {
      q = strchr(p,'\n');
      if (q) {
	*q++ = '\0';
      }
      cout << "            " << (const char*) p << endl;
      p = q;
    } while (q);
  }
}

static
void
toIOR(const char* iorstr,IOP::IOR& ior)
{
  size_t s = (iorstr ? strlen(iorstr) : 0);
  if (s<4)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  const char *p = iorstr;
  if (p[0] != 'I' ||
      p[1] != 'O' ||
      p[2] != 'R' ||
      p[3] != ':')
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  cdrMemoryStream buf((CORBA::ULong)s,0);

  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v;
    
    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    buf.marshalOctet(v);
  }

  buf.rewindInputPtr();
  CORBA::Boolean b = buf.unmarshalBoolean();
  buf.setByteSwapFlag(b);

  ior.type_id = IOP::IOR::unmarshaltype_id(buf);
  ior.profiles <<= buf;
}


int main(int argc, char* argv[])
{
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }


  // Get options:

  int c;
  int hexflag = 0;
  int omniflag = 0;

  while((c = getopt(argc,argv,"xo")) != EOF) {
    switch(c) {
    case 'x':
      hexflag = 1;
      break;
    case 'o':
      omniflag = 1;
      break;
    case '?':
    case 'h':
      {
	usage(argv[0]);
	return 1;
      }
    }
  }

  if (optind >= argc) {
    usage(argv[0]);
    return 1;
  }


  const char* str_ior = argv[optind];

  CORBA::ORB_var orb;
  argc = 0;
  orb = CORBA::ORB_init(argc,0,"omniORB4");

  IOP::IOR ior;

  try {

    toIOR(str_ior,ior);

    if (ior.profiles.length() == 0 && strlen(ior.type_id) == 0) {
      cout << "IOR is a nil object reference." << endl;
    }
    else {
      cout << "Type ID: \"" << (const char*) ior.type_id << "\"" << endl;
      cout << "Profiles:" << endl;

      for (unsigned long count=0; count < ior.profiles.length(); count++) {

	cout << count+1 << ". ";

	if (ior.profiles[count].tag == IOP::TAG_INTERNET_IOP) {

	  IIOP::ProfileBody pBody;
	  IIOP::unmarshalProfile(ior.profiles[count],pBody);

	  cout << "IIOP " << (int) pBody.version.major << "."
	       << (int) pBody.version.minor << " ";
	  cout << (const char*) pBody.address.host 
	       << " " << pBody.address.port << " ";

	  unsigned long j;

	  if( omniflag )
	    print_omni_key(pBody.object_key, hexflag);
	  else
	    print_key(pBody.object_key, hexflag);

	  print_tagged_components(pBody.components);

	  cout << endl;
	}
	else if (ior.profiles[count].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	  
	  cout << "Multiple Component Profile ";
	  IIOP::ProfileBody pBody;
	  IIOP::unmarshalMultiComponentProfile(ior.profiles[count],
					       pBody.components);
	  print_tagged_components(pBody.components);

	  cout << endl;
	  
	}
	else {
	  cout << "Unrecognised profile tag: 0x"
	       << hex << (unsigned)(ior.profiles[count].tag)
	       << endl;
	}
      }
    }
  }
  catch(CORBA::MARSHAL& ex) {
    cerr << "\nInvalid stringified IOR supplied." << endl;
    const char* ms = ex.NP_minorString();
    if (ms)
      cerr << "(CORBA::MARSHAL: minor = " << ms << ")" << endl;
    else
      cerr << "(CORBA::MARSHAL: minor = 0x" << hex << ex.minor() << ")"
	   << endl;
    return 1;
  }
  catch(...) {
    cerr << "\nException while processing stringified IOR." << endl;
    return 1;
  }

  orb->destroy();
  return 0;
}
