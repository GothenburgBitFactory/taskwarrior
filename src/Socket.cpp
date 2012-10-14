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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <Socket.h>

////////////////////////////////////////////////////////////////////////////////
Socket::Socket () :
  _socket (0),
  _limit (0), // Unlimited
  _debug (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Socket::Socket (int s) :
  _socket (s),
  _limit (0), // Unlimited
  _debug (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Socket::~Socket ()
{
  close ();
}

////////////////////////////////////////////////////////////////////////////////
// For clients.
void Socket::connect (const std::string& host, const std::string& port)
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
    {
      close ();
      continue;
    }

    break;
  }

  free (res);

  if (p == NULL)
    throw "ERROR: Could not connect to " + host + " " + port;
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
void Socket::bind (const std::string& port)
{
  // use IPv4 or IPv6, does not matter.
  struct addrinfo hints;
  memset (&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // use my IP

  struct addrinfo* res;
  if (::getaddrinfo (NULL, port.c_str (), &hints, &res) != 0)
    throw "ERROR: " + std::string (::gai_strerror (errno));

  if ((_socket = ::socket (res->ai_family,
                           res->ai_socktype,
                           res->ai_protocol)) == -1)
    throw "ERROR: Can not bind to port " + port;

  // When a socket is closed, it remains unavailable for a while (netstat -an).
  // Setting SO_REUSEADDR allows this program to assume control of a closed, but
  // unavailable socket.
  int on = 1;
  if (::setsockopt (_socket,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const void*) &on,
                    sizeof (on)) == -1)
    throw "ERROR: " + std::string (::strerror (errno));

  if (::bind (_socket, res->ai_addr, res->ai_addrlen) == -1)
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
  struct sockaddr_storage client;
  socklen_t length = sizeof client;
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
// get sockaddr, IPv4 or IPv6:
void* Socket::get_in_addr (struct sockaddr* sa)
{
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*) sa)->sin_addr);

  return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

////////////////////////////////////////////////////////////////////////////////

