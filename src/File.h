////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_FILE
#define INCLUDED_FILE

#include <stdio.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <Path.h>

class File : public Path
{
public:
  File ();
  File (const Path&);
  File (const File&);
  File (const std::string&);
  virtual ~File ();

  File& operator= (const File&);

  virtual bool create (int mode = 0640);
  virtual bool remove () const;

  bool open ();
  bool openAndLock ();
  void close ();

  bool lock ();
  bool waitForLock ();

  void read (std::string&);
  void read (std::vector <std::string>&);

  void write (const std::string&);
  void write (const std::vector <std::string>&);

  void append (const std::string&);
  void append (const std::vector <std::string>&);

  void truncate ();

  virtual mode_t mode ();
  virtual size_t size () const;
  virtual time_t mtime () const;
  virtual time_t ctime () const;
  virtual time_t btime () const;

  static bool create (const std::string&, int mode = 0640);
  static std::string read (const std::string&);
  static bool read (const std::string&, std::string&);
  static bool read (const std::string&, std::vector <std::string>&);
  static bool write (const std::string&, const std::string&);
  static bool write (const std::string&, const std::vector <std::string>&, bool addNewlines = true);
  static bool append (const std::string&, const std::string&);
  static bool append (const std::string&, const std::vector <std::string>&, bool addNewlines = true);
  static bool remove (const std::string&);

private:
  FILE* _fh;
  int   _h;
  bool  _locked;
};

#endif
////////////////////////////////////////////////////////////////////////////////
