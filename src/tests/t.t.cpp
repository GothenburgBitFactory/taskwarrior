////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
#include "../T.h"
#include "../task.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  plan (4);

  T t;
  std::string s = t.compose ();
  is ((int)s.length (), 46, "T::T (); T::compose ()");
  diag (s);

  t.setStatus (T::completed);
  s = t.compose ();
  is (s[37], '+', "T::setStatus (completed)");
  diag (s);

  t.setStatus (T::deleted);
  s = t.compose ();
  is (s[37], 'X', "T::setStatus (deleted)");
  diag (s);

  // Round trip test.
  std::string sample = "00000000-0000-0000-0000-000000000000 - [] [] Sample";
  T t2;
  t2.parse (sample);
  sample += "\n";
  is (t2.compose (), sample, "T::parse -> T::compose round trip");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

