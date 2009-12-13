////////////////////////////////////////////////////////////////////////////////
// Copyright 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "Context.h"
#include "main.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (24);

  // 1,2,3  <=>  2,3,4
  std::vector <std::string> string_one;
  string_one.push_back ("1");
  string_one.push_back ("2");
  string_one.push_back ("3");

  std::vector <std::string> string_two;
  string_two.push_back ("2");
  string_two.push_back ("3");
  string_two.push_back ("4");

  std::vector <std::string> string_three;
  string_three.push_back ("2");
  string_three.push_back ("3");
  string_three.push_back ("4");

  std::vector <std::string> string_four;

  // Differences?
  t.ok (!listDiff (string_one, string_one),   "std::string (1,2,3) == (1,2,3)");
  t.ok (listDiff (string_one, string_two),    "std::string (1,2,3) != (2,3,4)");
  t.ok (listDiff (string_one, string_three),  "std::string (1,2,3) != (2,3,4)");
  t.ok (listDiff (string_one, string_four),   "std::string (1,2,3) != ()");
  t.ok (!listDiff (string_four, string_four), "std::string () == ()");

  // What are the differences?
  std::vector<std::string> string_leftOnly;
  std::vector<std::string> string_rightOnly;
  listDiff (string_one, string_two, string_leftOnly, string_rightOnly);
  t.is ((int) string_leftOnly.size (), 1, "std::string (1,2,3) <=> (2,3,4) = 1<-");
  t.is (string_leftOnly[0], "1",          "std::string (1,2,3) <=> (2,3,4) = 1<-");

  t.is ((int) string_rightOnly.size (), 1, "std::string (1,2,3) <=> (2,3,4) = ->4");
  t.is (string_rightOnly[0], "4",          "std::string (1,2,3) <=> (2,3,4) = ->4");

  // What is the intersection?
  std::vector <std::string> string_join;
  listIntersect (string_one, string_two, string_join);
  t.is ((int) string_join.size (), 2, "std::string (1,2,3) intersect (2,3,4) = (2,3)");
  t.is (string_join[0], "2",          "std::string (1,2,3) intersect (2,3,4) = (2,3)");
  t.is (string_join[1], "3",          "std::string (1,2,3) intersect (2,3,4) = (2,3)");

  // Now do it all again, with integers.

  // 1,2,3  <=>  2,3,4
  std::vector <int> int_one;
  int_one.push_back (1);
  int_one.push_back (2);
  int_one.push_back (3);

  std::vector <int> int_two;
  int_two.push_back (2);
  int_two.push_back (3);
  int_two.push_back (4);

  std::vector <int> int_three;
  int_three.push_back (2);
  int_three.push_back (3);
  int_three.push_back (4);

  std::vector <int> int_four;

  // Differences?
  t.ok (!listDiff (int_one, int_one),   "int (1,2,3) == (1,2,3)");
  t.ok (listDiff (int_one, int_two),    "int (1,2,3) != (2,3,4)");
  t.ok (listDiff (int_one, int_three),  "int (1,2,3) != (2,3,4)");
  t.ok (listDiff (int_one, int_four),   "int (1,2,3) != ()");
  t.ok (!listDiff (int_four, int_four), "int () == ()");

  // What are the differences?
  std::vector<int> int_leftOnly;
  std::vector<int> int_rightOnly;
  listDiff (int_one, int_two, int_leftOnly, int_rightOnly);
  t.is ((int) int_leftOnly.size (), 1, "int (1,2,3) <=> (2,3,4) = 1<-");
  t.is (int_leftOnly[0], "1",          "int (1,2,3) <=> (2,3,4) = 1<-");

  t.is ((int) int_rightOnly.size (), 1, "int (1,2,3) <=> (2,3,4) = ->4");
  t.is (int_rightOnly[0], "4",          "int (1,2,3) <=> (2,3,4) = ->4");

  // What is the intersection?
  std::vector <int> int_join;
  listIntersect (int_one, int_two, int_join);
  t.is ((int) int_join.size (), 2, "int (1,2,3) intersect (2,3,4) = (2,3)");
  t.is (int_join[0], "2",          "int (1,2,3) intersect (2,3,4) = (2,3)");
  t.is (int_join[1], "3",          "int (1,2,3) intersect (2,3,4) = (2,3)");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

