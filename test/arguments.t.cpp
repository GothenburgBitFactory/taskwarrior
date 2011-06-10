////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <Arguments.h>
#include <text.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (113);

  const char* fake[] =
  {
    // task list proj:foo 1,3,5-10 12 pattern1 rc.name1=value1 rc.name2:value2 \
    //   234 -- pattern2 n:v
    "task",                // context.program
    "list",                // command
    "proj:foo",            // n:v
    "1,3,5-10",            // sequence
    "12",                  // sequence
    "pattern1",            // text
    "rc.name1=value1",     // n:v
    "rc.name2:value2",     // n:v
    "234",                 // text
    "--",                  // terminator
    "pattern2",            // text
    "n:v",                 // text (due to terminator)
  };

  // void capture (int, char**);
  Arguments a1;
  a1.capture (12, &fake[0]);
  t.is (a1.size (), (size_t)12, "12 arguments expected");

  // std::string combine ();
  t.is (a1.combine (),
        "task list proj:foo 1,3,5-10 12 pattern1 rc.name1=value1 rc.name2:value2 "
        "234 -- pattern2 n:v",
        "combine good");

  // bool is_attr (const std::string&);
  t.ok (Arguments::is_attr ("name:"),                       "name:               -> attr");
  t.ok (Arguments::is_attr ("name:\"\""),                   "name:\"\"             -> attr");
  t.ok (Arguments::is_attr ("name:one"),                    "name:one            -> attr");
  t.ok (Arguments::is_attr ("name:\"one\""),                "name:\"one\"          -> attr");
  t.ok (Arguments::is_attr ("name:\"one two\""),            "name:\"one two\"      -> attr");

  t.ok (Arguments::is_attr ("name="),                       "name=               -> attr");
  t.ok (Arguments::is_attr ("name=\"\""),                   "name=\"\"             -> attr");
  t.ok (Arguments::is_attr ("name=one"),                    "name=one            -> attr");
  t.ok (Arguments::is_attr ("name=\"one\""),                "name=\"one\"          -> attr");
  t.ok (Arguments::is_attr ("name=\"one two\""),            "name=\"one two\"      -> attr");

  t.notok (Arguments::is_attr ("name"),                     "name                -> not attr");
  t.notok (Arguments::is_attr ("(name=val and 1<2)"),      "(name=val and 1<2)   -> not attr");

  // bool is_attmod (const std::string&);
  t.ok (Arguments::is_attmod ("name.is:"),                  "name.is:            -> attmod");
  t.ok (Arguments::is_attmod ("name.is:\"\""),              "name.is:\"\"          -> attmod");
  t.ok (Arguments::is_attmod ("name.is:one"),               "name.is:one         -> attmod");
  t.ok (Arguments::is_attmod ("name.is:\"one\""),           "name.is:\"one\"       -> attmod");
  t.ok (Arguments::is_attmod ("name.is:\"one two\""),       "name.is:\"one two\"   -> attmod");

  t.ok (Arguments::is_attmod ("name.is="),                  "name.is=            -> attmod");
  t.ok (Arguments::is_attmod ("name.is=\"\""),              "name.is=\"\"          -> attmod");
  t.ok (Arguments::is_attmod ("name.is=one"),               "name.is=one         -> attmod");
  t.ok (Arguments::is_attmod ("name.is=\"one\""),           "name.is=\"one\"       -> attmod");
  t.ok (Arguments::is_attmod ("name.is=\"one two\""),       "name.is=\"one two\"   -> attmod");

  t.notok (Arguments::is_attmod ("name"),                   "name                -> not attmod");
  t.notok (Arguments::is_attmod ("(name=value and 1<2"),    "(name=value and 1<2 -> not attmod");

  // bool is_subst (const std::string&);
  t.notok (Arguments::is_subst ("///"),                     "///                 -> not subst");
  t.notok (Arguments::is_subst ("//to/"),                   "//to/               -> not subst");
  t.ok    (Arguments::is_subst ("/from//"),                 "/from//             -> subst");
  t.ok    (Arguments::is_subst ("/from/to/"),               "/from/to/           -> subst");
  t.ok    (Arguments::is_subst ("/from from/to to/"),       "/from from/to to/   -> subst");

  t.notok (Arguments::is_subst ("///g"),                    "///g                -> not subst");
  t.notok (Arguments::is_subst ("//to/g"),                  "//to/g              -> not subst");
  t.ok    (Arguments::is_subst ("/from//g"),                "/from//g            -> subst");
  t.ok    (Arguments::is_subst ("/from/to/g"),              "/from/to/g          -> subst");
  t.ok    (Arguments::is_subst ("/from from/to to/g"),      "/from from/to to/g  -> subst");

  // bool is_pattern (const std::string&);
  t.notok (Arguments::is_pattern ("//"),                    "//                  -> not pattern");
  t.notok (Arguments::is_pattern ("///"),                   "///                 -> not pattern");
  t.ok    (Arguments::is_pattern ("/one/"),                 "/one/               -> pattern");
  t.ok    (Arguments::is_pattern ("/one two/"),             "/one two/           -> pattern");
  t.ok    (Arguments::is_pattern ("/  /"),                  "/  /                -> pattern");

  // bool is_id (const std::string&);
  t.ok    (Arguments::is_id ("1"),                          "1                   -> id");
  t.ok    (Arguments::is_id ("1,2"),                        "1,2                 -> id");
  t.ok    (Arguments::is_id ("1-2"),                        "1-2                 -> id");
  t.ok    (Arguments::is_id ("1,2,3"),                      "1,2,3               -> id");
  t.ok    (Arguments::is_id ("1-2,3,4-5"),                  "1-2,3,4-5           -> id");
  t.notok (Arguments::is_id ("1-2-3"),                      "1-2-3               -> no id");

  // bool is_uuid (const std::string&);
  t.ok    (Arguments::is_uuid ("00000000-0000-0000-0000-000000000000"),
                               "00000000-0000-0000-0000-000000000000 -> uuid");
  t.ok    (Arguments::is_uuid ("eeeeeeee-eeee-eeee-eeee-eeeeeeeeeeee,ffffffff-ffff-ffff-ffff-ffffffffffff"),
                               "eeeeeeee-eeee-eeee-eeee-eeeeeeeeeeee,ffffffff-ffff-ffff-ffff-ffffffffffff -> uuid");

  // bool is_tag (const std::string&);
  t.ok    (Arguments::is_tag ("+one"),                      "+one                -> tag");
  t.ok    (Arguments::is_tag ("-one"),                      "-one                -> tag");
  t.notok (Arguments::is_tag ("+one "),                     "+one                -> not tag");
  t.notok (Arguments::is_tag ("-one "),                     "-one                -> not tag");
  t.notok (Arguments::is_tag ("+"),                         "+                   -> not tag");
  t.notok (Arguments::is_tag ("-"),                         "-                   -> not tag");
  t.notok (Arguments::is_tag ("++"),                        "++                  -> not tag");
  t.notok (Arguments::is_tag ("--"),                        "--                  -> not tag");
  t.notok (Arguments::is_tag ("+one two"),                  "+one two            -> not tag");

  // bool is_operator (const std::string&);
  t.ok    (Arguments::is_operator ("^"),   "^   -> operator");
  t.ok    (Arguments::is_operator ("!"),   "!   -> operator");
  t.ok    (Arguments::is_operator ("not"), "not -> operator");
  t.ok    (Arguments::is_operator ("-"),   "-   -> operator");
  t.ok    (Arguments::is_operator ("*"),   "*   -> operator");
  t.ok    (Arguments::is_operator ("/"),   "/   -> operator");
  t.ok    (Arguments::is_operator ("%"),   "%   -> operator");
  t.ok    (Arguments::is_operator ("+"),   "+   -> operator");
  t.ok    (Arguments::is_operator ("-"),   "-   -> operator");
  t.ok    (Arguments::is_operator ("<"),   "<   -> operator");
  t.ok    (Arguments::is_operator ("lt"),  "lt  -> operator");
  t.ok    (Arguments::is_operator ("<="),  "<=  -> operator");
  t.ok    (Arguments::is_operator ("le"),  "le  -> operator");
  t.ok    (Arguments::is_operator (">="),  ">=  -> operator");
  t.ok    (Arguments::is_operator ("ge"),  "ge  -> operator");
  t.ok    (Arguments::is_operator (">"),   ">   -> operator");
  t.ok    (Arguments::is_operator ("gt"),  "gt  -> operator");
  t.ok    (Arguments::is_operator ("~"),   "~   -> operator");
  t.ok    (Arguments::is_operator ("!~"),  "!~  -> operator");
  t.ok    (Arguments::is_operator ("="),   "=   -> operator");
  t.ok    (Arguments::is_operator ("eq"),  "eq  -> operator");
  t.ok    (Arguments::is_operator ("!="),  "!=  -> operator");
  t.ok    (Arguments::is_operator ("ne"),  "ne  -> operator");
  t.ok    (Arguments::is_operator ("and"), "and -> operator");
  t.ok    (Arguments::is_operator ("or"),  "or  -> operator");
  t.ok    (Arguments::is_operator ("("),   "(   -> operator");
  t.ok    (Arguments::is_operator (")"),   ")   -> operator");
  t.notok (Arguments::is_operator ("$"),   "$   -> not operator");

  // bool is_expression (const std::string&);
  t.notok (Arguments::is_expression ("foo"),   "foo      -> expression");
  t.ok    (Arguments::is_expression ("1+1"),   "1+1      -> expression");
  t.ok    (Arguments::is_expression ("a~b"),   "a~b      -> expression");
  t.ok    (Arguments::is_expression ("(1)"),   "(1)      -> expression");
  t.ok    (Arguments::is_expression ("not a"), "not a    -> expression");

  // static bool valid_modifier (const std::string&);
  t.ok    (Arguments::valid_modifier ("before"),        "before      -> modifier");
  t.ok    (Arguments::valid_modifier ("under"),         "under       -> modifier");
  t.ok    (Arguments::valid_modifier ("below"),         "below       -> modifier");
  t.ok    (Arguments::valid_modifier ("after"),         "after       -> modifier");
  t.ok    (Arguments::valid_modifier ("over"),          "over        -> modifier");
  t.ok    (Arguments::valid_modifier ("above"),         "above       -> modifier");
  t.ok    (Arguments::valid_modifier ("none"),          "none        -> modifier");
  t.ok    (Arguments::valid_modifier ("any"),           "any         -> modifier");
  t.ok    (Arguments::valid_modifier ("is"),            "is          -> modifier");
  t.ok    (Arguments::valid_modifier ("equals"),        "equals      -> modifier");
  t.ok    (Arguments::valid_modifier ("isnt"),          "isnt        -> modifier");
  t.ok    (Arguments::valid_modifier ("not"),           "not         -> modifier");
  t.ok    (Arguments::valid_modifier ("has"),           "has         -> modifier");
  t.ok    (Arguments::valid_modifier ("contains"),      "contains    -> modifier");
  t.ok    (Arguments::valid_modifier ("hasnt"),         "hasnt       -> modifier");
  t.ok    (Arguments::valid_modifier ("startswith"),    "startswith  -> modifier");
  t.ok    (Arguments::valid_modifier ("left"),          "left        -> modifier");
  t.ok    (Arguments::valid_modifier ("endswith"),      "endswith    -> modifier");
  t.ok    (Arguments::valid_modifier ("right"),         "right       -> modifier");
  t.ok    (Arguments::valid_modifier ("word"),          "word        -> modifier");
  t.ok    (Arguments::valid_modifier ("noword"),        "noword      -> modifier");
  t.notok (Arguments::valid_modifier ("duck"),          "duck        -> not modified");

  // TODO void extract_uuids (std::vector <std::string>&);
  // TODO void extract_filter ();
  // TODO void extract_modifications ();
  // TODO void extract_text ();

  // TODO bool extract_attr (const std::string&, std::string&, std::string&);
  // TODO bool extract_attmod (const std::string&, std::string&, std::string&, std::string&, std::string&);
  // TODO bool extract_subst (const std::string&, std::string&, std::string&, bool&);
  // TODO bool extract_pattern (const std::string&, std::string&);
  // TODO bool extract_id (const std::string&, std::vector <int>&);
  // TODO bool extract_uuid (const std::string&, std::vector <std::string>&);
  // TODO bool extract_tag (const std::string&, char&, std::string&);
  // TODO bool extract_operator (const std::string&, std::string&);

  // TODO Arguments extract_read_only_filter ();
  // TODO Arguments extract_write_filter ();
  // TODO Arguments extract_modifications ();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

