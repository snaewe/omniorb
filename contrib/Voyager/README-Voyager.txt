NOTES ON cs-voyager.cc
======================

cs-voyager.cc is an extension to omniORB 4 to support the wstring
marshalling format used by Voyager, from Recursion Software. This file
explains what the extension does and how it does it.

Background
----------

First some background. The wstring and wchar types were a new feature
in version 2.1 of the CORBA specification. At that time, GIOP version
1.1 was specified, with new features to support the wide character
types. GIOP version 1.2 further clarified and extended the marshalling
format for wstring and wchar.

GIOP 1.1 and 1.2 support a notion of code set "negotiation", where the
client picks a code set to use for wstring transmission (and one for
string transmission too), based on a list of code sets offered by the
server. The chosen code set is communicated in a service context in
the GIOP request the first time a connection is used.

omniORB 4 supports pluggable code set implementations, using the
interface defined in include/omniORB4/codeSets.h. Each code set
implementation is parameterised by the GIOP version is it used for.
Since GIOP 1.0 does not support wstrings there is, by default, no
wstring transmission code set for GIOP 1.0. The addition and reception
of the code set service context is performed by interceptors within
the ORB core. A clientSendRequest interceptor adds the service context
the first time a connection is used; a serverReceiveRequest
interceptor reads it.

Voyager
-------

Voyager's wstring support predates GIOP 1.1 and the notion of code set
negotiation. It supports a proprietary marshalling format very similar
to the GIOP 1.0 string marshalling format. It marshals wstrings as an
unsigned long length field indicating the number of 16-bit Unicode
characters in the wstring, followed by the characters themselves in
big endian format, followed by a terminating null wchar. The length
includes the terminating null. It is rather odd that the wchars are
always marshalled big endian, even when the rest of the stream is
little endian. Individual wchars are marshalled as a big-endian 16-bit
Unicode value.

cs-voyager
----------

cs-voyager.cc registers a transmission code set implementation for
Voyager's marshalling format, marking it as suitable for GIOP 1.0.
Additional interceptors are registered for clientSendRequest and
serverReceiveRequest. The interceptors check to see if the GIOP
version is 1.0 and, if so, select the Voyager code set. For other GIOP
versions, the new interceptors do nothing, allowing the original ORB
core interceptors to deal with service contexts as usual. This means
that omniORB extended by cs-voyager.cc will successfully communicate
wstrings with both Voyager and any GIOP 1.1/1.2 ORB at the same time.

The transmission code set implementation is registered at static
initialisation time. At the same time an omniInitialiser is
registered. The initialiser adds the interceptors when ORB_init() is
called.

To use, simply link cs-voyager.cc into your application.


Duncan Grisby, April 2002.
