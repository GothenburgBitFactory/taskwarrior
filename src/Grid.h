////////////////////////////////////////////////////////////////////////////////
// Copyright 2007 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_GRID
#define INCLUDED_GRID

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
class Grid
{
public:
  class Cell
  {
  public:
    Cell (const bool);
    Cell (const char);
    Cell (const int);
    Cell (const float);
    Cell (const double);
    Cell (const std::string&);

    operator bool () const;
    operator char () const;
    operator int () const;
    operator float () const;
    operator double () const;
    operator std::string () const;

    enum cellType {CELL_BOOL, CELL_CHAR, CELL_INT, CELL_FLOAT, CELL_DOUBLE, CELL_STRING};

    cellType type () const;

  private:
    cellType mType;
    bool mBool;
    char mChar;
    int mInt;
    float mFloat;
    double mDouble;
    std::string mString;
  };

public:
  Grid ();
  ~Grid ();

  void add (const unsigned int, const unsigned int, const bool);
  void add (const unsigned int, const unsigned int, const char);
  void add (const unsigned int, const unsigned int, const int);
  void add (const unsigned int, const unsigned int, const float);
  void add (const unsigned int, const unsigned int, const double);
  void add (const unsigned int, const unsigned int, const std::string&);

  unsigned int width () const;
  unsigned int height () const;

  Cell* byRow    (const unsigned int, const unsigned int) const;
  Cell* byColumn (const unsigned int, const unsigned int) const;

private:
  void expandGrid (const unsigned int, const unsigned int);
  void insertCell (const unsigned int, const unsigned int, Cell*);

private:
  std::vector < std::vector <Cell*>* > mRows;
  std::vector < std::vector <Cell*>* > mColumns;
};

#endif
////////////////////////////////////////////////////////////////////////////////

