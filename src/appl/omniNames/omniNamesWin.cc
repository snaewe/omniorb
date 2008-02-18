// -*- Mode: C++; -*-
//                          Package   : omniNames
// omniNames.h              Author    : Duncan Grisby
//
//    Copyright (C) 2007 Apasphere Ltd
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
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

#ifdef __WIN32__

#include "omniNames.h"

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

static const TCHAR* WIN32_DISP_NAME = TEXT("omniNames CORBA naming service");
static const TCHAR* WIN32_SVC_NAME  = TEXT("omninames");
static const TCHAR* WIN32_SVC_DESC  = TEXT("omniORB's implementation of the CORBA naming service");
static const TCHAR* REGISTRY_KEY    = TEXT("SOFTWARE\\omniNames");

// Service control functions
void WINAPI win32_svc_dispatch(DWORD, LPTSTR*);
VOID WINAPI win32_svc_ctrl(DWORD);
BOOL 	    win32_report_status(DWORD, DWORD, DWORD);

// Helper functions
static LPTSTR GetLastErrorText(LPTSTR, DWORD);

// Static data
static omniNames*     	     names;
static SERVICE_STATUS 	     s_status;
static SERVICE_STATUS_HANDLE h_status = NULL;


// Helpful registry functions
static
CORBA::Boolean
setRegString(HKEY rootkey, const char* subkey, const char* val)
{
  if (RegSetValueExA(rootkey, subkey, 0, REG_SZ,
		     (const BYTE*)val, strlen(val)) != ERROR_SUCCESS) {
    TCHAR err_msg[512];
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Registry error: " << err_msg << endl;
    return 0;
  }
  return 1;
}

static
CORBA::Boolean
setRegLong(HKEY rootkey, const char* subkey, CORBA::Long val)
{
  if (RegSetValueEx(rootkey, subkey, 0, REG_DWORD,
		    (const BYTE*)&val, sizeof(val)) != ERROR_SUCCESS) {
    TCHAR err_msg[512];
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Registry error: " << err_msg << endl;
    return 0;
  }
  return 1;
}

static
CORBA::Boolean
setRegArgs(HKEY rootkey, const char* subkey, int argc, char** argv)
{
  int len = 0;
  int i;
  for (i=0; i < argc; ++i) {
    len += strlen(argv[i]) + 1;
  }
  CORBA::String_var val = CORBA::string_alloc(len);
  char* ptr = (char*)val;

  for (i=0; i < argc; ++i) {
    strcpy(ptr, argv[i]);
    ptr += strlen(argv[i]) + 1;
  }
  *ptr = '\0';

  if (RegSetValueExA(rootkey, subkey, 0, REG_MULTI_SZ,
		     (const BYTE*)(char*)val, len+1) != ERROR_SUCCESS) {
    TCHAR err_msg[512];
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Registry error: " << err_msg << endl;
    return 0;
  }
  return 1;
}

static
char*
getRegString(HKEY rootkey, const char* subkey)
{
  int size = 512;
  CORBA::String_var val;

  while (1) {
    val = CORBA::string_alloc(size);
    DWORD type, len = size;

    LONG result = RegQueryValueExA(rootkey, subkey, 0, &type,
				   (BYTE*)(char*)val, &len);

    if (result == ERROR_MORE_DATA) {
      size = len;
      continue;
    }
    if (result != ERROR_SUCCESS) {
      return 0;
    }
    if (type != REG_SZ) {
      return 0;
    }
    return val._retn();
  }
}

static
long
getRegLong(HKEY rootkey, const char* subkey)
{
  CORBA::Long val;
  DWORD type, len = sizeof(val);

  LONG result = RegQueryValueExA(rootkey, subkey, 0, &type,
				 (BYTE*)&val, &len);
  if (result != ERROR_SUCCESS) {
    return 0;
  }
  if (type != REG_DWORD) {
    return 0;
  }
  return val;
}

static
char*
getRegArgs(HKEY rootkey, const char* subkey, int& argc, char**& argv,
	   int a_argc, char** a_argv)
{
  int size = 1024;
  CORBA::String_var val;

  while (1) {
    val = CORBA::string_alloc(size);
    DWORD type, len = size;

    LONG result = RegQueryValueExA(rootkey, subkey, 0, &type,
				   (BYTE*)(char*)val, &len);

    if (result == ERROR_MORE_DATA) {
      size = len;
      continue;
    }
    if (result != ERROR_SUCCESS) {
      return 0;
    }
    if (type != REG_MULTI_SZ) {
      return 0;
    }
    break;
  }
  char* c = (char*)val;
  int slen;
  argc = a_argc;
  while (1) {
    slen = strlen(c);
    if (slen == 0)
      break;

    ++argc;
    c += slen + 1;
  }
  argv = new char*[argc];

  int i;

  for (i=0; i < a_argc; ++i) {
    argv[i] = a_argv[i];
  }

  c = (char*)val;
  for (; i < argc; ++i) {
    argv[i] = c;
    c += strlen(c) + 1;
  }
  return val._retn();
}


