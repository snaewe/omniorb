// genior.cc
//
// Program to generate IORs


#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __NT__
#include <unistd.h>
#endif

#include <omniORB2/CORBA.h>


static CORBA::Char*
genRef(char* IRTypeId, char* hostname, int port, char* objectid, int hex);


static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] <IR Type ID> <hostname> <port> [object key]" << endl;
  cerr << "         flag:" << endl;
  cerr << "               -x interpret object key as a hexidecimal value." << endl;
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
  if (argc < 4) 
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
	  // Interpret key as hexidecimal
	  if ((objKey[0] != '0' || objKey[1] !='x'))
	    {
	      cerr << "A hexidecimal key should be prefixed by 0x" << endl;
	      return 0;
	    }
	  else if ( (strlen(objKey)%2) != 0)
	    {
	      cerr << "Hexidecimal key is corrupted." << endl;
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
		  cerr << "Hexidecimal key is corrupted." << endl;
		  return 0;
		}
		 
	      curr = tolower(objKey[j+1]);
	      
	      if (curr >= '0' && curr <= '9')
		  lo = curr - 48;
	      else if (curr >= 'a' && curr <='f')
		  lo = curr - 87;
	      else
		{
		  cerr << "Hexidecimal key is corrupted." << endl;
		  return 0;
		}

	     (*keySeed)[(j/2)-1] = (CORBA::Octet) (hi + lo);
	    }
	}
    }
      

  profb.object_key = *keySeed;

  IOP::TaggedProfile p;
  p.tag = IOP::TAG_INTERNET_IOP;
  
  IIOP::profileToEncapStream(profb,p.profile_data);


  int intfname_len = 8+strlen(IRTypeId)+1;
  CORBA::Char* intfname = new CORBA::Char[intfname_len];
  sprintf((char*) intfname,"IDL:%s:%d.%d",IRTypeId,(int) IIOP::current_major, (int)
	  IIOP::current_minor);
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
  
