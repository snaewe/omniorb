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



static CORBA::Char* convertRef(char* old_ior, char* hostname);

static void usage(char* progname)
{
  cerr << "usage: " << progname << " <stringified IOR> <new hostname>" << endl;
}



#ifdef __NT__

// NT doesn't have an implementation of getopt() - 
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

#ifndef __NT__
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
		  
		    IIOP::EncapStreamToProfile(
			  (*old_profiles)[count].profile_data,pBody);
		    pBody.host = (CORBA::Char*) hostname;

		    IIOP::profileToEncapStream(pBody,p.profile_data);
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