int
omniNames::
installService(int            port,
	       const char*    logdir,
	       const char*    errlog,
	       CORBA::Boolean ignoreport,
	       CORBA::Boolean nohostname,
	       CORBA::Boolean manual,
	       int argc, char** argv)
{
  DWORD err_code = NO_ERROR;
  TCHAR err_msg[512];

  TCHAR path[512];

  SC_HANDLE hservice = NULL;
  SC_HANDLE hmanager = NULL;

  DWORD path_result = GetModuleFileName(NULL, path, sizeof(path));

  if (path_result == 0 || path_result == sizeof(path)) {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Unable to install " << WIN32_DISP_NAME << ": " << err_msg << endl;
    return 2;
  }

  const char* svc_arg = " -runsvc -logdir c:\\names -errlog c:\\names\\err.log -start";
  CORBA::String_var path_arg = CORBA::string_alloc(path_result +
						   strlen(svc_arg));
  strcpy((char*)path_arg, path);
  strcat((char*)path_arg, svc_arg);
  
  {
    // Store configuration in registry
    HKEY rootkey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0, NULL, 0,
		       KEY_SET_VALUE | KEY_QUERY_VALUE,
		       NULL, &rootkey, NULL) != ERROR_SUCCESS) {

      GetLastErrorText(err_msg, sizeof(err_msg));
      cerr << "Unable to install " << WIN32_DISP_NAME << ": " << err_msg
	   << endl;
      return 2;
    }
    if (!setRegLong(rootkey, "port",       port))       return 2;
    if (!setRegLong(rootkey, "ignoreport", ignoreport)) return 2;
    if (!setRegLong(rootkey, "nohostname", nohostname)) return 2;

    if (logdir)
      if (!setRegString(rootkey, "logdir", logdir))
	return 2;

    if (errlog)
      if (!setRegString(rootkey, "errlog", errlog))
	return 2;

    if (!setRegArgs(rootkey, "args", argc, argv))
      return 2;

    RegCloseKey(rootkey);
  }

  hmanager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hmanager == NULL) {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Unable to open Service Control Manager: " << err_msg << endl;
    return 2;
  }

  DWORD start_type = manual ? SERVICE_DEMAND_START : SERVICE_AUTO_START;

  hservice = CreateServiceA(hmanager, WIN32_SVC_NAME, WIN32_DISP_NAME,
			    SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
			    start_type, SERVICE_ERROR_NORMAL,
			    (LPCTSTR)(char*)path_arg,
			    NULL, NULL, NULL, NULL, NULL);

  if (hservice) {
    // Set service description
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = (LPSTR)WIN32_SVC_DESC;
    if (!ChangeServiceConfig2(hservice, SERVICE_CONFIG_DESCRIPTION, &sd)) {
      GetLastErrorText(err_msg, sizeof(err_msg));
      cerr << "Error changing service description: " << err_msg << endl;
    }
    CloseServiceHandle(hservice);
  }

  CloseServiceHandle(hmanager);

  if (!hservice) {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Failed to create service: " << err_msg << endl;
    return 2;
  }

  cerr << WIN32_DISP_NAME << " installed." << endl;
  return 0;
}


int
omniNames::
removeService()
{
  DWORD err_code = NO_ERROR;
  TCHAR err_msg[512];

  SC_HANDLE hservice = NULL;
  SC_HANDLE hmanager = NULL;

  hmanager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hmanager == NULL) {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Unable to open Service Control Manager: " << err_msg << endl;
    return 2;
  }

  hservice = OpenService(hmanager, WIN32_SVC_NAME, SERVICE_ALL_ACCESS);
  if (hservice == NULL) {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Unable to open " << WIN32_DISP_NAME << " service: " << err_msg
	 << endl;
    CloseServiceHandle(hmanager);
    return 2;
  }

  // Try to stop the service
  SERVICE_STATUS status;
  if (ControlService(hservice, SERVICE_CONTROL_STOP, &status)) {
    cerr << "Stopping " << WIN32_DISP_NAME << "..." << flush;
    Sleep(1000);

    while (QueryServiceStatus(hservice, &status)) {
      if (status.dwCurrentState != SERVICE_STOP_PENDING)
	break;
      cerr << "." << flush;
      Sleep(1000);
    }
    cerr << endl;

    if (status.dwCurrentState == SERVICE_STOPPED)
      cerr << WIN32_DISP_NAME << " stopped." << endl;
    else
      cerr << WIN32_DISP_NAME << " failed to stop." << endl;
  }

  // now remove the service
  int return_status = 0;

  if (DeleteService(hservice)) {
    cerr << WIN32_DISP_NAME << " removed." << endl;
  }
  else {
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Failed to delete " << WIN32_DISP_NAME << ": " << err_msg << endl;
    return_status = 2;
  }

  CloseServiceHandle(hservice);
  CloseServiceHandle(hmanager);

  {
    // Clear registry
    if (RegDeleteKey(HKEY_LOCAL_MACHINE, REGISTRY_KEY) != ERROR_SUCCESS) {
      GetLastErrorText(err_msg, sizeof(err_msg));
      cerr << "Failed to clear registry key: " << err_msg << endl;
      return_status = 2;
    }
  }
  return return_status;
}

