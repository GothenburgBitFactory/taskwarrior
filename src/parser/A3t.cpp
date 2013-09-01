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

#include <iostream>
#include <A3t.h>
#include <Nibbler.h>
#include <Directory.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

static const int minimumMatchLength = 3;

////////////////////////////////////////////////////////////////////////////////
A3t::A3t (int argc, char** argv)
{
  _tree = new Tree ("root");
  if (! _tree)
    throw std::string ("Failed to allocate memory for parse tree.");

  for (int i = 0; i < argc; ++i)
  {
    Tree* branch = _tree->addBranch (new Tree (format ("arg{1}", i)));
    branch->attribute ("raw", argv[i]);
    branch->tag ("ORIGINAL");
    branch->tag ("?");
  }
}

////////////////////////////////////////////////////////////////////////////////
A3t::~A3t ()
{
}

////////////////////////////////////////////////////////////////////////////////
Tree* A3t::parse ()
{
  findBinary ();
  findTerminator ();
  findCommand ();
  findFileOverride ();
  findConfigOverride ();
  findSubstitution ();
  findPattern ();
  findTag ();
  findAttribute ();
  findAttributeModifier ();
  findUUIDList ();           // Before findIdSequence
  findIdSequence ();
  findOperator ();

  validate ();

  return _tree;
}

////////////////////////////////////////////////////////////////////////////////
void A3t::entity (const std::string& name, const std::string& value)
{
  _entities.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities, return category and canonicalized value.
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
// Walk the top-level tree branches, looking for the first raw value that
// autoCompletes to a valid command/report.
void A3t::findBinary ()
{
  if (_tree->_branches.size () >= 1)
  {
    _tree->_branches[0]->unTag ("?");
    _tree->_branches[0]->tag ("BINARY");
    std::string binary = _tree->_branches[0]->attribute ("raw");
    std::string::size_type slash = binary.rfind ('/');
    if (slash != std::string::npos)
      binary = binary.substr (slash + 1);

    _tree->_branches[0]->attribute ("basename", binary);

    if (binary == "cal" || binary == "calendar")
      _tree->_branches[0]->tag ("CALENDAR");
    else if (binary == "task" || binary == "tw" || binary == "t")
      _tree->_branches[0]->tag ("TW");
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
    }
  } 
}

////////////////////////////////////////////////////////////////////////////////
// Walk the top-level tree branches, looking for the first raw value that
// autoCompletes to a valid command/report.
void A3t::findCommand ()
{
  std::string command;
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
  {
    // Parser override operator.
    if ((*i)->attribute ("raw") == "--")
      break;

    // Skip known args.
    if (! (*i)->hasTag ("?"))
      continue;

    if (canonicalize (command, "report", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("REPORT");
      (*i)->attribute ("canonical", command);
    }

    else if (canonicalize (command, "readcmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("READCMD");
      (*i)->attribute ("canonical", command);
    }

    else if (canonicalize (command, "writecmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("WRITECMD");
      (*i)->attribute ("canonical", command);
    }

    else if (canonicalize (command, "specialcmd", (*i)->attribute ("raw")))
    {
      (*i)->unTag ("?");
      (*i)->tag ("CMD");
      (*i)->tag ("SPECIALCMD");
      (*i)->attribute ("canonical", command);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3t::findFileOverride ()
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
  }
}

////////////////////////////////////////////////////////////////////////////////
// rc.<name>[:=]<value>
void A3t::findConfigOverride ()
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
    if (arg.find ("rc.") == 0)
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
// /pattern/
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
      (*i)->attribute ("pattern", pattern);
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
          if (canonicalize (canonical, "attribute", name))
          {
            (*i)->unTag ("?");
            (*i)->tag ("ATTRIBUTE");
            (*i)->attribute ("name", canonical);
            (*i)->attribute ("value", value);
          }

          if (canonicalize (canonical, "uda", name))
            (*i)->tag ("UDA");

          if (canonicalize (canonical, "pseudo", name))
          {
            (*i)->unTag ("?");
            (*i)->tag ("PSEUDO");
            (*i)->attribute ("name", canonical);
            (*i)->attribute ("value", value);
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
              (*i)->attribute ("value", value);
              (*i)->attribute ("modifier", modifier);
              (*i)->attribute ("sense", sense);
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
      std::vector <std::pair <int, int> >::iterator r;
      for (r = ranges.begin (); r != ranges.end (); ++r)
      {
        Tree* branch = (*i)->addBranch (new Tree ("range"));
        branch->attribute ("min", r->first);
        branch->attribute ("max", r->second);
      }
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
      std::vector <std::string>::iterator u;
      for (u = sequence.begin (); u != sequence.end (); ++u)
      {
        Tree* branch = (*i)->addBranch (new Tree ("list"));
        branch->attribute ("value", *u);
      }
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
// Validate the parse tree.
void A3t::validate ()
{
  // Look for any unrecognized original args.
  std::vector <Tree*>::iterator i;
  for (i = _tree->_branches.begin (); i != _tree->_branches.end (); ++i)
    if ((*i)->hasTag ("?"))
      // TODO Restore the exception, when functionality is high enough to
      //      tolerate it.
      //throw std::string ("Unrecognized argument '") + (*i)->attribute ("raw") + "'";
      std::cout << "Unrecognized argument '" << (*i)->attribute ("raw") << "'\n";

  // TODO Any RC node must have a root/*[+RC]/data[@file] that exists.
}

////////////////////////////////////////////////////////////////////////////////
