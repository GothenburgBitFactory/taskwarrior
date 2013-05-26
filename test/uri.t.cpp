////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2013, Johannes Schlatow.
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
#include <iostream>
#include <sstream>
#include <Context.h>
#include <Uri.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (54);

  Uri uri1 ("asfd://user@host/folder/");
  uri1.parse ();
  t.is (uri1._user,      "user",    "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1._host,      "host",    "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1._port,      "",        "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1._path,      "folder/", "Uri::parse() : asdf://user@host/folder/");
  t.is (uri1._protocol,  "asfd",    "Uri::parse() : asdf://user@host/folder/");
  t.ok (uri1.append ("file.test"),          "Uri::append() to path");
  t.is (uri1._path,      "folder/file.test", "Uri::append() ok");

  Uri uri2 ("user@host:folder/file.test");
  uri2.parse ();
  t.is (uri2._user,      "user",             "Uri::parse() : user@host:folder/file.test");
  t.is (uri2._host,      "host",             "Uri::parse() : user@host:folder/file.test");
  t.is (uri2._port,      "",                 "Uri::parse() : user@host/folder/file.test");
  t.is (uri2._path,      "folder/file.test", "Uri::parse() : user@host/folder/file.test");
  t.is (uri2._protocol,  "ssh",              "Uri::parse() : user@host/folder/file.test");
  t.notok (uri2.append ("test.dat"),        "Uri::append() to file");

  Uri uri3 ("rsync://hostname.abc.de:1234//abs/path");
  uri3.parse ();
  t.is (uri3._user,     "",                "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3._host,     "hostname.abc.de", "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3._port,     "1234",            "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3._path,     "/abs/path",       "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");
  t.is (uri3._protocol, "rsync",           "Uri::parse() : rsync://hostname.abc.de:1234//abs/path");

  Uri uri4 ("hostname:");
  uri4.parse ();
  t.is (uri4._user,     "",         "Uri::parse() : hostname:");
  t.is (uri4._host,     "hostname", "Uri::parse() : hostname:");
  t.is (uri4._port,     "",         "Uri::parse() : hostname:");
  t.is (uri4._path,     "",         "Uri::parse() : hostname:");
  t.is (uri4._protocol, "ssh",      "Uri::parse() : hostname:");
  t.notok (uri4.is_local (),       "Uri::is_local() : hostname:");
  t.ok (uri4.append ("file.test"), "Uri::append() : hostname:");
  t.is (uri4._path,     "file.test","Uri::append() : ok");

  context.config.set ("merge.default.uri", "../folder/");
  context.config.set ("push.test.uri",     "/home/user/.task/");

  Uri uri5 ("", "merge");
  t.ok (uri5.is_local (), "Uri::is_local() : ../server/");
  uri5.parse ();
  t.is (uri5._path,     "../folder/",      "Uri::expand() default");

  Uri uri6 ("test", "push");
  t.ok (uri6.is_local(), "Uri::is_local() : /home/user/.task/");
  uri6.parse ();
  t.is (uri6._path,     "/home/user/.task/", "Uri::expand() test");

  Uri uri7 ("ftp://'user@name'@host:321/path/to/x");
  uri7.parse ();
  t.is (uri7._user,      "user@name", "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7._host,      "host",      "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7._port,      "321",       "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7._path,      "path/to/x", "Uri::parse() : ftp://'user@name'@host:321/path/to/x");
  t.is (uri7._protocol,  "ftp",       "Uri::parse() : ftp://'user@name'@host:321/path/to/x");

  Uri uri8 ("http://'us/er@n:ame'@host/path/to/x");
  uri8.parse ();
  t.is (uri8._user,      "us/er@n:ame", "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8._host,      "host",        "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8._port,      "",            "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8._path,      "path/to/x",   "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");
  t.is (uri8._protocol,  "http",        "Uri::parse() : http://'us/er@n:ame'@host/path/to/x");

  Uri uri9 ("'user@name'@host:path/to/x");
  uri9.parse ();
  t.is (uri9._user,  "user@name",   "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9._host,  "host",        "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9._port,  "",            "Uri::parse() : 'user@name'@host:path/to/x");
  t.is (uri9._path,  "path/to/x",   "Uri::parse() : 'user@name'@host:path/to/x");

  // bug #668
  Uri uri10 ("user.name@host.com:undo.data");
  uri10.parse ();
  t.is (uri10._user,  "user.name",  "Uri::parse() : user.name@host.com:undo.data");
  t.is (uri10._host,  "host.com",   "Uri::parse() : user.name@host.com:undo.data");
  t.is (uri10._port,  "",           "Uri::parse() : user.name@host.com:undo.data");
  t.is (uri10._path,  "undo.data",  "Uri::parse() : user.name@host.com:undo.data");
  t.is (uri10._protocol, "ssh",     "Uri::parse() : user.name@host.com:undo.data");

  Uri uri11 ("ssh://user.name@host.com/undo.data");
  uri11.parse ();
  t.is (uri11._user,  "user.name",  "Uri::parse() : ssh://user.name@host.com/undo.data");
  t.is (uri11._host,  "host.com",   "Uri::parse() : ssh://user.name@host.com/undo.data");
  t.is (uri11._port,  "",           "Uri::parse() : ssh://user.name@host.com/undo.data");
  t.is (uri11._path,  "/undo.data", "Uri::parse() : ssh://user.name@host.com/undo.data");
  t.is (uri11._protocol, "ssh",     "Uri::parse() : ssh://user.name@host.com/undo.data");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// vim: et ts=2 sw=2
