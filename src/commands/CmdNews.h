////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_CMDNEWS
#define INCLUDED_CMDNEWS

#include <CmdConfig.h>
#include <CmdContext.h>
#include <Command.h>
#include <Version.h>

#include <string>

class NewsItem {
 public:
  Version _version;
  std::string _title;
  std::string _bg_title;
  std::string _background;
  std::string _punchline;
  std::string _update;
  std::string _reasoning;
  std::string _actions;

  void render();

  static std::vector<NewsItem> all();
  static void version2_6_0(std::vector<NewsItem>&);
  static void version3_0_0(std::vector<NewsItem>&);
  static void version3_1_0(std::vector<NewsItem>&);
  static void version3_2_0(std::vector<NewsItem>&);
  static void version3_3_0(std::vector<NewsItem>&);
  static void version3_4_0(std::vector<NewsItem>&);

 private:
  NewsItem(Version, const std::string&, const std::string& = "", const std::string& = "",
           const std::string& = "", const std::string& = "", const std::string& = "",
           const std::string& = "");
};

class CmdNews : public Command {
 public:
  CmdNews();
  int execute(std::string&);

  static bool should_nag();
};

#endif
////////////////////////////////////////////////////////////////////////////////
