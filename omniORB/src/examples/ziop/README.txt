Experimental ZIOP support
=========================

omniORB has experimental support for ZIOP, which compresses
transmitted messages.

To enable ZIOP on Unix platforms, give an argument of --enable-ziop to
the configure script.

To enable it on Windows, set the EnableZIOP make variable in the
platform configuration file and define OMNIORB_ENABLE_ZIOP in
include/omniORB4/CORBA_sysdep_trad.h.


omniORB has an almost complete implementation of the ZIOP
specification, with the following differences:

1. To avoid a dependency on CORBA::Any, compression policies are
   obtained with functions in the omniZIOP namespace, rather than with
   the standard orb->create_policy(). See include/omniORB4/omniZIOP.h
   for details.

2. Client-side policies are global, set with omniZIOP::setGlobalPolicies().
   CORBA::Object::_set_policy_overrides() is not supported.

3. POAs can be given ZIOP policies as shown in ziop_impl.cc, but they
   can also use the global policies set with omniZIOP::setGlobalPolicies().
   This is useful to apply ZIOP policies to the RootPOA or omniINSPOA.


In addition to the standard policies, whether or not to enable ZIOP is
determined by client and server transport rules. For a client to use
ZIOP, the matching client transport rule must include "ziop";
similarly, for a server to use ZIOP, the matching server transport
rule must include "ziop". e.g. to use the examples:

  ziop_impl -ORBserverTransportRule "* unix,ssl,tcp,ziop"

  ziop_clt -ORBclientTransportRule "* unix,ssl,tcp,ziop" IOR:...

This allows you to enable ZIOP for WAN links, but disable it for LAN
communication, for example.


The fact that a server supports ZIOP is encoded in its IORs. This
means that if a client uses a corbaloc URI to reference an object, the
object reference does not contain ZIOP details, and thus the
communication cannot use ZIOP. If a client is absolutely certain that
a server supports ZIOP, it can extend an object reference with ZIOP
details using omniZIOP::setServerPolicies(). Using the new object
reference, the client will be able to make ZIOP calls.

Creating a ZIOP-enabling object reference in this way is dangerous!
If the server does not actually support ZIOP, it will receive
compressed messages that it cannot handle. A well-behaved server will
throw a CORBA::MARSHAL exception in response.
