////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_STLMACROS
#define INCLUDED_STLMACROS

#define foreach(i, c)                                              \
for (typeof (c) *foreach_p = & (c);                                \
     foreach_p;                                                    \
     foreach_p = 0)                                                \
  for (typeof (foreach_p->begin()) i = foreach_p->begin();         \
       i != foreach_p->end();                                      \
       ++i)

#endif
////////////////////////////////////////////////////////////////////////////////
