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
#ifndef INCLUDED_TLSCLIENT
#define INCLUDED_TLSCLIENT

#ifdef HAVE_LIBGNUTLS

#include <string>
#include <gnutls/gnutls.h>

class TLSClient
{
public:
  TLSClient ();
  ~TLSClient ();
  void limit (int);
  void debug (int);
  void trust (bool);
  void init (const std::string&, const std::string&);
  void connect (const std::string&, const std::string&);
  void bye ();

  void send (const std::string&);
  void recv (std::string&);

private:
  std::string                      _ca;
  std::string                      _cert;
  std::string                      _key;
  gnutls_certificate_credentials_t _credentials;
  gnutls_session_t                 _session;
  int                              _socket;
  int                              _limit;
  bool                             _debug;
};

#endif
#endif

////////////////////////////////////////////////////////////////////////////////

