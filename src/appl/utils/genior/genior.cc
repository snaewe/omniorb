// -*- Mode: C++; -*-
//                          Package   : genior
// genior.cc                Author    : Eoin Carroll (ewc)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
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
#include <ctype.h>

#ifndef __WIN32__
#include <unistd.h>
#endif

#include <omniORB2/CORBA.h>


static CORBA::Char*
genRef(char* IRTypeId, char* hostname, int port, char* objectid, int hex);


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
  if (argc < 4) 
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
  int hexflag = 0;

  while((c = getopt(argc,argv,"x")) != EOF)
    {
      switch(c)
	{
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
  char *IRTypeId, *hostname, *objKey;

  if (hexflag) 
    {
      if (argc < 6)
	{
	  usage(argv[0]);
	  return 1;
	}
      IRTypeId = strdup(argv[2]);
      hostname = strdup(argv[3]);
      port = atoi(argv[4]);
      objKey = strdup(argv[5]);
    }
  else 
    {
      if (argc < 4)
	{
	  usage(argv[0]);
	  return 1;
	}

      IRTypeId = strdup(argv[1]);
      hostname = strdup(argv[2]);
      port = atoi(argv[3]);

      if (argc < 5) objKey = 0;
      else objKey = strdup(argv[4]);

    }

  if (port == 0)
    {
      cerr << "Port number is invalid." << endl;
      return -1;
    }


  CORBA::Char* ior = genRef(IRTypeId, hostname, port, objKey, hexflag);
  
  if (ior == 0)
    {
      cerr << "Error creating IOR." << endl;
      return -1;
    }
  else
    {
      cerr << (char*) ior << endl;
    }

  free(IRTypeId);
  free(hostname);
  if (objKey != 0)  free(objKey);

  delete[] ior;

  return 1;
}



CORBA::Char* genRef(char* IRTypeId, char* hostname, int port, char* objKey, 
	         int hex)
{
  
  IIOP::ProfileBody profb;

  profb.iiop_version.major = IIOP::current_major;
  profb.iiop_version.minor = IIOP::current_minor;
  profb.port = (CORBA::UShort) port;
  profb.host = (CORBA::Char*) hostname;

  omniORB::seqOctets* keySeed;

  if (objKey == 0)
    {
      // No key specified - generate one.
      omniORB::objectKey k;
      omniORB::generateNewKey(k);
      keySeed = omniORB::keyToOctetSequence(k);
    }
  else
    {
      // Use key given.

     
      if (!hex)
	{
	  // Key is a text value.
	  keySeed = new omniORB::seqOctets;
	  keySeed->length(strlen(objKey));
	  
	  unsigned int j;

	  for (j=0; j<strlen(objKey); j++)
	    (*keySeed)[j] = (CORBA::Octet) objKey[j];
	 
	}
      else
	{
	  // Interpret key as hexadecimal
	  if ((objKey[0] != '0' || objKey[1] !='x'))
	    {
	      cerr << "A hexadecimal key should be prefixed by 0x" << endl;
	      return 0;
	    }
	  else if ( (strlen(objKey)%2) != 0)
	    {
	      cerr << "Hexadecimal key is corrupted." << endl;
	      return 0;
	    }
	

	  keySeed = new omniORB::seqOctets;
	  keySeed->length((strlen(objKey)/2)-1);

	  unsigned int j;

	  for (j=2; j < strlen(objKey); j+=2)
	    {
	      char curr = (int) tolower(objKey[j]);
	      int hi,lo;

	      if (curr >= '0' && curr <= '9')
		hi = ((curr - 48) << 4);
	      else if (curr >= 'a' && curr <='f')
		hi = ((curr - 87) << 4);
	      else
		{
		  cerr << "Hexadecimal key is corrupted." << endl;
		  return 0;
		}
		 
	      curr = tolower(objKey[j+1]);
	      
	      if (curr >= '0' && curr <= '9')
		  lo = curr - 48;
	      else if (curr >= 'a' && curr <='f')
		  lo = curr - 87;
	      else
		{
		  cerr << "Hexadecimal key is corrupted." << endl;
		  return 0;
		}

	     (*keySeed)[(j/2)-1] = (CORBA::Octet) (hi + lo);
	    }
	}
    }
      

  profb.object_key = *keySeed;

  IOP::TaggedProfile p;
  p.tag = IOP::TAG_INTERNET_IOP;
  
  profileToEncapStream(profb,p.profile_data);


  int intfname_len = strlen(IRTypeId)+1;
  CORBA::Char* intfname = new CORBA::Char[intfname_len];
  sprintf((char*) intfname,"%s",IRTypeId);
  intfname[intfname_len-1]='\0';

  IOP::TaggedProfileList* pList = new IOP::TaggedProfileList;
  pList->length(1);
  (*pList)[0] = p;
  CORBA::Char* string_ior =  IOP::iorToEncapStr(intfname,pList);
  

  delete pList;
  delete keySeed;
  delete[] intfname;
	  
  return string_ior;
}  
  
