////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <Thread.h>

////////////////////////////////////////////////////////////////////////////////
Thread::Thread ()
{
}

////////////////////////////////////////////////////////////////////////////////
Thread::~Thread ()
{
}

////////////////////////////////////////////////////////////////////////////////
int Thread::start (void* inArg)
{
  mArg = inArg;
  return pthread_create (&mTID, NULL, (void*(*)(void*)) Thread::entryPoint, (void*) this);
}

////////////////////////////////////////////////////////////////////////////////
void Thread::wait ()
{
  pthread_join (mTID, NULL);
}

////////////////////////////////////////////////////////////////////////////////
void Thread::cancel ()
{
  pthread_cancel (mTID);
}

////////////////////////////////////////////////////////////////////////////////
void Thread::detach ()
{
  pthread_detach (mTID);
}

////////////////////////////////////////////////////////////////////////////////
void* Thread::entryPoint (void* inThis)
{
  Thread* p = (Thread*) inThis;
  p->execute (p->arg ());
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void* Thread::arg ()
{
  return mArg;
}

////////////////////////////////////////////////////////////////////////////////
