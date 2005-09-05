Generating keys for use with the SSL transport
==============================================

For the SSL transport to work, servers need signed certificates.
Depending on the server settings, clients may also need signed
certificates.

The following is a brief description of how to become your own
certificate authority and issue and sign certificates, using the
openssl command line tools.


Before starting, find the default openssl.cnf file that was installed
with openssl, copy it to a suitable location, and edit it as you feel
appropriate.

Now, build a certificate directory structure, authority key and
certificate:

  mkdir demoCA demoCA/private demoCA/newcerts

  openssl req -config openssl.cnf -x509 -newkey rsa:2048 -keyout demoCA/private/cakey.pem -out demoCA/cacert.pem

  echo 01 >demoCA/serial
  touch demoCA/index.txt


Next, issue a key request and sign it:

  openssl req -config openssl.cnf -new -keyout server_key.pem -out server_req.pem -days 3650

  openssl ca -config openssl.cnf -policy policy_anything -out server_cert.pem -in server_req.pem 


Amongst other things, you now have a server key file in server_key.pem
and a certificate in server_cert.pem. To make a key file suitable to
use in omniORB, concatenate the key and certificate files together.
You can skip the human-readable(ish) text in the certificate file
before the -----BEGIN CERTIFICATE----- marker.


If need be, create more certificates for servers and clients in the
same way.
