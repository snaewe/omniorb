// -*- Mode: C++; -*-
//                          Package   : genior
// genior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of genior.
//
//  Genior is free software; you can redistribute it and/or modify
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
//
// Program to generate IORs


#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __WIN32__
#include <unistd.h>
#endif

#include <omniORB4/CORBA.h>
#include <ctype.h>


static GIOP::Version giop_version = { 1,2 };
static CORBA::Boolean extra_profiles = 1;

static char* genRef(const char* IRTypeId, const char* hostname, 
		    int port,  const char* objectid, int hex);


static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] <IR Type ID> <hostname> <port> [object key]" << endl;
  cerr << "         flag:" << endl;
  cerr << "               -x interpret object key as a hexadecimal value." << endl;
}




#if defined(__WIN32__) || defined(__VMS) && __VMS_VER < 60200000

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


#if !defined(__WIN32__)
extern char* optarg;
extern int optind;
#endif


int main(int argc, char* argv[])
{
  if (argc < 4) {
    usage(argv[0]);
    return 1;
  }	

  // Get options:

  int c;
  int hexflag = 0;

  while((c = getopt(argc,argv,"x")) != EOF) {
    switch(c) {
    case 'x':
      hexflag = 1;
      break;
    case '?':
      {
	usage(argv[0]);
	return 1;
      }
    }
  }
	  

  int port;
  CORBA::String_var IRTypeId, hostname, objKey;

  if (hexflag) {
    if (argc < 6) {
      usage(argv[0]);
      return 1;
    }
    IRTypeId = CORBA::string_dup(argv[2]);
    hostname = CORBA::string_dup(argv[3]);
    port = atoi(argv[4]);
    objKey = CORBA::string_dup(argv[5]);
  }
  else {
    if (argc < 4) {
      usage(argv[0]);
      return 1;
    }
    IRTypeId = CORBA::string_dup(argv[1]);
    hostname = CORBA::string_dup(argv[2]);
    port = atoi(argv[3]);
    
    if (argc >= 5)
      objKey = CORBA::string_dup(argv[4]);
  }

  if (port == 0) {
    cerr << "Port number is invalid." << endl;
    return 1;
  }

  CORBA::String_var ior;
  try {
    ior = genRef(IRTypeId, hostname, port, objKey, hexflag);
  }
  catch(...) {
    cerr << "Unexpected exception caught. Abort." << endl;
    return 1;
  }
  if ((const char*)ior) {
    cout << (const char*) ior << endl;
    return 0;
  }
  else {
    return 1;
  }
}


static
char*
genRef(const char* IRTypeId, const char* hostname, int port, 
       const char* objKey, int hex)
{
  CORBA::ORB_var orb;
  int argc = 0;
  
  orb = CORBA::ORB_init(argc,0,"omniORB4");

  IIOP::Address addrs;
  addrs.host = hostname;
  addrs.port = port;

  omniORB::seqOctets* keySeed;

  if (objKey == 0)
    {
      // No key specified - generate one.
      omniORB::objectKey k;
      omniORB::generateNewKey(k);
      keySeed = omniORB::keyToOctetSequence(k);
    }
  else {
    // Use key given.

    if (!hex) {
      // Key is a text value.
      keySeed = new omniORB::seqOctets;
      keySeed->length(strlen(objKey));
      unsigned int j;

      for (j=0; j<strlen(objKey); j++)
	(*keySeed)[j] = (CORBA::Octet) objKey[j];
    }
    else {
      // Interpret key as hexadecimal
      if ((objKey[0] != '0' || objKey[1] !='x')) {
	cerr << "A hexadecimal key should be prefixed by 0x" << endl;
	return 0;
      }
      else if ( (strlen(objKey)%2) != 0) {
	cerr << "Hexadecimal key is corrupted." << endl;
	return 0;
      }
	
      keySeed = new omniORB::seqOctets;
      keySeed->length((strlen(objKey)/2)-1);
      unsigned int j;

      for (j=2; j < strlen(objKey); j+=2) {
	char curr = (int) tolower(objKey[j]);
	int hi,lo;
	
	if (curr >= '0' && curr <= '9')
	  hi = ((curr - 48) << 4);
	else if (curr >= 'a' && curr <='f')
	  hi = ((curr - 87) << 4);
	else {
	  cerr << "Hexadecimal key is corrupted." << endl;
	  return 0;
	}
	curr = tolower(objKey[j+1]);
	if (curr >= '0' && curr <= '9')
	  lo = curr - 48;
	else if (curr >= 'a' && curr <='f')
	  lo = curr - 87;
	else {
	  cerr << "Hexadecimal key is corrupted." << endl;
	  return 0;
	}
	(*keySeed)[(j/2)-1] = (CORBA::Octet) (hi + lo);
      }
    }
  }
  
  omniIOR* ior = new omniIOR(IRTypeId,*keySeed,&addrs,1,giop_version,extra_profiles);
  omniObjRef* objref = omni::createObjRef(CORBA::Object::_PD_repoId,ior,0);
  
  CORBA::String_var result;
  result = omniObjRef::_toString(objref);
  
  return result._retn();
}  
  
