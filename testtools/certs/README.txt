Instructions for generating the SSL certificates (every 10 years)
=================================================================

New CA key/certificate
----------------------

openssl req -x509 -nodes -new -sha256 -days 3650 -newkey rsa:4096 -keyout cakey.pem -out cacert.pem -subj "/CN=KDAB Certificate Test CA/C=SE/ST=Värmland/O=KDAB Test Root Certification Authority/emailAddress=ca@example.com"

Localhost signed certificate
----------------------------

Add this into a file called cert.ext:

authorityInfoAccess=OCSP;URI:http://ocsp.example.com:8888/
basicConstraints=CA:FALSE

Create a certificate request:

openssl req -nodes -new -sha256 -newkey rsa:4096 -keyout test-127.0.0.1-key.pem -out test-127.0.0.1-cert.crt -subj "/CN=127.0.0.1/C=SE/ST=Värmland/O=KDSoap Tests/emailAddress=test@example.com"

Sign the certificate with the CA:

openssl x509 -req -sha256 -days 3650 -in test-127.0.0.1-cert.crt -CA cacert.pem -CAkey cakey.pem -CAcreateserial -out test-127.0.0.1-cert.pem -extfile cert.ext

You then just need to commit the .pem files, no need of the .crt or .srl, I think.
