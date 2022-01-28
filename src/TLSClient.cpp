////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>

#ifdef HAVE_LIBGNUTLS

#include <TLSClient.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <gnutls/x509.h>
#include <shared.h>
#include <format.h>

#define HEADER_SIZE 4
#define MAX_BUF 16384

#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
static int verify_certificate_callback (gnutls_session_t);
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
static void gnutls_log_function (int level, const char* message)
{
  std::cout << "c: " << level << ' ' << message;
}

////////////////////////////////////////////////////////////////////////////////
#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
static int verify_certificate_callback (gnutls_session_t session)
{
  const TLSClient* client = (TLSClient*) gnutls_session_get_ptr (session); // All
  return client->verify_certificate ();
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
TLSClient::~TLSClient ()
{
  gnutls_deinit (_session); // All
  gnutls_certificate_free_credentials (_credentials); // All
#if GNUTLS_VERSION_NUMBER < 0x030300
  gnutls_global_deinit (); // All
#endif

  if (_socket)
  {
    shutdown (_socket, SHUT_RDWR);
    close (_socket);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::limit (int max)
{
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with 'c: ...'.
void TLSClient::debug (int level)
{
  if (level)
    _debug = true;

  gnutls_global_set_log_function (gnutls_log_function); // All
  gnutls_global_set_log_level (level); // All
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::trust (const enum trust_level value)
{
  _trust = value;
  if (_debug)
  {
    if (_trust == allow_all)
      std::cout << "c: INFO Server certificate will be trusted automatically.\n";
    else if (_trust == ignore_hostname)
      std::cout << "c: INFO Server certificate will be verified but hostname ignored.\n";
    else
      std::cout << "c: INFO Server certificate will be verified.\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::ciphers (const std::string& cipher_list)
{
  _ciphers = cipher_list;
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::init (
  const std::string& ca,
  const std::string& cert,
  const std::string& key)
{
  _ca   = ca;
  _cert = cert;
  _key  = key;

  int ret;
#if GNUTLS_VERSION_NUMBER < 0x030300
  ret = gnutls_global_init (); // All
  if (ret < 0)
    throw format ("TLS init error. {1}", gnutls_strerror (ret)); // All
#endif

  ret = gnutls_certificate_allocate_credentials (&_credentials); // All
  if (ret < 0)
    throw format ("TLS allocation error. {1}", gnutls_strerror (ret)); // All

#if GNUTLS_VERSION_NUMBER >= 0x030014
  // Automatic loading of system installed CA certificates.
  ret = gnutls_certificate_set_x509_system_trust (_credentials); // 3.0.20
  if (ret < 0)
    throw format ("Bad System Trust. {1}", gnutls_strerror (ret)); // All
#endif

  if (_ca != "")
  {
    // The gnutls_certificate_set_x509_key_file call returns number of
    // certificates parsed on success (including 0, when no certificate was
    // found) and negative values on error
    ret = gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM); // All
    if (ret == 0)
      throw format ("CA file {1} contains no certificate.", _ca);
    else if (ret < 0)
      throw format ("Bad CA file: {1}", gnutls_strerror (ret)); // All

  }

  // TODO This may need 0x030111 protection.
  if (_cert != "" &&
      _key != "" &&
      (ret = gnutls_certificate_set_x509_key_file (_credentials, _cert.c_str (), _key.c_str (), GNUTLS_X509_FMT_PEM)) < 0) // 3.1.11
    throw format ("Bad client CERT/KEY file. {1}", gnutls_strerror (ret)); // All

#if GNUTLS_VERSION_NUMBER < 0x030406
#if GNUTLS_VERSION_NUMBER >= 0x020a00
  // The automatic verification for the server certificate with
  // gnutls_certificate_set_verify_function only works with gnutls
  // >=2.9.10. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  gnutls_certificate_set_verify_function (_credentials, verify_certificate_callback); // 2.10.0
#endif
#endif
  ret = gnutls_init (&_session, GNUTLS_CLIENT); // All
  if (ret < 0)
    throw format ("TLS client init error. {1}", gnutls_strerror (ret)); // All

  // Use default priorities unless overridden.
  if (_ciphers == "")
    _ciphers = "NORMAL";

  const char *err;
  ret = gnutls_priority_set_direct (_session, _ciphers.c_str (), &err); // All
  if (ret < 0)
  {
    if (_debug && ret == GNUTLS_E_INVALID_REQUEST)
      std::cout << "c: ERROR Priority error at: " << err << '\n';

    throw format ("Error initializing TLS. {1}", gnutls_strerror (ret)); // All
  }

  // Apply the x509 credentials to the current session.
  ret = gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, _credentials); // All
  if (ret < 0)
    throw format ("TLS credentials error. {1}", gnutls_strerror (ret)); // All
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::connect (const std::string& host, const std::string& port)
{
  _host = host;
  _port = port;

  int ret;
#if GNUTLS_VERSION_NUMBER >= 0x030406
  // For _trust == TLSClient::allow_all we perform no action
  if (_trust == TLSClient::ignore_hostname)
    gnutls_session_set_verify_cert (_session, nullptr, 0); // 3.4.6
  else if (_trust == TLSClient::strict)
    gnutls_session_set_verify_cert (_session, _host.c_str (), 0); // 3.4.6
#endif

  // SNI.  Only permitted when _host is a DNS name, not an IPv4/6 address.
  std::string dummyAddress;
  int dummyPort;
  if (! isIPv4Address (_host, dummyAddress, dummyPort) &&
      ! isIPv6Address (_host, dummyAddress, dummyPort))
  {
    ret = gnutls_server_name_set (_session, GNUTLS_NAME_DNS, _host.c_str (), _host.length ()); // All
    if (ret < 0)
      throw format ("TLS SNI error. {1}", gnutls_strerror (ret)); // All
  }

  // Store the TLSClient instance, so that the verification callback can access
  // it during the handshake below and call the verification method.
  gnutls_session_set_ptr (_session, (void*) this); // All

  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints {};
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  ret = ::getaddrinfo (host.c_str (), port.c_str (), &hints, &res);
  if (ret != 0)
    throw std::string (::gai_strerror (ret));

  // Try them all, stop on success.
  struct addrinfo* p;
  for (p = res; p != nullptr; p = p->ai_next)
  {
    if ((_socket = ::socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;

    // When a socket is closed, it remains unavailable for a while (netstat -an).
    // Setting SO_REUSEADDR allows this program to assume control of a closed,
    // but unavailable socket.
    int on = 1;
    if (::setsockopt (_socket,
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (const void*) &on,
                      sizeof (on)) == -1)
      throw std::string (::strerror (errno));

    if (::connect (_socket, p->ai_addr, p->ai_addrlen) == -1)
      continue;

    break;
  }

  free (res);

  if (p == nullptr)
    throw format ("Could not connect to {1} {2}", host, port);

#if GNUTLS_VERSION_NUMBER >= 0x030100
  gnutls_handshake_set_timeout (_session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT); // 3.1.0
#endif

#if GNUTLS_VERSION_NUMBER >= 0x030109
  gnutls_transport_set_int (_session, _socket); // 3.1.9
#else
  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) (intptr_t) _socket); // All
#endif

  // Perform the TLS handshake
  do
  {
    ret = gnutls_handshake (_session); // All
  }
  while (ret < 0 && gnutls_error_is_fatal (ret) == 0); // All

  if (ret < 0)
  {
#if GNUTLS_VERSION_NUMBER >= 0x030406
    if (ret == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR)
    {
      auto type = gnutls_certificate_type_get (_session); // All
      auto status = gnutls_session_get_verify_cert_status (_session); // 3.4.6
      gnutls_datum_t out;
      gnutls_certificate_verification_status_print (status, type, &out, 0);  // 3.1.4

      std::string error {(const char*) out.data};
      gnutls_free (out.data); // All

      throw format ("Handshake failed. {1}", error); // All
    }
#else
    throw format ("Handshake failed. {1}", gnutls_strerror (ret)); // All
#endif
  }

#if GNUTLS_VERSION_NUMBER < 0x020a00
  // The automatic verification for the server certificate with
  // gnutls_certificate_set_verify_function does only work with gnutls
  // >=2.10.0. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  ret = verify_certificate ();
  if (ret < 0)
  {
    if (_debug)
      std::cout << "c: ERROR Certificate verification failed.\n";
    throw format ("Error initializing TLS. {1}", gnutls_strerror (ret)); // All
  }
#endif

  if (_debug)
  {
#if GNUTLS_VERSION_NUMBER >= 0x03010a
    char* desc = gnutls_session_get_desc (_session); // 3.1.10
    std::cout << "c: INFO Handshake was completed: " << desc << '\n';
    gnutls_free (desc);
#else
    std::cout << "c: INFO Handshake was completed.\n";
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::bye ()
{
  gnutls_bye (_session, GNUTLS_SHUT_RDWR); // All
}

////////////////////////////////////////////////////////////////////////////////
int TLSClient::verify_certificate () const
{
  if (_trust == TLSClient::allow_all)
    return 0;

  if (_debug)
    std::cout << "c: INFO Verifying certificate.\n";

  // This verification function uses the trusted CAs in the credentials
  // structure. So you must have installed one or more CA certificates.
  unsigned int status = 0;
  const char* hostname = _host.c_str();
#if GNUTLS_VERSION_NUMBER >= 0x030104
  if (_trust == TLSClient::ignore_hostname)
    hostname = nullptr;

  int ret = gnutls_certificate_verify_peers3 (_session, hostname, &status); // 3.1.4
  if (ret < 0)
  {
    if (_debug)
      std::cout << "c: ERROR Certificate verification peers3 failed. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  // status 16450 == 0100000001000010
  //   GNUTLS_CERT_INVALID             1<<1
  //   GNUTLS_CERT_SIGNER_NOT_FOUND    1<<6
  //   GNUTLS_CERT_UNEXPECTED_OWNER    1<<14  Hostname does not match

  if (_debug && status)
    std::cout << "c: ERROR Certificate status=" << status << '\n';
#else
  int ret = gnutls_certificate_verify_peers2 (_session, &status); // All
  if (ret < 0)
  {
    if (_debug)
      std::cout << "c: ERROR Certificate verification peers2 failed. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  if (_debug && status)
    std::cout << "c: ERROR Certificate status=" << status << '\n';

  if ((status == 0) && (_trust != TLSClient::ignore_hostname))
  {
    if (gnutls_certificate_type_get (_session) == GNUTLS_CRT_X509) // All
    {
      const gnutls_datum* cert_list;
      unsigned int cert_list_size;
      gnutls_x509_crt cert;

      cert_list = gnutls_certificate_get_peers (_session, &cert_list_size); // All
      if (cert_list_size == 0)
      {
        if (_debug)
          std::cout << "c: ERROR Certificate get peers failed. " << gnutls_strerror (ret) << '\n'; // All
        return GNUTLS_E_CERTIFICATE_ERROR;
      }

      ret = gnutls_x509_crt_init (&cert); // All
      if (ret < 0)
      {
        if (_debug)
          std::cout << "c: ERROR x509 init failed. " << gnutls_strerror (ret) << '\n'; // All
        return GNUTLS_E_CERTIFICATE_ERROR;
      }

      ret = gnutls_x509_crt_import (cert, &cert_list[0], GNUTLS_X509_FMT_DER); // All
      if (ret < 0)
      {
        if (_debug)
          std::cout << "c: ERROR x509 cert import. " << gnutls_strerror (ret) << '\n'; // All
        gnutls_x509_crt_deinit(cert); // All
        return GNUTLS_E_CERTIFICATE_ERROR;
      }

      if (gnutls_x509_crt_check_hostname (cert, hostname) == 0) // All
      {
        if (_debug)
          std::cout << "c: ERROR x509 cert check hostname. " << gnutls_strerror (ret) << '\n'; // All
        gnutls_x509_crt_deinit(cert);
        return GNUTLS_E_CERTIFICATE_ERROR;
      }
    }
    else
      return GNUTLS_E_CERTIFICATE_ERROR;
  }
#endif

#if GNUTLS_VERSION_NUMBER >= 0x030104
  gnutls_certificate_type_t type = gnutls_certificate_type_get (_session); // All
  gnutls_datum_t out;
  ret = gnutls_certificate_verification_status_print (status, type, &out, 0); // 3.1.4
  if (ret < 0)
  {
    if (_debug)
      std::cout << "c: ERROR certificate verification status. " << gnutls_strerror (ret) << '\n'; // All
    return GNUTLS_E_CERTIFICATE_ERROR;
  }

  if (_debug)
    std::cout << "c: INFO " << out.data << '\n';
  gnutls_free (out.data);
#endif

  if (status != 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  // Continue handshake.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::send (const std::string& data)
{
  std::string packet = "XXXX" + data;

  // Encode the length.
  unsigned long l = packet.length ();
  packet[0] = l >>24;
  packet[1] = l >>16;
  packet[2] = l >>8;
  packet[3] = l;

  unsigned int total = 0;

  int status;
  do
  {
    status = gnutls_record_send (_session, packet.c_str () + total, packet.length () - total); // All
  }
  while ((status > 0 && (total += status) < packet.length ()) ||
          status == GNUTLS_E_INTERRUPTED ||
          status == GNUTLS_E_AGAIN);

  if (_debug)
    std::cout << "c: INFO Sending 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::recv (std::string& data)
{
  data = "";          // No appending of data.
  int received = 0;
  int total = 0;

  // Get the encoded length.
  unsigned char header[HEADER_SIZE] {};
  do
  {
    received = gnutls_record_recv (_session, header + total, HEADER_SIZE - total); // All
  }
  while ((received > 0 && (total += received) < HEADER_SIZE) ||
          received == GNUTLS_E_INTERRUPTED ||
          received == GNUTLS_E_AGAIN);

  if (total < HEADER_SIZE) {
    throw std::string ("Failed to receive header: ") +
        (received < 0 ? gnutls_strerror(received) : "connection lost?");
  }

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];
  if (_debug)
    std::cout << "c: INFO expecting " << expected << " bytes.\n";

  if (_limit && expected >= (unsigned long) _limit) {
    std::ostringstream err_str;
    err_str << "Expected message size " << expected << " is larger than allowed limit " << _limit;
    throw err_str.str ();
  }

  // Arbitrary buffer size.
  char buffer[MAX_BUF];

  // Keep reading until no more data.  Concatenate chunks of data if a) the
  // read was interrupted by a signal, and b) if there is more data than
  // fits in the buffer.
  do
  {
    int chunk_size = 0;
    do
    {
      received = gnutls_record_recv (_session, buffer + chunk_size, MAX_BUF - chunk_size); // All
      if (received > 0) {
        total += received;
        chunk_size += received;
      }
    }
    while ((received > 0 && (unsigned long) total < expected && chunk_size < MAX_BUF) ||
            received == GNUTLS_E_INTERRUPTED ||
            received == GNUTLS_E_AGAIN);

    // Other end closed the connection.
    if (received == 0)
    {
      if (_debug)
        std::cout << "c: INFO Peer has closed the TLS connection\n";
      break;
    }

    // Something happened.
    if (received < 0)
      throw std::string (gnutls_strerror (received)); // All

    data.append (buffer, chunk_size);

    // Stop at defined limit.
    if (_limit && total > _limit)
      break;
  }
  while (received > 0 && total < (int) expected);

  if (_debug)
    std::cout << "c: INFO Receiving 'XXXX"
              << data.c_str ()
              << "' (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
#endif
