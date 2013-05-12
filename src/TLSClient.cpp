////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2013, Paul Beckingham, Federico Hernandez.
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
#include <TLSClient.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <netdb.h>

#define MAX_BUF 1024

////////////////////////////////////////////////////////////////////////////////
static void gnutls_log_function (int level, const char* message)
{
  std::cout << "c: " << level << " " << message;
}

////////////////////////////////////////////////////////////////////////////////
TLSClient::TLSClient ()
: _ca ("")
, _socket (0)
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
// std::cout, labelled with >>> for outgoing, <<< for incoming.
void TLSClient::debug (int level)
{
  _debug = true;

  gnutls_global_set_log_function (gnutls_log_function);
  gnutls_global_set_log_level (level);
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::init (const std::string& ca)
{
  _ca = ca;

  gnutls_global_init ();
  gnutls_certificate_allocate_credentials (&_credentials);
  gnutls_certificate_set_x509_trust_file (_credentials, _ca.c_str (), GNUTLS_X509_FMT_PEM);
  gnutls_init (&_session, GNUTLS_CLIENT);

  // Use default priorities.
  const char *err;
  int ret = gnutls_priority_set_direct (_session, "NORMAL", &err);
  if (ret < 0)
  {
    if (ret == GNUTLS_E_INVALID_REQUEST)
      std::cout << "c: ERROR Priority error at: " << err << "\n";

    exit (1);
  }

  // Apply the x509 credentials to the current session.
  gnutls_credentials_set (_session, GNUTLS_CRD_CERTIFICATE, _credentials);
}

////////////////////////////////////////////////////////////////////////////////
void TLSClient::connect (const std::string& host, const std::string& port)
{
  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints;
  memset (&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  if (::getaddrinfo (host.c_str (), port.c_str (), &hints, &res) != 0)
    throw "ERROR: " + std::string (::gai_strerror (errno));

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
      throw "ERROR: " + std::string (::strerror (errno));

    if (::connect (_socket, p->ai_addr, p->ai_addrlen) == -1)
      continue;

    break;
  }

  free (res);

  if (p == NULL)
    throw "ERROR: Could not connect to " + host + " " + port;

  gnutls_transport_set_ptr (_session, (gnutls_transport_ptr_t) _socket);

  // Perform the TLS handshake
  int ret = gnutls_handshake (_session);

  if (ret < 0)
  {
    std::cout << "c: ERROR Handshake failed\n";
    gnutls_perror (ret);
  }
  else
  {
    std::cout << "c: INFO Handshake was completed\n";
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

  int total = 0;
  int remaining = packet.length ();

  while (total < remaining)
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

    total     += status;
    remaining -= status;
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
      std::cout << "c: INFO Peer has closed the TLS connection\n";
      break;
    }

    // Something happened.
    if (received < 0)
      throw "ERROR: " + std::string (gnutls_strerror (received));

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
