////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <unistd.h>
#include <Context.h>
#include <A3t.h>
#include <Nibbler.h>
#include <Directory.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

#ifdef FEATURE_STDIN
#include <sys/select.h>
#endif

extern Context context;

// TODO Tie this to rc.abbreviation.minimum.
static const int minimumMatchLength = 3;

// Alias expansion limit.  Any more indicates some kind of error.
const int safetyValveDefault = 10;

////////////////////////////////////////////////////////////////////////////////
A3t::A3t ()
{
  _tree = new Tree ("root");
  if (! _tree)
    throw std::string ("Failed to allocate memory for parse tree.");
}

////////////////////////////////////////////////////////////////////////////////
A3t::~A3t ()
{
  delete _tree;
}

////////////////////////////////////////////////////////////////////////////////
// char** argv --> std::vector <std::string> _args
void A3t::initialize (int argc, const char** argv)
{
  // Create top-level nodes.
  for (int i = 0; i < argc; ++i)
  {
    Tree* branch = _tree->addBranch (new Tree (format ("arg{1}", i)));
    branch->attribute ("raw", argv[i]);
    branch->tag ("ORIGINAL");
    branch->tag ("?");
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::clear ()
{
  delete _tree;

  _tree = new Tree ("root");
  if (! _tree)
    throw std::string ("Failed to allocate memory for parse tree.");
}

////////////////////////////////////////////////////////////////////////////////
// Add an arg for every word from std::cin.
//
// echo one two -- three | task zero --> task zero one two
// 'three' is left in the input buffer.
void A3t::appendStdin ()
{
#ifdef FEATURE_STDIN
  // Use 'select' to determine whether there is any std::cin content buffered
  // before trying to read it, to prevent blocking.
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;

  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (STDIN_FILENO, &fds);

  int result = select (STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  if (result && result != -1)
  {
    if (FD_ISSET (0, &fds))
    {
      int i = 0;
      std::string arg;
      while (std::cin >> arg)
      {
        // It the terminator token is found, stop reading.
        if (arg == "--")
          break;

        Tree* branch = _tree->addBranch (new Tree (format ("stdin{1}", i++)));
        branch->attribute ("raw", arg);
        branch->tag ("ORIGINAL");
        branch->tag ("STDIN");
        branch->tag ("?");
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
Tree* A3t::tree ()
{
  return _tree;
}

////////////////////////////////////////////////////////////////////////////////
Tree* A3t::parse ()
{
  findTerminator ();
  findSubstitution ();
  findPattern ();
  findTag ();
  findAttribute ();
  findAttributeModifier ();
  findOperator ();
  findFilter ();
  findModifications ();

  findPlainArgs ();
  findMissingOperators ();

  validate ();

  return _tree;
}

////////////////////////////////////////////////////////////////////////////////
void A3t::entity (const std::string& name, const std::string& value)
{
  _entities.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities category, return canonicalized value.
bool A3t::canonicalize (
  std::string& canonicalized,
  const std::string& category,
  const std::string& value) const
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range (category);

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    options.push_back (e->second);

  // Match against the options, throw away results.
  std::vector <std::string> matches;
  if (autoComplete (value, options, matches, minimumMatchLength) == 1)
  {
    canonicalized = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate and tag the binary.
void A3t::findBinary ()
{
  if (_tree->_branches.size () >= 1)
  {
    _tree->_branches[0]->unTag ("?");
    _tree->_branches[0]->tag ("BINARY");
    std::string binary = _tree->_branches[0]->attribute ("raw");
    std::string::size_type slash = binary.rfind ('/');
    if (slash != std::string::npos)
    {
      binary = binary.substr (slash + 1);
    }

    _tree->_branches[0]->attribute ("basename", binary);

    if (binary == "cal" || binary == "calendar")
    {
      _tree->_branches[0]->tag ("CALENDAR");
    }
    else if (binary == "task" || binary == "tw" || binary == "t")
    {
      _tree->_branches[0]->tag ("TW");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// The parser override operator terminates all subsequent cleverness, leaving
// all args in the raw state.
void A3t::findTerminator ()
{
  bool found = false;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->attribute ("raw") == "--")
    {
      (*i)->unTag ("?");
      (*i)->tag ("TERMINATOR");
      found = true;
    }
    else if (found)
    {
      (*i)->unTag ("?");
      (*i)->tag ("WORD");
      (*i)->tag ("TERMINATED");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Walk the top-level tree branches, looking for the first raw value that
// autoCompletes to a valid command/report.
void A3t::findCommand ()
{
  // There can be only one.
  // Scan for an existing CMD tag, to short-circuit scanning for another.
  std::string command;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
    if ((*i)->hasTag ("CMD"))
      return;

  // No CMD tag found, now look for a command.
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

/*
    if (canonicalize (command, "report", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("REPORT");
      (*i)->attribute ("canonical", command);
      break;
    }
*/

    else if (canonicalize (command, "readcmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("READCMD");
      (*i)->attribute ("canonical", command);
      break;
    }

    else if (canonicalize (command, "writecmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("WRITECMD");
      (*i)->attribute ("canonical", command);
      break;
    }

    else if (canonicalize (command, "helper", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("HELPER");
      (*i)->attribute ("canonical", command);
      break;
    }

/*
    else if (canonicalize (command, "specialcmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("SPECIALCMD");
      (*i)->attribute ("canonical", command);
      break;
    }
*/
  }
}

////////////////////////////////////////////////////////////////////////////////
// rc:<file>
// rc.<name>[:=]<value>
void A3t::findOverrides ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string arg = (*i)->attribute ("raw");
    if (arg.find ("rc:") == 0)
    {
      (*i)->unTag ("?");
      (*i)->tag ("RC");
      (*i)->attribute ("file", arg.substr (3));
    }
    else if (arg.find ("rc.") == 0)
    {
      std::string::size_type sep = arg.find ('=', 3);
      if (sep == std::string::npos)
        sep = arg.find (':', 3);

      if (sep != std::string::npos)
      {
        (*i)->unTag ("?");
        (*i)->tag ("CONFIG");
        (*i)->attribute ("name", arg.substr (3, sep - 3));
        (*i)->attribute ("value", arg.substr (sep + 1));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Look for RC and return file as a File.
void A3t::getOverrides (
  std::string& home,
  File& rc)
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("RC"))
    {
      rc = File ((*i)->attribute ("file"));
      home = rc;

      std::string::size_type last_slash = rc._data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc._data.substr (0, last_slash);
      else
        home = ".";

      context.header (format (STRING_A3_ALTERNATE_RC, rc._data));

      // Keep looping, because if there are multiple rc:file arguments, we
      // want the last one to dominate.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Look for CONFIG data.location and return value as a Path.
void A3t::getDataLocation (Path& data)
{
  std::string location = context.config.get ("data.location");
  if (location != "")
    data = location;

  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("CONFIG") &&
        (*i)->attribute ("name") == "data.location")
    {
      data = Directory ((*i)->attribute ("value"));
      context.header (format (STRING_A3_ALTERNATE_DATA, (std::string) data));
    }

    // Keep looping, because if there are multiple overrides, we want the last
    // one to dominate.
  }
}

////////////////////////////////////////////////////////////////////////////////
// Takes all CONFIG name/value pairs and overrides configuration.
// leaving only the plain args.
void A3t::applyOverrides ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("CONFIG"))
    {
      std::string name  = (*i)->attribute ("name");
      std::string value = (*i)->attribute ("value");
      context.config.set (name, value);
      context.footnote (format (STRING_A3_OVERRIDE_RC, name, value));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::injectDefaults ()
{
  // Scan the top-level branches for evidence of ID, UUID, overrides and other
  // arguments.
  bool found_command  = false;
  bool found_sequence = false;
  bool found_other    = false;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("BINARY"))
      ;

    else if ((*i)->hasTag ("CMD"))
      found_command = true;

    else if ((*i)->hasTag ("ID") || (*i)->hasTag ("UUID"))
      found_sequence = true;

    else if (! (*i)->hasTag ("RC") &&
             ! (*i)->hasTag ("CONFIG"))
      found_other = true;
  }

  // If no command was specified, then a command will be inserted.
  if (! found_command)
  {
    // Default command.
    if (! found_sequence)
    {
      // Apply overrides, if any.
      std::string defaultCommand = context.config.get ("default.command");
      if (defaultCommand != "")
      {
        context.debug ("No command or sequence found - assuming default.command.");
        Tree* t = captureFirst (defaultCommand);
        t->tag ("DEFAULT");

        std::string combined;
        std::vector <Tree*>::iterator i;
        for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
        {
          if (combined.length ())
            combined += ' ';

          combined += (*i)->attribute ("raw");
        }

        context.header ("[" + combined + "]");
      }
      else
      {
        throw std::string (STRING_TRIVIAL_INPUT);
      }
    }
    else
    {
      // TODO There is a problem, in that this block is not run.

      // Information command.
      if (! found_other)
      {
        context.debug ("Sequence but no command found - assuming 'information' command.");
        context.header (STRING_ASSUME_INFO);
        Tree* t = captureFirst ("information");
        t->tag ("ASSUMED");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
Tree* A3t::captureFirst (const std::string& arg)
{
  // Insert the arg as the new first branch.
  Tree* t = new Tree ("argIns");
  if (!t)
    throw std::string (STRING_ERROR_MEMORY);

  t->attribute ("raw", arg);
  t->tag ("?");
  t->_trunk = _tree;

  std::vector <Tree*>::iterator i = _tree->_branches.begin ();
  if (i != _tree->_branches.end ())
    i++;  // Walk past the binary.

  _tree->_branches.insert (i, t);

  findCommand ();
  return t;
}

////////////////////////////////////////////////////////////////////////////////
const std::string A3t::getFilterExpression ()
{
  // Construct an efficient ID/UUID clause.
  std::string sequence = "";
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("FILTER") && ! (*i)->hasTag ("PSEUDO"))
    {
      if ((*i)->_branches.size ())
      {
        std::vector <Tree*>::iterator b;
        for (b = (*i)->_branches.begin (); b != (*i)->_branches.end (); ++b)
        {
          if (sequence != "")
            sequence += " ";

          sequence += (*b)->attribute ("raw");
        }
      }
      else
      {
        if (sequence != "")
          sequence += " ";

        sequence += (*i)->attribute ("raw");
      }
    }
  }

  if (sequence != "")
    sequence = "( " + sequence + " )";

  if (context.verbose ("filter"))
    context.footnote ("Filter: " + sequence);

  return sequence;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> A3t::getWords () const
{
  std::vector <std::string> words;
  std::vector <Tree*>::const_iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if (! (*i)->hasTag ("BINARY") &&
        ! (*i)->hasTag ("RC")     &&
        ! (*i)->hasTag ("CONFIG") &&
        ! (*i)->hasTag ("CMD")    &&
        ! (*i)->hasTag ("TERMINATOR"))
      words.push_back ((*i)->attribute ("raw"));
  }

  return words;
}

////////////////////////////////////////////////////////////////////////////////
// /pattern/ --> description ~ pattern
void A3t::findPattern ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    Nibbler n ((*i)->attribute ("raw"));
    std::string pattern;
    if (n.getQuoted ('/', pattern) &&
        n.depleted () &&
        pattern.length () > 0)
    {
      (*i)->unTag ("?");
      (*i)->tag ("PATTERN");

      Tree* branch = (*i)->addBranch (new Tree ("argPat"));
      branch->attribute ("raw", "description");

      branch = (*i)->addBranch (new Tree ("argPat"));
      branch->attribute ("raw", "~");
      branch->tag ("OP");

      branch = (*i)->addBranch (new Tree ("argPat"));
      branch->attribute ("raw", pattern);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// /from/to/[g]
void A3t::findSubstitution ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    std::string from;
    std::string to;
    bool global = false;
    if (n.getQuoted ('/', from) &&
        n.backN ()              &&
        n.getQuoted ('/', to))
    {
      if (n.skip ('g'))
        global = true;

      if (n.depleted () &&
          !Directory (raw).exists ())
      {
        (*i)->unTag ("?");
        (*i)->tag ("SUBSTITUTION");
        (*i)->attribute ("from", from);
        (*i)->attribute ("to", to);
        (*i)->attribute ("global", global ? 1 : 0);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// +tag
void A3t::findTag ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    std::string tag;
    std::string sign;
    if (n.getN (1, sign)             &&
        (sign == "+" || sign == "-") &&
        n.getUntilEOS (tag)          &&
        tag.find (' ') == std::string::npos)
    {
      (*i)->unTag ("?");
      (*i)->tag ("TAG");
      (*i)->attribute ("sign", sign);
      (*i)->attribute ("tag", tag);

      Tree* branch = (*i)->addBranch (new Tree ("argTag"));
      branch->attribute ("raw", "tags");

      branch = (*i)->addBranch (new Tree ("argTag"));
      branch->attribute ("raw", (sign == "+" ? "_hastag_" : "_notag_"));
      branch->tag ("OP");

      branch = (*i)->addBranch (new Tree ("argTag"));
      branch->attribute ("raw", tag);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"][<value>]['"]
void A3t::findAttribute ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    // Look for a valid attribute name.
    std::string name;
    if (n.getName (name) &&
        name.length ())
    {
      if (n.skip (':'))
      {
        std::string value;
        if (n.getQuoted   ('"', value)  ||
            n.getQuoted   ('\'', value) ||
            n.getUntilEOS (value))
        {
          std::string canonical;
          if (canonicalize (canonical, "uda", name))
          {
            (*i)->tag ("UDA");
            (*i)->tag ("MODIFIABLE");
          }

          else if (canonicalize (canonical, "pseudo", name))
          {
            (*i)->unTag ("?");
            (*i)->tag ("PSEUDO");
            (*i)->attribute ("name", canonical);
            (*i)->attribute ("raw", value);
          }

          else if (canonicalize (canonical, "attribute", name))
          {
            (*i)->unTag ("?");
            (*i)->tag ("ATTRIBUTE");
            (*i)->attribute ("name", canonical);
            (*i)->attribute ("raw", value);

            std::map <std::string, Column*>::const_iterator col;
            col = context.columns.find (canonical);
            if (col != context.columns.end () &&
                col->second->modifiable ())
            {
              (*i)->tag ("MODIFIABLE");
            }

            Tree* branch = (*i)->addBranch (new Tree ("argAtt"));
            branch->attribute ("raw", canonical);

            branch = (*i)->addBranch (new Tree ("argAtt"));
            branch->attribute ("raw", "==");
            branch->tag ("OP");

            branch = (*i)->addBranch (new Tree ("argAtt"));
            branch->attribute ("raw", value);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// <name>.<mod>[:=]['"]<value>['"]
void A3t::findAttributeModifier ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    std::string name;
    if (n.getUntil (".", name))
    {
      std::string canonical;
      if (canonicalize (canonical, "attribute", name) ||
          canonicalize (canonical, "uda",       name))
      {
        if (n.skip ('.'))
        {
          std::string sense = "+";
          if (n.skip ('~'))
            sense = "-";

          std::string modifier;
          n.getUntilOneOf (":=", modifier);

          if (n.skip (':') ||
              n.skip ('='))
          {
            std::string value;
            if (n.getQuoted   ('"', value)  ||
                n.getQuoted   ('\'', value) ||
                n.getUntilEOS (value))
            {
              (*i)->unTag ("?");
              (*i)->tag ("ATTMOD");
              (*i)->attribute ("name", canonical);
              (*i)->attribute ("raw", value);
              (*i)->attribute ("modifier", modifier);
              (*i)->attribute ("sense", sense);

              Tree* branch = (*i)->addBranch (new Tree ("argAttmod"));
              branch->attribute ("raw", canonical);

              if (modifier == "before" || modifier == "under" || modifier == "below")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "<");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "after" || modifier == "over" || modifier == "above")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", ">");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "none")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "==");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "''");
              }
              else if (modifier == "any")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "!=");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "''");
              }
              else if (modifier == "is" || modifier == "equals")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "==");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "isnt" || modifier == "not")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "!=");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "has" || modifier == "contains")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "hasnt")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "!~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", value);
              }
              else if (modifier == "startswith" || modifier == "left")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "'^" + value + "'");
              }
              else if (modifier == "endswith" || modifier == "right")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "'" + value + "$'");
              }
              else if (modifier == "word")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));

#if defined (DARWIN)
                branch->attribute ("raw", value);
#elif defined (SOLARIS)
                branch->attribute ("raw", "'\\<" + value + "\\>'");
#else
                branch->attribute ("raw", "'\\b" + value + "\\b'");
#endif
              }
              else if (modifier == "noword")
              {
                branch = (*i)->addBranch (new Tree ("argAttmod"));
                branch->attribute ("raw", "!~");
                branch->tag ("OP");

                branch = (*i)->addBranch (new Tree ("argAttmod"));

#if defined (DARWIN)
                branch->attribute ("raw", value);
#elif defined (SOLARIS)
                branch->attribute ("raw", "'\\<" + value + "\\>'");
#else
                branch->attribute ("raw", "'\\b" + value + "\\b'");
#endif
              }
              else
                throw format (STRING_A3_UNKNOWN_ATTMOD, modifier);

              std::map <std::string, Column*>::const_iterator col;
              col = context.columns.find (canonical);
              if (col != context.columns.end () &&
                  col->second->modifiable ())
              {
                (*i)->tag ("MODIFIABLE");
              }
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// A sequence can be:
//
//   a single ID:          1
//   a list of IDs:        1,3,5
//   a list of IDs:        1 3 5
//   a range:              5-10
//   or a combination:     1,3,5-10 12
//
// If a sequence is followed by a non-number, then subsequent numbers are not
// interpreted as IDs.  For example:
//
//    1 2 three 4
//
// The sequence is "1 2".
//
void A3t::findIdSequence ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    std::vector <std::pair <int, int> > ranges;
    int id;
    if (n.getUnsignedInt (id))
    {
      if (n.skip ('-'))
      {
        int end;
        if (!n.getUnsignedInt (end))
          throw std::string (STRING_A3_ID_AFTER_HYPHEN);

        if (id > end)
          throw std::string (STRING_A3_RANGE_INVERTED);

        ranges.push_back (std::pair <int, int> (id, end));
      }
      else
        ranges.push_back (std::pair <int, int> (id, id));

      while (n.skip (','))
      {
        if (n.getUnsignedInt (id))
        {
          if (n.skip ('-'))
          {
            int end;
            if (!n.getUnsignedInt (end))
              throw std::string (STRING_A3_ID_AFTER_HYPHEN);

            if (id > end)
              throw std::string (STRING_A3_RANGE_INVERTED);

            ranges.push_back (std::pair <int, int> (id, end));
          }
          else
            ranges.push_back (std::pair <int, int> (id, id));
        }
        else
          throw std::string (STRING_A3_MALFORMED_ID);
      }

      (*i)->unTag ("?");
      (*i)->tag ("ID");

      Tree* branch = (*i)->addBranch (new Tree ("argSeq"));
      branch->attribute ("raw", "(");
      branch->tag ("OP");

      std::vector <std::pair <int, int> >::iterator r;
      for (r = ranges.begin (); r != ranges.end (); ++r)
      {
        if (r != ranges.begin ())
        {
          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "or");
          branch->tag ("OP");
        }

        if (r->first == r->second)
        {
          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "id");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "==");
          branch->tag ("OP");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", r->first);
        }
        else
        {
          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "(");
          branch->tag ("OP");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "id");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", ">=");
          branch->tag ("OP");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", r->first);

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "and");
          branch->tag ("OP");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "id");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "<=");
          branch->tag ("OP");

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", r->second);

          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", ")");
          branch->tag ("OP");
        }
      }

      branch = (*i)->addBranch (new Tree ("argSeq"));
      branch->attribute ("raw", ")");
      branch->tag ("OP");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::findUUIDList ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");
    Nibbler n (raw);

    std::vector <std::string> sequence;
    std::string uuid;
    if (n.getUUID (uuid) ||
        n.getPartialUUID (uuid))
    {
      sequence.push_back (uuid);

      while (n.skip (','))
      {
        if (!n.getUUID (uuid) &&
            !n.getPartialUUID (uuid))
          throw std::string (STRING_A3_UUID_AFTER_COMMA);

        sequence.push_back (uuid);
      }

      if (!n.depleted ())
        throw std::string (STRING_A3_PATTERN_GARBAGE);

      (*i)->unTag ("?");
      (*i)->tag ("UUID");

      Tree* branch = (*i)->addBranch (new Tree ("argSeq"));
      branch->attribute ("raw", "(");
      branch->tag ("OP");

      std::vector <std::string>::iterator u;
      for (u = sequence.begin (); u != sequence.end (); ++u)
      {
        if (u != sequence.begin ())
        {
          branch = (*i)->addBranch (new Tree ("argSeq"));
          branch->attribute ("raw", "or");
          branch->tag ("OP");
        }

        branch = (*i)->addBranch (new Tree ("argSeq"));
        branch->attribute ("raw", "uuid");

        branch = (*i)->addBranch (new Tree ("argSeq"));
        branch->attribute ("raw", "=");
        branch->tag ("OP");

        branch = (*i)->addBranch (new Tree ("argSeq"));
        branch->attribute ("raw", *u);
      }

      branch = (*i)->addBranch (new Tree ("argSeq"));
      branch->attribute ("raw", ")");
      branch->tag ("OP");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::findOperator ()
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range ("operator");

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    options.push_back (e->second);

  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    std::string raw = (*i)->attribute ("raw");

    std::vector <std::string>::iterator opt;
    for (opt = options.begin (); opt != options.end (); ++opt)
    {
      if (*opt == raw)
      {
        (*i)->unTag ("?");
        (*i)->tag ("OP");
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Anything before CMD, but not BINARY, RC or CONFIG --> FILTER
// Anything after READCMD, but not BINARY, RC or CONFIG --> FILTER
void A3t::findFilter ()
{
  bool before_cmd = true;
  bool after_readcmd = false;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    if ((*i)->hasTag ("CMD"))
      before_cmd = false;

    if ((*i)->hasTag ("READCMD"))
      after_readcmd = true;

    if (before_cmd &&
        ! (*i)->hasTag ("CMD") &&
        ! (*i)->hasTag ("BINARY") &&
        ! (*i)->hasTag ("RC") &&
        ! (*i)->hasTag ("CONFIG"))
    {
      (*i)->unTag ("?");
      (*i)->tag ("FILTER");
    }

    if (after_readcmd &&
        ! (*i)->hasTag ("CMD") &&
        ! (*i)->hasTag ("BINARY") &&
        ! (*i)->hasTag ("RC") &&
        ! (*i)->hasTag ("CONFIG"))
    {
      (*i)->unTag ("?");
      (*i)->tag ("FILTER");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::findModifications ()
{
  bool after_writecmd = false;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("WRITECMD"))
      after_writecmd = true;

    if (after_writecmd &&
        ! (*i)->hasTag ("CMD") &&
        ! (*i)->hasTag ("TERMINATOR") &&
        ! (*i)->hasTag ("BINARY") &&
        ! (*i)->hasTag ("RC") &&
        ! (*i)->hasTag ("CONFIG"))
    {
      (*i)->unTag ("?");
      (*i)->tag ("MODIFICATION");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// This is called after parsing. The intention is to find plain arguments that
// are not otherwise recognized, and potentially promote them to patterns.
void A3t::findPlainArgs ()
{
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("FILTER")   &&
        (*i)->hasTag ("ORIGINAL") &&  // TODO Wrong, can come in through alias/filter
        (*i)->countTags () == 2)
    {
      std::cout << "# plain arg '" << (*i)->attribute ("raw") << "'\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::findMissingOperators ()
{
  while (insertOr ())
    ;

  while (insertAnd ())
    ;
}

////////////////////////////////////////////////////////////////////////////////
// Two consecutive ID/UUID arguments get an 'or' inserted between them.
bool A3t::insertOr ()
{
  std::vector <Tree*>::iterator prev = _tree->_branches.begin ();
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if ((*i)->hasTag ("?"))
      continue;

    if ((*i)->hasTag ("FILTER") && ! (*i)->hasTag ("PSEUDO"))
    {
      if (prev != _tree->_branches.begin () &&
          ((*prev)->hasTag ("ID") || (*prev)->hasTag ("UUID")) &&
          ((*i)->hasTag ("ID") || (*i)->hasTag ("UUID")))
      {
        Tree* branch = new Tree ("argOp");
        branch->attribute ("raw", "or");
        branch->tag ("OP");
        branch->tag ("FILTER");

        _tree->_branches.insert (i, branch);
        return true;
      }
    }

    prev = i;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Two consecutive FILTER, non-OP arguments that are not "(" or ")" need an
// "and" operator inserted between them.
//
//   ) <non-op>         -->  ) and <non-op>
//   <non-op> (         -->  <non-op> <and> (
//   ) (                -->  ) and (
//   <non-op> <non-op>  -->  <non-op> and <non-op>
//
bool A3t::insertAnd ()
{
  std::vector <Tree*>::iterator prev = _tree->_branches.begin ();
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("FILTER") && ! (*i)->hasTag ("PSEUDO"))
    {
      if ((*i)->_branches.size ())
      {
        std::vector <Tree*>::iterator sub;
        for (sub = (*i)->_branches.begin (); sub != (*i)->_branches.end (); ++sub)
        {
          if (sub != prev &&
              prev != _tree->_branches.begin () &&
              (! (*prev)->hasTag ("OP") || (*prev)->attribute ("raw") == ")") &&
              (! (*sub)->hasTag ("OP") || (*sub)->attribute ("raw") == "("))
          {
            std::cout << "# missingOperator '"
                      << (*prev)->attribute ("raw")
                      << " "
                      << (*sub)->attribute ("raw")
                      << "' --> '"
                      << (*prev)->attribute ("raw")
                      << " and "
                      << (*sub)->attribute ("raw")
                      << "'\n";

            Tree* branch = new Tree ("argOp");
            branch->attribute ("raw", "and");
            branch->tag ("OP");
            branch->tag ("FILTER");
            (*i)->_branches.insert (sub, branch);
            return true;
          }

          prev = sub;
        }
      }
      else
      {
        if (i != prev &&
            prev != _tree->_branches.begin () &&
            (! (*prev)->hasTag ("OP") || (*prev)->attribute ("raw") == ")") &&
            (! (*i)->hasTag ("OP") || (*i)->attribute ("raw") == "("))
        {
          std::cout << "# missingOperator '"
                    << (*prev)->attribute ("raw")
                    << " "
                    << (*i)->attribute ("raw")
                    << "' --> '"
                    << (*prev)->attribute ("raw")
                    << " and "
                    << (*i)->attribute ("raw")
                    << "'\n";

          Tree* branch = new Tree ("argOp");
          branch->attribute ("raw", "and");
          branch->tag ("OP");
          branch->tag ("FILTER");
          _tree->_branches.insert (i, branch);
          return true;
        }

        prev = i;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Validate the parse tree.
void A3t::validate ()
{
  // Look for any unrecognized original args.
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
    if ((*i)->hasTag ("?"))
      //throw std::string ("Unrecognized argument '") + (*i)->attribute ("raw") + "'";
      //std::cout << "Unrecognized argument '" << (*i)->attribute ("raw") << "'\n";
      context.debug ("Unrecognized argument '" + (*i)->attribute ("raw") + "'");

  // TODO Any RC node must have a root/+RC @file that exists.
  // TODO There must be a root/+CMD.
}

////////////////////////////////////////////////////////////////////////////////
