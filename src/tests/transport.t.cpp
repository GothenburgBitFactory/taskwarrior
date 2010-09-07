////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <Context.h>
//#include <Att.h>
#include <Transport.h>
#include <test.h>

Context context;

class TransportTest : public Transport
{
  public:
      TransportTest (const std::string& uri) : Transport (uri)	{};

      std::string getHost() { return host; };
      std::string getPath() { return path; };
		std::string getUser() { return user; };
		std::string getPort() { return port; };             

	   virtual void recv(std::string) {};
		virtual void send(const std::string&) {};
};

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (16);

  TransportTest tport1 ("asfd://user@host/folder/");
  t.is (tport1.getUser (), "user",    "Transport::parseUri() : asfd://user@host/folder/");
  t.is (tport1.getHost (), "host",    "Transport::parseUri() : asfd://user@host/folder/");
  t.is (tport1.getPort (), "",        "Transport::parseUri() : asfd://user@host/folder/");
  t.is (tport1.getPath (), "folder/", "Transport::parseUri() : asfd://user@host/folder/");

  TransportTest tport2 ("user@host:22/folder/file.test");
  t.is (tport2.getUser (), "user",             "Transport::parseUri() : user@host:22/folder/file.test");
  t.is (tport2.getHost (), "host",             "Transport::parseUri() : user@host:22/folder/file.test");
  t.is (tport2.getPort (), "22",               "Transport::parseUri() : user@host:22/folder/file.test");
  t.is (tport2.getPath (), "folder/file.test", "Transport::parseUri() : user@host:22/folder/file.test");

  TransportTest tport3 ("hostname.abc.de/file.test");
  t.is (tport3.getUser (), "",                "Transport::parseUri() : hostname.abc.de/file.test");
  t.is (tport3.getHost (), "hostname.abc.de", "Transport::parseUri() : hostname.abc.de/file.test");
  t.is (tport3.getPort (), "",                "Transport::parseUri() : hostname.abc.de/file.test");
  t.is (tport3.getPath (), "file.test",       "Transport::parseUri() : hostname.abc.de/file.test");

  TransportTest tport4 ("hostname/");
  t.is (tport4.getUser (), "",         "Transport::parseUri() : hostname/");
  t.is (tport4.getHost (), "hostname", "Transport::parseUri() : hostname/");
  t.is (tport4.getPort (), "",         "Transport::parseUri() : hostname/");
  t.is (tport4.getPath (), "",         "Transport::parseUri() : hostname/");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
