// -*- Mode: C++; -*-
//                              Package   : convertior
// convertior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of convertior.
//
//  Convertior is free software; you can redistribute it and/or modify
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
// Takes an IOR, hostname as arguments. Outputs IOR with new hostname.

#include <iostream.h>
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


static char* convertRef(const char* old_ior, const char* hostname);

static void usage(char* progname)
{
  cerr << "usage: " << progname << " <stringified IOR> <new hostname>" << endl;
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

#endif



int main(int argc, char* argv[])
{
  if (argc < 3) 
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

  CORBA::String_var oldior(CORBA::string_dup(argv[1]));
  CORBA::String_var newhost(CORBA::string_dup(argv[2]));

  CORBA::String_var newior; 
  newior = convertRef(oldior,newhost);

  if ((const char*)newior) {
    cout << (const char*) newior << endl;
    return 0;
  }
  else {
    return 1;
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

static
char* toString(IOP::IOR& ior)
{
  cdrMemoryStream buf(CORBA::ULong(0),1);
  buf.marshalBoolean(omni::myByteOrder);
  ior.type_id >>= buf;
  ior.profiles >>= buf;

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewindInputPtr();
  size_t s = buf.bufSize();
  CORBA::Char * data = (CORBA::Char *)buf.bufPtr();

  char *result = new char[4+s*2+1];
  result[4+s*2] = '\0';
  result[0] = 'I';
  result[1] = 'O';
  result[2] = 'R';
  result[3] = ':';
  for (int i=0; i < (int)s; i++) {
    int j = 4 + i*2;
    int v = (data[i] & 0xf0);
    v = v >> 4;
    if (v < 10)
      result[j] = '0' + v;
    else
      result[j] = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      result[j+1] = '0' + v;
    else
      result[j+1] = 'a' + (v - 10);
  }
  return result;
}

static
char* convertRef(const char* old_ior, const char* hostname)
{
  try {
    IOP::IOR ior;

    toIOR(old_ior,ior);

    for (unsigned long count=0; count < ior.profiles.length(); count++) {

      if (ior.profiles[count].tag == IOP::TAG_INTERNET_IOP) {
	IIOP::ProfileBody pBody;
	IIOP::unmarshalProfile(ior.profiles[count],pBody);
	pBody.address.host = hostname;
	IOP::TaggedProfile profile;
	IIOP::encodeProfile(pBody,profile);
	CORBA::ULong max = profile.profile_data.maximum();
	CORBA::ULong len = profile.profile_data.length();
	CORBA::Octet* buf = profile.profile_data.get_buffer(1);
	ior.profiles[count].profile_data.replace(max,len,buf,1);
      }
    }
    CORBA::String_var result = toString(ior);
    return result._retn();

  }
  catch(CORBA::MARSHAL& ex) {
    cerr << "Invalid stringified IOR supplied." << endl;
    cerr << "(Minor = " << ex.minor() << ")" << endl;
    return 0;
  }
  catch(...) {
    cerr << "Exception while processing stringified IOR." << endl;
    return 0;
  }

}
