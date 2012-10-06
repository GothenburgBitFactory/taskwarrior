////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include <Socket.h>

////////////////////////////////////////////////////////////////////////////////
Socket::Socket (int family, int type, int protocol) :
  _port (0),
  _family (family),
  _socket (0),
  _debug (false),
  _limit (0) // Unlimited
{
  // family:   AF_INET (IPv4), AF_INET6 (IPv6), AF_LOCAL, AF_ROUTE, AF_KEY.
  // type:     SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET, SOCK_RAW, SOCK_PACKET (Linux).
  // protocol: IPPROTO_TCP, IPPROTO_UDP, IPPROTO_SCTP.
  if ((_socket = ::socket (family, type, protocol)) < 0)
    // This looks like a blank error message, but don't forget that the Error
    // class will use ::strerror and append a system error message if errno is
    // non-zero.
    throw "ERROR: " + std::string (::strerror (errno));

  // When a socket is closed, it remains unavailable for a while (netstat -an).
  // Setting SO_REUSEADDR allows this program to assume control of a closed, but
  // unavailable socket.
  int on = 1;
  if (::setsockopt (_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof (on)) < 0)
    throw "ERROR: " + std::string (::strerror (errno));
}

////////////////////////////////////////////////////////////////////////////////
Socket::Socket (int s) :
  _port (0),
  _family (AF_UNSPEC),
  _socket (s),
  _debug (false),
  _limit (0) // Unlimited
{
}

////////////////////////////////////////////////////////////////////////////////
Socket::~Socket ()
{
  close ();
}

////////////////////////////////////////////////////////////////////////////////
// For clients.
void Socket::connect (const std::string& host, const int port)
{
  _port = port;
  std::string machine;

  // If the address is of the form \d+\.\d+\.\d+\.\d+
  bool ip = true;
  for (unsigned int i = 0; i < host.length (); ++i)
    if (!isdigit (host[i]) && host[i] != '.')
    {
      ip = false;
      break;
    }

  if (ip)
  {
    struct in_addr in;
    in.s_addr = inet_addr (host.c_str ());
    struct hostent* he = gethostbyaddr ((char*) &in,
                                        sizeof (in.s_addr),
                                        AF_INET);
    if (!he)
      throw std::string ("ERROR: Cannot resolve host from ") + host;

    machine = he->h_name;
  }
  else
  {
    struct hostent* he = gethostbyname (host.c_str ());
    if (!he)
      throw std::string ("ERROR: Cannot resolve host from ") + host;

    machine = inet_ntoa (*((struct in_addr*)he->h_addr));
  }

  struct sockaddr_in server = {0};
  server.sin_family = _family;
  server.sin_port = htons (_port);
  if (::inet_pton (_family, machine.c_str (), &server.sin_addr) != 1)
    throw "ERROR: " + std::string (::strerror (errno));

  if (::connect (_socket, (struct sockaddr*) &server, sizeof (server)) < 0)
    throw "ERROR: " + std::string (::strerror (errno));
}

////////////////////////////////////////////////////////////////////////////////
void Socket::close ()
{
  if (_socket)
    ::close (_socket);
  _socket = 0;
}

////////////////////////////////////////////////////////////////////////////////
// For servers.
void Socket::bind (int family, int port)
{
  struct sockaddr_in server;
  memset (&server, 0, sizeof (server));
  server.sin_family = _family = family;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (_port = port);

  if (::bind (_socket, (struct sockaddr*) &server, sizeof (server)) < 0)
    throw "ERROR: " + std::string (::strerror (errno));
}

////////////////////////////////////////////////////////////////////////////////
void Socket::listen (int queue /*= 5*/)
{
  if (::listen (_socket, queue) < 0)
    throw "ERROR: " + std::string (::strerror (errno));
}

////////////////////////////////////////////////////////////////////////////////
int Socket::accept ()
{
  struct sockaddr_in client;
  socklen_t length = sizeof (client);
  int connection;

  do
  {
    memset (&client, 0, length);
    connection = ::accept (_socket, (struct sockaddr*) &client, &length);
  }
  while (errno == EINTR);

  if (connection < 0)
    throw "ERROR: " + std::string (::strerror (errno));

  return connection;
}

////////////////////////////////////////////////////////////////////////////////
void Socket::write (const std::string& data)
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
      status = ::send (_socket, packet.c_str () + total, remaining, 0);
    }
    while (errno == EINTR);

    if (status == -1)
      break;

    total     += status;
    remaining -= status;
  }

  if (_debug)
    std::cout << ">>> "
              << data.c_str ()
              << " (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void Socket::read (std::string& data)
{
  data = "";          // No appending of data.
  int received = 0;

  // Get the encoded length.
  unsigned char header[4];
  do
  {
    received = ::recv (_socket, header, sizeof (header), 0);
  }
  while (errno == EINTR);
  int total = received;

  // Decode the length.
  unsigned long expected = (header[0]<<24) |
                           (header[1]<<16) |
                           (header[2]<<8) |
                            header[3];

  // TODO This would be a good place to assert 'expected < _limit'.

  // Arbitrary buffer size.
  char buffer[8192];

  // Keep reading until no more data.  Concatenate chunks of data if a) the
  // read was interrupted by a signal, and b) if there is more data than
  // fits in the buffer.
  do
  {
    do
    {
      received = ::recv (_socket, buffer, sizeof (buffer) - 1, 0);
    }
    while (errno == EINTR);

    // Other end closed the connection.
    if (received == 0)
      break;

    // Something happened.
    if (received < 0)
      throw "ERROR: " + std::string (::strerror (errno));

    buffer [received] = '\0';
    data += buffer;
    total += received;

    // Stop at defined limit.
    if (_limit && total > _limit)
      break;
  }
  while (received > 0 && total < (int) expected);

  if (_debug)
    std::cout << "<<< "
              << data.c_str ()
              << " (" << total << " bytes)"
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void Socket::limit (int max)
{
  _limit = max;
}

////////////////////////////////////////////////////////////////////////////////
// Calling this method results in all subsequent socket traffic being sent to
// std::cout, labelled with >>> for outgoing, <<< for incoming.
void Socket::debug ()
{
  _debug = true;
}

////////////////////////////////////////////////////////////////////////////////

