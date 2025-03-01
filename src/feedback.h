////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2025, Tomas Babej, Paul Beckingham, Federico Hernandez,
//                        Tobias Predel.
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

#ifndef INCLUDED_FEEDBACK
#define INCLUDED_FEEDBACK

#include <cmake.h>
// cmake.h include header must come first

#include <Task.h>

#include <string>
#include <vector>

std::string renderAttribute(const std::string& name, const std::string& value,
                            const std::string& format = "");
void feedback_affected(const std::string& effect);
void feedback_affected(const std::string& effect, int quantity);
void feedback_affected(const std::string& effect, const Task& task);
void feedback_reserved_tags(const std::string& tag);
void feedback_special_tags(const Task& task, const std::string& tag);
void feedback_unblocked(const Task& task);
void feedback_backlog();
std::string onProjectChange(Task& task, bool scope = true);
std::string onProjectChange(Task& task1, Task& task2);
std::string onExpiration(Task& task);

#endif

////////////////////////////////////////////////////////////////////////////////
