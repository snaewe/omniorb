omniConnectionMgmt library
==========================

This is an experimental omniORB extension for application-level
connection management. Its purpose is to allow clients and servers to
negotiate private GIOP connections, and to control how the connections
are used in multi-threaded situations.

The omniConnectionMgmt library has two functions, defined in
include/omniORB4/omniConnectionMgmt.h:

namespace omniConnectionMgmt {

  void init();

  CORBA::Object_ptr
  makeRestrictedReference(CORBA::Object_ptr obj,
			  CORBA::ULong      connection_id,
			  CORBA::ULong      max_connections,
			  CORBA::ULong      max_threads,
			  CORBA::Boolean    data_batch,
			  CORBA::Boolean    permit_interleaved,
			  CORBA::Boolean    server_hold_open);
};

The init() function must be called before CORBA::ORB_init() in every
process that is to take part in the connection management.

The makeRestrictedReference() function is the single entry-point to
the connection management functionality. It builds an annotated object
reference that contains information for the connection management
system. It returns a new reference, leaving the original object
reference unchanged.

Some parameters affect the client side; the others the server side:


Client-side parameters
----------------------

connection_id

  This number identifies the private connection set. All object
  references with the same connection_id will share the same set of
  GIOP connections. Object references with different connection ids
  are guaranteed to use different connections from each other, and
  from object references that have not been annotated with
  makeRestrictedReference().

max_connections

  This parameter overrides the omniORB maxGIOPConnectionPerServer
  configuration parameter for the given connection_id. It determines
  the maximum number of separate GIOP connections that will be opened
  to the object's server to service concurrent calls. It is common to
  set this value to 1, indicating that only one connection will be
  used for the given connection_id. Note that this parameter can only
  be used to reduce the default maxGIOPConnectionPerServer value, not
  increase it.

data_batch

  omniORB usually configures TCP connections to disable Nagle's
  algorithm, which batches small messages together into single IP
  packages, since that is best for the usual CORBA usage pattern of
  two-way requests. Setting this parameter to true overrides that, and
  enables Nagle's algorithm on TCP connections or equivalent
  functionality on other transports. This can increase throughput if a
  client is sending a large number of small oneway calls.

permit_interleaved

  This parameter overrides the oneCallPerConnection configuration
  parameter that determines whether multi-threaded clients can
  interleave calls on a single connection, issuing a new request
  message while a previous request is still waiting for a reply. If
  permit_interleaved is true, clients can interleave messages; if it
  is false, they cannot.


Server-side parameters
----------------------

max_threads

  This parameter overrides the maxServerThreadPerConnection
  configuration parameter that determines the maximum number of
  concurrent threads the server will use to service requests coming
  from a connection. Note that this parameter is only relevant if
  either the client permits interleaved calls, or if oneway operations
  are used, since those are the only circumstances under which the
  server can receive a new request on a connection while already
  handling a request. As with the max_connections client-side
  parameter, this parameter can only reduce the default number of
  threads, not increase it.

server_hold_open

  Normally, both clients and servers can decide to close a GIOP
  connection at any time. When using normal two-way calls, this is no
  problem since if a server closes a connection, the client is
  guaranteed to notice it when it waits for a reply, and can retry the
  call if necessary. With oneway calls, however, if a server closes a
  connection just as the client is sending a request, the client will
  not know whether the oneway call was received or not, and the call
  will potentially be lost. By setting the server_hold_open parameter
  to true, the server will not close the connection, relying on the
  client to do so. In that case, oneway calls will not be lost unless
  there is a network problem that breaks the GIOP connection.


Usage
-----

The omniConnectionMgmt extension is very easy to use -- simply call
the init() method in all processes involved, then restrict references
as required. makeRestrictedReference() adds profile information to the
object reference's IOR, meaning that the parameters become part of the
object reference and are transmitted along with it. In other words, a
server can create a restricted reference and send it to a client, and
the client will automatically make use of the restricted parameters
when it invokes operations on the object reference. Alternatively, a
client can restrict a normal reference it receives, in order to change
its own behaviour.


Implementation details
----------------------

This section explains how the extension works. These details are not
required to use the extension, but are useful if you wish to
interoperate with it.

The extension annotates object references by adding a new tagged
component to their IORs, and controls server behaviour using a GIOP
service context. The contents of both are defined in IDL in
idl/omniConnectionData.idl.

In IIOP 1.1 and above, the omniConnectionData::ComponentData struct is
added as an encapsulation within a tagged component in the IIOP
profile; IIOP 1.0 IORs do not have tagged components inside the IIOP
profile, so the component is added inside a MultipleComponentProfile.
The IOR component has tag value 0x41545404.

The omniConnectionData::ServiceData struct is sent in an encapsulation
as a service context on the first call on a new connection. The
service context tag value is also 0x41545404.

The extension is implemented in the single omniConnectionMgmt.cc
source file. The code is quite well commented, and should be quite
easy to follow.
