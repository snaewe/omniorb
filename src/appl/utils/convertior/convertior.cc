// -*- Mode: C++; -*-
//                              Package   : convertior
// convertior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
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


static CORBA::Char* convertRef(char* old_ior, char* hostname);

static void usage(char* progname)
{
  cerr << "usage: " << progname << " <stringified IOR> <new hostname>" << endl;
}



#ifdef __WIN32__

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


static
void
EncapStreamToProfile(const _CORBA_Unbounded_Sequence_Octet &s,
			   IIOP::ProfileBody &p)
{
  CORBA::ULong begin = 0;
  CORBA::ULong end = 0;

  // s[0] - byteorder
  end += 1;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  CORBA::Boolean byteswap = ((s[begin] == omni::myByteOrder) ? 0 : 1);

  // s[1] - iiop_version.major
  // s[2] - iiop_version.minor
  begin = end;
  end = begin + 2;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  p.iiop_version.major = s[begin];
  p.iiop_version.minor = s[begin+1];
  if (p.iiop_version.major != IIOP::current_major ||
      p.iiop_version.minor != IIOP::current_minor)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  // s[3] - padding
  // s[4] - s[7] host string length
  begin = end + 1;
  end = begin + 4;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) s[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) s[begin]);
      len = Swap32(t);
    }

    // s[8] - s[8+len-1] host string
    begin = end;
    end = begin + len;
    if (s.length() <= end)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // Is this string null terminated?
    if (((char)s[end-1]) != '\0')
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    p.host = new CORBA::Char[len];
    if (!p.host)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    memcpy((void *)p.host,(void *)&(s[begin]),len);
  }
    
  // align to CORBA::UShort
  begin = (end + 1) & ~(1);
  // s[begin] port number
  end = begin + 2;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  if (!byteswap) {
    p.port = ((CORBA::UShort &) s[begin]);
  }
  else {
    CORBA::UShort t = ((CORBA::UShort &) s[begin]);
    p.port = Swap16(t);
  }

  // align to CORBA::ULong
  begin = (end + 3) & ~(3);
  // s[begin]  object key length
  end = begin + 4;
  if (s.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) s[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) s[begin]);
      len = Swap32(t);
    }

    begin = end;
    end = begin + len;
    if (s.length() < end)
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // extract object key
    p.object_key.length(len);
    memcpy((void *)&p.object_key[0],(void *)&(s[begin]),len);
  }
  return;
}

static
void
profileToEncapStream(IIOP::ProfileBody &p,
			   _CORBA_Unbounded_Sequence_Octet &s)
{
  CORBA::ULong hlen = strlen((const char *)p.host) + 1;
  CORBA::ULong klen = p.object_key.length();
  {
    // calculate the total size of the encapsulated stream
    CORBA::ULong total = 8 + hlen;        // first 4 bytes + aligned host
    total = ((total + 1) & ~(1)) + 2;     // aligned port value
    total = ((total + 3) & ~(3)) + 4 +	// aligned object key
      klen;
    
    s.length(total);
  }


  s[0] = omni::myByteOrder;
  s[1] = IIOP::current_major;
  s[2] = IIOP::current_minor;
  s[3] = 0;
  {
    CORBA::ULong &l = (CORBA::ULong &) s[4];
    l = hlen;
  }
  memcpy((void *)&(s[8]),(void *)p.host,hlen);
  CORBA::ULong idx = ((8 + hlen) + 1) & ~(1);
  {
    CORBA::UShort &l = (CORBA::UShort &) s[idx];
    l = p.port;
  }
  idx = ((idx + 2) + 3) & ~(3);
  {
    CORBA::ULong &l = (CORBA::ULong &) s [idx];
    l = klen;
  }
  idx += 4;
  memcpy((void *)&s[idx],(void *)&p.object_key[0],klen);
  return;
}



int main(int argc, char* argv[])
{
  if (argc < 3) 
    {
      usage(argv[0]);
      return 1;
    }	


  // Get options:

#ifndef __WIN32__
  extern char* optarg;
  extern int optind;
#endif

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

  char* oldior = strdup(argv[1]);
  char* newhost = strdup(argv[2]);



  CORBA::Char* newior = convertRef(oldior,newhost);
  
  free(oldior);
  free(newhost);

  if (newior == 0)
    {
      cerr << "Error creating new IOR." << endl;
      return -1;
    }
  else
    {
      cerr << (char*) newior << endl;
    }


  delete[] newior;
  return 1;
}  


CORBA::Char* convertRef(char* old_ior, char* hostname)
{
  CORBA::Char* repoID;
  IOP::TaggedProfileList* old_profiles;
  IOP::TaggedProfileList* new_profiles = new IOP::TaggedProfileList;

  try
    {
      IOP::EncapStrToIor((CORBA::Char*) old_ior, repoID, old_profiles);

      new_profiles->length(old_profiles->length());

      if (*repoID == '\0')
	{
	  cerr << "IOR is a nil object reference." << endl;
	  return 0;
	}
      else
	{	  
	  for (long count=0; count < old_profiles->length(); count++)
	    {
	      if ((*old_profiles)[count].tag == IOP::TAG_INTERNET_IOP)
		  {
		    IIOP::ProfileBody pBody;
		    IOP::TaggedProfile p;
		  
		    EncapStreamToProfile(
			  (*old_profiles)[count].profile_data,pBody);
		    pBody.host = (CORBA::Char*) hostname;

		    profileToEncapStream(pBody,p.profile_data);
		    p.tag = IOP::TAG_INTERNET_IOP;

		    (*new_profiles)[count] = p;
		  }
	      else if ((*old_profiles)[count].tag ==
		       IOP::TAG_MULTIPLE_COMPONENTS)
		  {
		    cout << "Multiple Component Tag - Unable to process." 
			 << endl;
		    return 0;
		  }
	      else
		{
		  cout << "Unrecognised profile tag: " 
		       << (int) ((*old_profiles)[count].tag) 
		       << endl;
		  return 0;
		}
	    }
	}
    }
  catch(CORBA::MARSHAL& ex)
    {
      cerr << "Invalid stringified IOR supplied." << endl;
      cerr << "(Minor = " << ex.minor() << ")" << endl;
      return 0;
    }
  catch(...)
    {
      cerr << "Exception while processing stringified IOR." << endl;
      return 0;
    }

      
  CORBA::Char* new_ior = IOP::iorToEncapStr(repoID,new_profiles);
  
  delete[] repoID;
  delete old_profiles;
  delete new_profiles;

  return new_ior;
}
