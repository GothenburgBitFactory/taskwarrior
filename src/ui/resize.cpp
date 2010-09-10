////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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
#include <map>
#include <vector>
#include <string>
#include <stdlib.h>
#include "Rectangle.h"
#include "Tree.h"
#include "Lisp.h"

static void recalcNode (Tree*, int, int, int, int);
static void distribute (const int, const std::vector <std::string>&, std::vector <int>&);
static void getRequests (Tree*, std::vector <std::string>&);

////////////////////////////////////////////////////////////////////////////////
void resize (
  const std::string& input,
  const int width,
  const int height,
  std::string& name,
  std::map <std::string, Rectangle>& panels)
{
  // Parse the input into a tree.
  Lisp l;
  Tree* root = l.parse (input);

  // Navigate the tree and calculate sizes.  This requires a breadth-first
  // traversal, so that calculations at one level are then propagated down.
  // After all, the window dimensions dictate the layout size, and so on down.
  Tree* layout = (*root)[0];
  layout->attribute ("left",   0);
  layout->attribute ("top",    0);
  layout->attribute ("width",  width);
  layout->attribute ("height", height);
  recalcNode (layout, 0, 0, width, height);

  // Extract the name of the layout.
  std::vector <std::string> tags = layout->allTags ();
  if (tags.size () >= 2)
    name = tags[1];
  else
    name = "anonymous";

  // Extract panels.
  panels.clear ();
  std::vector <Tree*> nodes;
  layout->enumerate (nodes);
  std::vector <Tree*>::iterator it;
  for (it = nodes.begin (); it != nodes.end (); ++it)
  {
    tags = (*it)->allTags ();
    if (tags[0] == "panel")
      panels[tags[1]] =
        Rectangle (
          atoi ((*it)->attribute ("left").c_str ()),
          atoi ((*it)->attribute ("top").c_str ()),
          atoi ((*it)->attribute ("width").c_str ()),
          atoi ((*it)->attribute ("height").c_str ()));
  }

  delete root;
}

////////////////////////////////////////////////////////////////////////////////
// Specified node has associated width and height.  Subdivide either width or
// height across all tree branches depending on which container the parent is.
static void recalcNode (Tree* parent, int left, int top, int width, int height)
{
  // What kind of parent is this?  layout, horizontal or vertical?
  std::string parent_type = "?";
  std::vector <std::string> parent_tags = parent->allTags ();
  if (parent_tags.size ())
    parent_type = parent_tags[0];
  else
    throw std::string ("Error: node has no specified type");

  if (parent_type == "horizontal")
  {
    std::vector <std::string> requested;
    getRequests (parent, requested);

    std::vector <int> allocated;
    distribute (width, requested, allocated);

    int current_left = left;
    for (int i = 0; i < parent->branches (); ++i)
    {
      (*parent)[i]->attribute ("left", current_left);
      (*parent)[i]->attribute ("top", top);
      (*parent)[i]->attribute ("width", allocated[i]);
      (*parent)[i]->attribute ("height", height);
      current_left += allocated[i];
    }
  }

  else if (parent_type == "vertical")
  {
    std::vector <std::string> requested;
    getRequests (parent, requested);

    std::vector <int> allocated;
    distribute (height, requested, allocated);

    int current_top = top;
    for (int i = 0; i < parent->branches (); ++i)
    {
      (*parent)[i]->attribute ("left", left);
      (*parent)[i]->attribute ("top", current_top);
      (*parent)[i]->attribute ("width", width);
      (*parent)[i]->attribute ("height", allocated[i]);
      current_top += allocated[i];
    }
  }

  else if (parent_type == "layout")
  {
    if (! (*parent)[0])
      throw std::string ("Error: layout has no contents.");

    (*parent)[0]->attribute ("left",   left);
    (*parent)[0]->attribute ("top",    top);
    (*parent)[0]->attribute ("width",  width);
    (*parent)[0]->attribute ("height", height);
  }

  // Now recurse to each branch of parent that is a container.
  for (int i = 0; i < parent->branches (); ++i)
  {
    Tree* child = (*parent)[i];
    if (child->hasTag ("horizontal") ||
        child->hasTag ("vertical"))
    {
      recalcNode (child,
        atoi (child->attribute ("left").c_str ()),
        atoi (child->attribute ("top").c_str ()),
        atoi (child->attribute ("width").c_str ()),
        atoi (child->attribute ("height").c_str ()));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// If the call looks like this:
//   distribute (60, [10, *, *, *, *, *, 50%], [])
//
// Then the result is:
//   distribute (60, [10, *, 50%], [10, 5, 5, 5, 5, 5, 25])
//
// A literal number is a request.  It will be granted, provided there is space.
// A percentage is a request for a portion of the unallocated amount.
// A wildcard is a request for an equal share, among all wildcards, of the
// unallocated amount.
//
static void distribute (
  const int total,
  const std::vector <std::string>& input,
  std::vector <int>& output)
{
  int allocated   = 0;
  int unallocated = total;

  output.clear ();

  // First count up the requested.
  for (unsigned int i = 0; i < input.size (); ++i)
    if (input[i] == "*" || input[i].find ("%") != std::string::npos)
      output.push_back (0);
    else
    {
      int value = atoi (input[i].c_str ());
      output.push_back (value);
      allocated   += value;
      unallocated -= value;
    }

  if (allocated > total)
    throw std::string ("Error: over allocation by request.");

  // Now include the proportional.
  int before_allocated = allocated;
  for (unsigned int i = 0; i < input.size (); ++i)
  {
    if (input[i].find ("%") != std::string::npos)
    {
      int value = atoi (input[i].c_str ());
      value = (value * unallocated) / 100;
			output[i] = value;

      allocated   += value;
    }
  }

  unallocated -= (allocated - before_allocated);

  if (allocated > total)
    throw std::string ("Error: over allocation by request.");

  // Count the wildcards.
  int wildcards = 0;
  for (unsigned int i = 0; i < input.size (); ++i)
    if (input[i] == "*")
      ++wildcards;

  // Evenly distribute unallocated among the wildcards.
  for (unsigned int i = 0; i < input.size (); ++i)
  {
    if (input[i] == "*")
    {
      if (wildcards > 1)
      {
        int portion = unallocated / wildcards;
        --wildcards;

        output[i] = portion;

        allocated   += portion;
        unallocated -= portion;
      }
      else
      {
        output[i] = unallocated;
        allocated  += unallocated;
        unallocated = 0;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
static void getRequests (Tree* node, std::vector <std::string>& requested)
{
  requested.clear ();

  for (int i = 0; i < node->branches (); ++i)
  {
    Tree* child = (*node)[i];
    std::vector <std::string> child_tags = child->allTags ();

    switch (child_tags.size ())
    {
    case 1:  // (xcontainer (...))
      requested.push_back ("*");
      break;

    case 2:  // (xcontainer size (...))
      requested.push_back (child_tags[1]);
      break;

    case 3:  // (panel name size)
      requested.push_back (child_tags[2]);
      break;

    default:
      throw std::string ("Error: unexpected number of tags in a node");
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
