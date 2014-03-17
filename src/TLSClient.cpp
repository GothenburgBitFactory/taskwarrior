////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>

#ifdef HAVE_LIBGNUTLS

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef OPENBSD
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <sys/types.h>
#include <netdb.h>
#include <TLSClient.h>
#include <text.h>
#include <i18n.h>

#define MAX_BUF 16384

static int verify_certificate_callback (gnutls_session_t);

static bool trust_override = false;

////////////////////////////////////////////////////////////////////////////////
static void gnutls_log_function (int level, const char* message)
{
  std::cout << "c: " << level << " " << message;
}

////////////////////////////////////////////////////////////////////////////////
static int verify_certificate_callback (gnutls_session_t session)
{
  if (trust_override)
    return 0;

  // Get the hostname from the session.
  const char* hostname = (const char*) gnutls_session_get_ptr (session);

  // This verification function uses the trusted CAs in the credentials
  // structure. So you must have installed one or more CA certificates.
  unsigned int status = 0;
#if GNUTLS_VERSION_NUMBER >= 0x030104
  int ret = gnutls_certificate_verify_peers3 (session, NULL, &status);
#else
  int ret = gnutls_certificate_verify_peers2 (session, &status);
#endif
  if (ret < 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

#if GNUTLS_VERSION_NUMBER >= 0x030105
  gnutls_certificate_type_t type = gnutls_certificate_type_get (session);
  gnutls_datum_t out;
  ret = gnutls_certificate_verification_status_print (status, type, &out, 0);
  if (ret < 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  //std::cout << "c: INFO " << out.data << "\n";

  gnutls_free (out.data);
#endif

  if (status != 0)
    return GNUTLS_E_CERTIFICATE_ERROR;

  // Continue handshake.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
TLSClient::TLSClient ()
: _ca ("")
, _cert ("")
, _key ("")
, _session(0)
, _socket (0)
, _limit (0)
, _debug (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TLSClient::~TLSClient ()
{
  gnutls_deinit (_session);
  gnutls_certificate_free_credentials (_credentials);
  gnutls_global_deinit ();

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

  gnutls_global_set_log_function (gnutls_log_function);
  gnutls_global_set_log_level (level);
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::trust (bool value)
{
  trust_override = value;
  if (_debug)
  {
    if (trust_override)
      std::cout << "c: INFO Server certificate trusted automatically.\n";
    else
      std::cout << "c: INFO Server certificate trust verified.\n";
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

  gnutls_global_init ();
  gnutls_certificate_allocate_credentials (&_credentials);

  if (_ca != "" &&
      gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM) < 0)
    throw std::string ("Missing CA file.");

  if (_cert != "" &&
      _key != "" &&
      gnutls_certificate_set_x509_key_file (_credentials, _cert.c_str (), _key.c_str (), GNUTLS_X509_FMT_PEM) < 0)
    throw std::string ("Missing CERT file.");

#if GNUTLS_VERSION_NUMBER >= 0x02090a
  // The automatic verification for the server certificate with
  // gnutls_certificate_set_verify_function only works with gnutls
  // >=2.9.10. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  gnutls_certificate_set_verify_function (_credentials, verify_certificate_callback);
#endif
  gnutls_init (&_session, GNUTLS_CLIENT);

  // Use default priorities unless overridden.
  if (_ciphers == "")
    _ciphers = "NORMAL";

  const char *err;
  int ret = gnutls_priority_set_direct (_session, _ciphers.c_str (), &err);
  if (ret < 0)
  {
    if (_debug && ret == GNUTLS_E_INVALID_REQUEST)
      std::cout << "c: ERROR Priority error at: " << err << "\n";

    throw std::string (STRING_TLS_INIT_FAIL);
  }

  // Apply the x509 credentials to the current session.
  gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, _credentials);
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::connect (const std::string& host, const std::string& port)
{
  // Store the host name, so the verification callback can access it during the
  // handshake below.
  gnutls_session_set_ptr (_session, (void*) host.c_str ());

  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints = {0};
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  if (::getaddrinfo (host.c_str (), port.c_str (), &hints, &res) != 0)
    throw std::string (::gai_strerror (errno));

  // Try them all, stop on success.
  struct addrinfo* p;
  for (p = res; p != NULL; p = p->ai_next)
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

  if (p == NULL)
    throw format (STRING_CMD_SYNC_CONNECT, host, port);

#if GNUTLS_VERSION_NUMBER >= 0x030109
  gnutls_transport_set_int (_session, _socket);
#else
  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) (long) _socket);
#endif

  // Perform the TLS handshake
  int ret;
  do
  {
    ret = gnutls_handshake (_session);
  }
  while (ret < 0 && gnutls_error_is_fatal (ret) == 0);
  if (ret < 0)
    throw format (STRING_CMD_SYNC_HANDSHAKE, gnutls_strerror (ret));

#if GNUTLS_VERSION_NUMBER < 0x02090a
  // The automatic verification for the server certificate with
  // gnutls_certificate_set_verify_function does only work with gnutls
  // >=2.9.10. So with older versions we should call the verify function
  // manually after the gnutls handshake.
  ret = verify_certificate_callback(_session);
  if (ret < 0)
    throw std::string (STRING_TLS_INIT_FAIL);
#endif

  if (_debug)
  {
#if GNUTLS_VERSION_NUMBER >= 0x03010a
    char* desc = gnutls_session_get_desc (_session);
    std::cout << "c: INFO Handshake was completed: " << desc << "\n";
    gnutls_free (desc);
#else
    std::cout << "c: INFO Handshake was completed.\n";
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::bye ()
{
  gnutls_bye (_session, GNUTLS_SHUT_RDWR);
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
  unsigned int remaining = packet.length ();

  while (total < packet.length ())
  {
    int status;
    do
    {
      status = gnutls_record_send (_session, packet.c_str () + total, remaining);
    }
    while (errno == GNUTLS_E_INTERRUPTED ||
           errno == GNUTLS_E_AGAIN);

    if (status == -1)
      break;

    total     += (unsigned int) status;
    remaining -= (unsigned int) status;
  }

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

  // Get the encoded length.
  unsigned char header[4] = {0};
  do
  {
    received = gnutls_record_recv (_session, header, 4);
  }
  while (received > 0 &&
         (errno == GNUTLS_E_INTERRUPTED ||
          errno == GNUTLS_E_AGAIN));

  int total = received;

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];
  if (_debug)
    std::cout << "c: INFO expecting " << expected << " bytes.\n";

  // TODO This would be a good place to assert 'expected < _limit'.

  // Arbitrary buffer size.
  char buffer[MAX_BUF];

  // Keep reading until no more data.  Concatenate chunks of data if a) the
  // read was interrupted by a signal, and b) if there is more data than
  // fits in the buffer.
  do
  {
    do
    {
      received = gnutls_record_recv (_session, buffer, MAX_BUF - 1);
    }
    while (received > 0 &&
           (errno == GNUTLS_E_INTERRUPTED ||
            errno == GNUTLS_E_AGAIN));

    // Other end closed the connection.
    if (received == 0)
    {
      if (_debug)
        std::cout << "c: INFO Peer has closed the TLS connection\n";
      break;
    }

    // Something happened.
    if (received < 0 && gnutls_error_is_fatal (received) == 0)
    {
      if (_debug)
        std::cout << "c: WARNING " << gnutls_strerror (received) << "\n";
    }

    else if (received < 0)
      throw std::string (gnutls_strerror (received));

    buffer [received] = '\0';
    data += buffer;
    total += received;

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
