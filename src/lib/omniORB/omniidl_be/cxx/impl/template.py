# -*- python -*-
#                           Package   : omniidl
# template.py               Created on: 2000/02/13
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   Example interface implementation templates

# $Id$
# $Log$
# Revision 1.5.2.3  2005/06/08 09:40:39  dgrisby
# Update example code, IDL dumping.
#
# Revision 1.5.2.2  2005/01/06 23:10:07  dgrisby
# Big merge from omni4_0_develop.
#
# Revision 1.5.2.1  2003/03/23 21:02:35  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.3.2.3  2002/10/20 20:17:11  dgrisby
# Fix reference to omniORB 3.
#
# Revision 1.3.2.2  2000/10/12 15:37:52  sll
# Updated from omni3_1_develop.
#
# Revision 1.4  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.1.2.2  2000/05/16 11:16:01  djs
# Updated to simplify memory management, correct errors in function prototypes,
# add missing attribute functions and generate #warnings which the user should
# remove when they fill in the gaps in the output.
#
# Revision 1.1.2.1  2000/02/18 23:01:25  djs
# Updated example implementation code generating module
#
# Revision 1.1  2000/02/13 15:54:15  djs
# Beginnings of code to generate example interface implementations
#

## Example code to actually implement an interface
##
interface_def = """\
//
// Example class implementing IDL interface @fq_name@
//
class @impl_fqname@: public @fq_POA_name@ {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  //virtual ~@impl_name@();
public:
  // standard constructor
  @impl_name@();
  virtual ~@impl_name@();

  // methods corresponding to defined IDL attributes and operations
  @operations@
};
"""

interface_code = """\
//
// Example implementational code for IDL interface @fqname@
//
@impl_fqname@::@impl_name@(){
  // add extra constructor code here
}
@impl_fqname@::~@impl_name@(){
  // add extra destructor code here
}
//   Methods corresponding to IDL attributes and operations
@operations@

// End of example implementational code
"""

interface_ior = """\
{
  // IDL interface: @fqname@
  CORBA::Object_var ref = @inst_name@->_this();
  CORBA::String_var sior(orb->object_to_string(ref));
  std::cout << "IDL object @fqname@ IOR = '" << (char*)sior << "'" << std::endl;
}
"""

main = """\
//
// Example code for implementing IDL interfaces in file @file@
//

#include <iostream>
#include <@idl_hh@>

@interfaces@

int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    @allocate_objects@

    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    @activate_objects@

    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    @output_references@

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  return 0;
}
"""
