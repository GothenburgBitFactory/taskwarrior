////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TASKMOD
#define INCLUDED_TASKMOD
#define L10N                                           // Localization complete.

#include <string>
#include <Task.h>

class Taskmod {
  friend bool compareTaskmod (Taskmod first, Taskmod second);

public:
  Taskmod ();
  Taskmod (int resourceID);
  Taskmod (const Taskmod& other);
  ~Taskmod ();

  // operators
  bool operator< (const Taskmod& compare);
  bool operator> (const Taskmod& compare);
  bool operator== (const Taskmod& compare);
  bool operator!= (const Taskmod& compare);
  Taskmod& operator= (const Taskmod& other);

  // helper
  void reset (long timestamp=0);
  bool isNew ();
  bool issetBefore ();
  bool issetAfter ();
  bool isValid ();

  std::string getUuid ();
  std::string toString ();

  // setter
  void setAfter (const Task& after);
  void setBefore (const Task& before);
  void setTimestamp (long timestamp);
  void incSequenceNumber ();

  // getter
  Task& getAfter ();
  Task& getBefore ();
  long getTimestamp () const;
  unsigned long getSequenceNumber () const;
  int getResource () const;
  std::string getTimeStr () const;

protected:
  Task _after;
  Task _before;
  long _timestamp;
  bool _bAfterSet;
  bool _bBeforeSet;
  unsigned long _sequenceNumber;
  int  _resource;

  static unsigned long curSequenceNumber;
};

bool compareTaskmod (Taskmod first, Taskmod second);

#endif

