// catior.cc
//
// Lists contents of an IOR.

#include <iostream.h>
#include <stdlib.h>

#include <omniORB2/CORBA.h>



static void usage(char* progname)
{
  cerr << "usage: " << progname << " [-x] <stringified IOR>" << endl;
  cerr << "         flag:" << endl;
  cerr << "               -x print object key as a hexadecimal value." << endl;
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
  if (argc < 2) 
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

  CORBA::Char* repoID;
  IOP::TaggedProfileList* profiles;

  try
    {
      IOP::EncapStrToIor((CORBA::Char*) str_ior, repoID, profiles);
      if (*repoID == '\0')
	{
	  cerr << "IOR is a nil object reference." << endl;
	}
      else
	{
	  cerr << "Type ID: \"" << (char*) repoID << "\"" << endl;
	  cerr << "Profiles:" << endl;
	  
	  for (long count=0; count < profiles->length(); count++)
	    {
	      cout << count+1 << ". ";
	      
	      if ((*profiles)[count].tag == IOP::TAG_INTERNET_IOP)
		  {
		    IIOP::ProfileBody pBody;
		    IIOP::EncapStreamToProfile((*profiles)[count].profile_data,pBody);
		    cout << "IIOP " << (int) pBody.iiop_version.major << "."
			 << (int) pBody.iiop_version.minor << " ";
		    cout << (char*) pBody.host << " " << pBody.port << " ";


		    unsigned long j;
		    if (hexflag)
		      {
			// Output key in hexidecimal form.
			
			cout << "0x";
			

			for (j=0; j < (pBody.object_key).length(); j++) 
			  {
			    int v = (((pBody.object_key)[j] & 0xf0) >> 4);

			    if (v < 10)
			      cout << (char)('0' + v);
			    else
			      cout << (char)('a' + (v - 10));

			    v = (((pBody.object_key)[j] & 0xf));

			    if (v < 10)
			      cout << (char)('0' + v);
			    else
			      cout << (char)('a' + (v - 10));
			  }

			cout << "  (" << (pBody.object_key).length() 
			     << " bytes)" << endl;
		      }
		    else
		      {
			  // Output key as text

			  cout << "\"";
			  
			  for(j=0; j < (pBody.object_key).length(); j++)
			    {
				if ((char) ((pBody.object_key)[j]) >= ' ' &&
				    (char) ((pBody.object_key)[j]) <= '~')
				         cout << (char)((pBody.object_key)[j]);
				else
				    cout << ".";
			    }
			  cout << "\"" << endl;
		      }
		  }
	      else if ((*profiles)[count].tag == IOP::TAG_MULTIPLE_COMPONENTS)
		  {
		    cout << "Multiple Component Tag" << endl;
		  }
	      else
		  {
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
  catch(...)
    {
      cerr << "Exception while processing stringified IOR." << endl;
      return -1;
    }

      
  
  delete[] repoID;
  delete profiles;
  return 1;
}