int
omniNames::
runService(int port, const char* logdir, const char* errlog,
	   CORBA::Boolean ignoreport, CORBA::Boolean nohostname,
	   int a_argc, char** a_argv)
{
  CORBA::String_var logdir_v;
  CORBA::String_var errlog_v;
  CORBA::String_var packed_args;
  int               argc = a_argc;
  char**            argv = a_argv;

  {
    HKEY rootkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_KEY, 0,
		     KEY_QUERY_VALUE, &rootkey) == ERROR_SUCCESS) {

      if (!port)
	port = getRegLong(rootkey, "port");

      if (!ignoreport)
	ignoreport = getRegLong(rootkey, "ignoreport");

      if (!nohostname)
	nohostname = getRegLong(rootkey, "nohostname");

      if (!logdir) {
	logdir   = getRegString(rootkey, "logdir");
	logdir_v = logdir;
      }
      if (!errlog) {
	errlog   = getRegString(rootkey, "errlog");
	errlog_v = errlog;
      }
      packed_args = getRegArgs(rootkey, "args", argc, argv, a_argc, a_argv);

      RegCloseKey(rootkey);
    }
  }

  SERVICE_TABLE_ENTRY dispatch[] = {
    { (LPTSTR)WIN32_SVC_NAME, (LPSERVICE_MAIN_FUNCTION)win32_svc_dispatch },
    { NULL, NULL }
  };

  names = new omniNames(port, logdir, errlog, ignoreport, nohostname, 1,
			argc, argv);
  if (argv != a_argv)
    delete [] argv;

  // Run the service here - call returns when service stopped
  if (StartServiceCtrlDispatcher(dispatch) == 0) {
    char err_msg[512];
    GetLastErrorText(err_msg, sizeof(err_msg));
    cerr << "Unable to start service control dispatcher: " << err_msg << endl;

    delete names;
    names = 0;
    return 1;
  }
  delete names;
  names = 0;
  return 0;
}


class NamesThread : public omni_thread
{
public:
  NamesThread()
  {
    start_undetached();
  }

  void* run_undetached(void*)
  {
    names->run();
    return 0;
  }
};



// Service dispatch function
void WINAPI win32_svc_dispatch(DWORD dargc, LPTSTR *largv)
{
  // register our service control handler:
  h_status = RegisterServiceCtrlHandler(WIN32_SVC_NAME, win32_svc_ctrl);

  if (h_status) {
    if (win32_report_status(SERVICE_START_PENDING, NO_ERROR, 5000)) {

      // Start main omniNames thread
      NamesThread* nthread = new NamesThread;

      if (!names->waitForStart(5)) {
	names->stop();
	win32_report_status(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
	return;
      }
      win32_report_status(SERVICE_RUNNING, NO_ERROR, 0);
      nthread->join(0);
      win32_report_status(SERVICE_STOPPED, NO_ERROR, 0);
    }
  }
  return;
}

// called by the SCM whenever ControlService() is called on this service.
VOID WINAPI win32_svc_ctrl(DWORD code)
{
  // Handle the requested control code.
  if (code == SERVICE_CONTROL_STOP) {
    win32_report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
    if (names)
      names->stop();
  }
  else {
    win32_report_status(s_status.dwCurrentState, NO_ERROR, 0);
  }
}

// Sets the current status of the service and reports it to the SCM
BOOL win32_report_status (DWORD state, DWORD exit_code, DWORD wait_hint)
{
  static DWORD checkpt = 0;

  s_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  s_status.dwServiceSpecificExitCode = 0;

  if (state == SERVICE_START_PENDING || state == SERVICE_STOPPED)
    s_status.dwControlsAccepted = 0;
  else
    s_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

  s_status.dwCurrentState  = state;
  s_status.dwWin32ExitCode = exit_code;
  s_status.dwWaitHint      = wait_hint;

  if (state == SERVICE_RUNNING || state == SERVICE_STOPPED)
    s_status.dwCheckPoint = 0;
  else
    s_status.dwCheckPoint = ++checkpt;

  // report the status of the service to the service control manager.
  BOOL result = SetServiceStatus(h_status, &s_status);

  return result;
}


LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
  DWORD dwRet;
  LPTSTR lpszTemp = NULL;

  dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL,
			GetLastError(),
			LANG_NEUTRAL,
			(LPTSTR)&lpszTemp,
			0,
			NULL);

  // supplied buffer is not long enough
  if (!dwRet || ((long)dwSize < (long)dwRet+14)) {
    lpszBuf[0] = TEXT('\0');
  }
  else {
    lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline
    sprintf(lpszBuf, "%s (0x%x)", lpszTemp, GetLastError());
  }

  if (lpszTemp)
    LocalFree((HLOCAL)lpszTemp);

  return lpszBuf;
}


#endif // __WIN32__
