////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Johannes Schlatow.
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
#include <Uri.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (44);

  Uri uri1 ("asfd://user@host/folder/");
  uri1.parse ();
  t.is (uri1.user,      "user",    "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1.host,      "host",    "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1.port,      "",        "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1.path,      "folder/", "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1.protocol,  "asfd",    "Uri::parse() : asdf://user@host/folder/");
  t.ok (uri1.append ("file.test"),          "Uri::append() to path");
  t.is (uri1.path,      "folder/file.test", "Uri::append() ok");

  Uri uri2 ("user@host:folder/file.test");
  uri2.parse ();
  t.is (uri2.user,      "user",             "Uri::parse() : user@host:folder/file.test");
  t.is (uri2.host,      "host",             "Uri::parse() : user@host:folder/file.test");
  t.is (uri2.port,      "",                 "Uri::parse() : user@host/folder/file.test");
  t.is (uri2.path,      "folder/file.test", "Uri::parse() : user@host/folder/file.test");
  t.is (uri2.protocol,  "ssh",              "Uri::parse() : user@host/folder/file.test");
  t.notok (uri2.append ("test.dat"),        "Uri::append() to file");

  Uri uri3 ("rsync://hostname.abc.de:1234//abs/path");
  uri3.parse ();
  t.is (uri3.user,     "",                "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3.host,     "hostname.abc.de", "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3.port,     "1234",            "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3.path,     "/abs/path",       "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3.protocol, "rsync",           "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");

  Uri uri4 ("hostname:");
  uri4.parse ();
  t.is (uri4.user,     "",         "Uri::parse() : hostname:");
  t.is (uri4.host,     "hostname", "Uri::parse() : hostname:");
  t.is (uri4.port,     "",         "Uri::parse() : hostname:");
  t.is (uri4.path,     "",         "Uri::parse() : hostname:");
  t.is (uri4.protocol, "ssh",      "Uri::parse() : hostname:");
  t.notok (uri4.is_local (),       "Uri::is_local() : hostname:");
  t.ok (uri4.append ("file.test"), "Uri::append() : hostname:");
  t.is (uri4.path,     "file.test","Uri::append() : ok");

  context.config.set ("merge.default.uri", "../folder/");
  context.config.set ("push.test.uri",     "/home/user/.task/");

  Uri uri5 ("", "merge");
  t.ok (uri5.is_local (), "Uri::is_local() : ../server/");
  uri5.parse ();
  t.is (uri5.path,     "../folder/",      "Uri::expand() default");

  Uri uri6 ("test", "push");
  t.ok (uri6.is_local(), "Uri::is_local() : /home/user/.task/");
  uri6.parse ();
  t.is (uri6.path,     "/home/user/.task/", "Uri::expand() test");

  Uri uri7 ("ftp://'user@name'@host:321/path/to/x");
  uri7.parse ();
  t.is (uri7.user,      "user@name", "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7.host,      "host",      "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7.port,      "321",       "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7.path,      "path/to/x", "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7.protocol,  "ftp",       "Uri::parse() : ftp://'user@name'@host:321/path/to/x");

  Uri uri8 ("http://'us/er@n:ame'@host/path/to/x");
  uri8.parse ();
  t.is (uri8.user,      "us/er@n:ame", "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8.host,      "host",        "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8.port,      "",            "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8.path,      "path/to/x",   "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8.protocol,  "http",        "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");

  Uri uri9 ("'user@name'@host:path/to/x");
  uri9.parse ();
  t.is (uri9.user,  "user@name",   "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9.host,  "host",        "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9.port,  "",            "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9.path,  "path/to/x",   "Uri::parse() : 'user@name'@host:path/to/x");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
