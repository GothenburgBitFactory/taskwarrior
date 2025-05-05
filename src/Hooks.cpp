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

#include <cmake.h>
// cmake.h include header must come first

#include <Hooks.h>

#include <algorithm>
// If <iostream> is included, put it after <stdio.h>, because it includes
// <stdio.h>, and therefore would ignore the _WITH_GETLINE.
#ifdef FREEBSD
#define _WITH_GETLINE
#endif
#include <Context.h>
#include <DOM.h>
#include <FS.h>
#include <JSON.h>
#include <Lexer.h>
#include <Timer.h>
#include <Variant.h>
#include <format.h>
#include <shared.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <util.h>

#define STRING_HOOK_ERROR_OBJECT "Hook Error: JSON Object '{...}' expected from hook script: {1}"
#define STRING_HOOK_ERROR_NODESC \
  "Hook Error: JSON Object missing 'description' attribute from hook script: {1}"
#define STRING_HOOK_ERROR_NOUUID \
  "Hook Error: JSON Object missing 'uuid' attribute from hook script: {1}"
#define STRING_HOOK_ERROR_SYNTAX "Hook Error: JSON syntax error in: {1}"
#define STRING_HOOK_ERROR_JSON "Hook Error: JSON "
#define STRING_HOOK_ERROR_NOPARSE "Hook Error: JSON failed to parse: "
#define STRING_HOOK_ERROR_BAD_NUM \
  "Hook Error: Expected {1} JSON task(s), found {2}, in hook script: {3}"
#define STRING_HOOK_ERROR_SAME1 \
  "Hook Error: JSON must be for the same task: {1}, in hook script: {2}"
#define STRING_HOOK_ERROR_SAME2 \
  "Hook Error: JSON must be for the same task: {1} != {2}, in hook script: {3}"
#define STRING_HOOK_ERROR_NOFEEDBACK "Hook Error: Expected feedback from failing hook script: {1}"

////////////////////////////////////////////////////////////////////////////////
void Hooks::initialize() {
  _debug = Context::getContext().config.getInteger("debug.hooks");

  // Scan <rc.hooks.location>
  //      <rc.data.location>/hooks
  Directory d;
  if (Context::getContext().config.has("hooks.location")) {
    d = Directory(Context::getContext().config.get("hooks.location"));
  } else {
    d = Directory(Context::getContext().config.get("data.location"));
    d += "hooks";
  }

  if (d.is_directory() && d.readable()) {
    _scripts = d.list();
    std::sort(_scripts.begin(), _scripts.end());

    if (_debug >= 1) {
      for (auto& i : _scripts) {
        Path p(i);
        if (!p.is_directory()) {
          std::string name = p.name();
          if (name.substr(0, 6) == "on-add" || name.substr(0, 9) == "on-modify" ||
              name.substr(0, 9) == "on-launch" || name.substr(0, 7) == "on-exit")
            Context::getContext().debug("Found hook script " + i);
          else
            Context::getContext().debug("Found misnamed hook script " + i);
        }
      }
    }
  } else if (_debug >= 1)
    Context::getContext().debug("Hook directory not readable: " + d._data);

  _enabled = Context::getContext().config.getBoolean("hooks");
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::enable(bool value) {
  bool old_value = _enabled;
  _enabled = value;
  return old_value;
}

////////////////////////////////////////////////////////////////////////////////
// The on-launch event is triggered once, after initialization, before any
// processing occurs, i.e first
//
// Input:
// - none
//
// Output:
// - JSON not allowed.
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onLaunch() const {
  if (!_enabled) return;

  Timer timer;

  std::vector<std::string> matchingScripts = scripts("on-launch");
  if (matchingScripts.size()) {
    for (auto& script : matchingScripts) {
      std::vector<std::string> input;
      std::vector<std::string> output;
      int status = callHookScript(script, input, output);

      std::vector<std::string> outputJSON;
      std::vector<std::string> outputFeedback;
      separateOutput(output, outputJSON, outputFeedback);

      assertNTasks(outputJSON, 0, script);

      if (status == 0) {
        for (auto& message : outputFeedback) Context::getContext().footnote(message);
      } else {
        assertFeedback(outputFeedback, script);
        for (auto& message : outputFeedback) Context::getContext().error(message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }
  }

  Context::getContext().time_hooks_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
// The on-exit event is triggered once, after all processing is complete, i.e.
// last
//
// Input:
// - read-only line of JSON for each task added/modified
//
// Output:
// - all emitted JSON is ignored
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onExit() const {
  if (!_enabled) return;

  Timer timer;

  std::vector<std::string> matchingScripts = scripts("on-exit");
  if (matchingScripts.size()) {
    // Get the set of changed tasks.
    std::vector<Task> tasks;
    Context::getContext().tdb2.get_changes(tasks);

    // Convert to a vector of strings.
    std::vector<std::string> input;
    input.reserve(tasks.size());
    for (auto& t : tasks) input.push_back(t.composeJSON());

    // Call the hook scripts, with the invariant input.
    for (auto& script : matchingScripts) {
      std::vector<std::string> output;
      int status = callHookScript(script, input, output);

      std::vector<std::string> outputJSON;
      std::vector<std::string> outputFeedback;
      separateOutput(output, outputJSON, outputFeedback);

      assertNTasks(outputJSON, 0, script);

      if (status == 0) {
        for (auto& message : outputFeedback) Context::getContext().footnote(message);
      } else {
        assertFeedback(outputFeedback, script);
        for (auto& message : outputFeedback) Context::getContext().error(message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }
  }

  Context::getContext().time_hooks_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
// The on-add event is triggered separately for each task added
//
// Input:
// - line of JSON for the task added
//
// Output:
// - emitted JSON for the input task is added, if the exit code is zero,
//   otherwise ignored.
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onAdd(Task& task) const {
  if (!_enabled) return;

  Timer timer;

  std::vector<std::string> matchingScripts = scripts("on-add");
  if (matchingScripts.size()) {
    // Convert task to a vector of strings.
    std::vector<std::string> input;
    input.push_back(task.composeJSON());

    // Call the hook scripts.
    for (auto& script : matchingScripts) {
      std::vector<std::string> output;
      int status = callHookScript(script, input, output);

      std::vector<std::string> outputJSON;
      std::vector<std::string> outputFeedback;
      separateOutput(output, outputJSON, outputFeedback);

      if (status == 0) {
        assertNTasks(outputJSON, 1, script);
        assertValidJSON(outputJSON, script);
        assertSameTask(outputJSON, task, script);

        // Propagate forward to the next script.
        input[0] = outputJSON[0];

        for (auto& message : outputFeedback) Context::getContext().footnote(message);
      } else {
        assertFeedback(outputFeedback, script);
        for (auto& message : outputFeedback) Context::getContext().error(message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }

    // Transfer the modified task back to the original task.
    task = Task(input[0]);
  }

  Context::getContext().time_hooks_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
// The on-modify event is triggered separately for each task added or modified
//
// Input:
// - line of JSON for the original task
// - line of JSON for the modified task, the diff being the modification
//
// Output:
// - emitted JSON for the input task is saved, if the exit code is zero,
//   otherwise ignored.
// - all emitted non-JSON lines are considered feedback or error messages
//   depending on the status code.
//
void Hooks::onModify(Task& before, Task& after) const {
  if (!_enabled) return;

  Timer timer;

  std::vector<std::string> matchingScripts = scripts("on-modify");
  if (matchingScripts.size()) {
    // Convert vector of tasks to a vector of strings.
    std::vector<std::string> input;
    input.push_back(before.composeJSON());  // [line 0] original, never changes
    input.push_back(after.composeJSON());   // [line 1] modified

    // Call the hook scripts.
    for (auto& script : matchingScripts) {
      std::vector<std::string> output;
      int status = callHookScript(script, input, output);

      std::vector<std::string> outputJSON;
      std::vector<std::string> outputFeedback;
      separateOutput(output, outputJSON, outputFeedback);

      if (status == 0) {
        assertNTasks(outputJSON, 1, script);
        assertValidJSON(outputJSON, script);
        assertSameTask(outputJSON, before, script);

        // Propagate accepted changes forward to the next script.
        input[1] = outputJSON[0];

        for (auto& message : outputFeedback) Context::getContext().footnote(message);
      } else {
        assertFeedback(outputFeedback, script);
        for (auto& message : outputFeedback) Context::getContext().error(message);

        throw 0;  // This is how hooks silently terminate processing.
      }
    }

    after = Task(input[1]);
  }

  Context::getContext().time_hooks_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> Hooks::list() const { return _scripts; }

////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> Hooks::scripts(const std::string& event) const {
  std::vector<std::string> matching;
  for (const auto& i : _scripts) {
    if (i.find("/" + event) != std::string::npos) {
      File script(i);
      if (script.executable()) matching.push_back(i);
    }
  }

  return matching;
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::separateOutput(const std::vector<std::string>& output, std::vector<std::string>& json,
                           std::vector<std::string>& feedback) const {
  for (auto& i : output) {
    if (isJSON(i))
      json.push_back(i);
    else
      feedback.push_back(i);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::isJSON(const std::string& input) const {
  return input.length() > 2 && input[0] == '{' && input[input.length() - 1] == '}';
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertValidJSON(const std::vector<std::string>& input,
                            const std::string& script) const {
  for (auto& i : input) {
    if (i.length() < 3 || i[0] != '{' || i[i.length() - 1] != '}') {
      Context::getContext().error(format(STRING_HOOK_ERROR_OBJECT, Path(script).name()));
      throw 0;
    }

    try {
      json::value* root = json::parse(i);
      if (root->type() != json::j_object) {
        Context::getContext().error(format(STRING_HOOK_ERROR_OBJECT, Path(script).name()));
        throw 0;
      }

      if (((json::object*)root)->_data.find("description") == ((json::object*)root)->_data.end()) {
        Context::getContext().error(format(STRING_HOOK_ERROR_NODESC, Path(script).name()));
        throw 0;
      }

      if (((json::object*)root)->_data.find("uuid") == ((json::object*)root)->_data.end()) {
        Context::getContext().error(format(STRING_HOOK_ERROR_NOUUID, Path(script).name()));
        throw 0;
      }

      delete root;
    }

    catch (const std::string& e) {
      Context::getContext().error(format(STRING_HOOK_ERROR_SYNTAX, i));
      if (_debug) Context::getContext().error(STRING_HOOK_ERROR_JSON + e);
      throw 0;
    }

    catch (...) {
      Context::getContext().error(STRING_HOOK_ERROR_NOPARSE + i);
      throw 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertNTasks(const std::vector<std::string>& input, unsigned int n,
                         const std::string& script) const {
  if (input.size() != n) {
    Context::getContext().error(
        format(STRING_HOOK_ERROR_BAD_NUM, n, (int)input.size(), Path(script).name()));
    throw 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertSameTask(const std::vector<std::string>& input, const Task& task,
                           const std::string& script) const {
  std::string uuid = task.get("uuid");

  for (auto& i : input) {
    auto root_obj = (json::object*)json::parse(i);

    // If there is no UUID at all.
    auto u = root_obj->_data.find("uuid");
    if (u == root_obj->_data.end() || u->second->type() != json::j_string) {
      Context::getContext().error(format(STRING_HOOK_ERROR_SAME1, uuid, Path(script).name()));
      throw 0;
    }

    auto up = (json::string*)u->second;
    auto text = up->dump();
    Lexer::dequote(text);
    std::string json_uuid = json::decode(text);
    if (json_uuid != uuid) {
      Context::getContext().error(
          format(STRING_HOOK_ERROR_SAME2, uuid, json_uuid, Path(script).name()));
      throw 0;
    }

    delete root_obj;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::assertFeedback(const std::vector<std::string>& input, const std::string& script) const {
  bool foundSomething = false;
  for (auto& i : input)
    if (nontrivial(i)) foundSomething = true;

  if (!foundSomething) {
    Context::getContext().error(format(STRING_HOOK_ERROR_NOFEEDBACK, Path(script).name()));
    throw 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::vector<std::string>& Hooks::buildHookScriptArgs(std::vector<std::string>& args) const {
  Variant v;

  // Hooks API version.
  args.push_back("api:2");

  // Command line Taskwarrior was called with.
  getDOM("context.args", v);
  args.push_back("args:" + std::string(v));

  // Command to be executed.
  args.push_back("command:" + Context::getContext().cli2.getCommand());

  // rc file used after applying all overrides.
  args.push_back("rc:" + Context::getContext().rc_file._data);

  // Directory containing *.data files.
  args.push_back("data:" + Context::getContext().data_dir._data);

  // Taskwarrior version, same as returned by "task --version"
  args.push_back("version:" + std::string(VERSION));

  return args;
}

////////////////////////////////////////////////////////////////////////////////
int Hooks::callHookScript(const std::string& script, const std::vector<std::string>& input,
                          std::vector<std::string>& output) const {
  if (_debug >= 1) Context::getContext().debug("Hook: Calling " + script);

  if (_debug >= 2) {
    Context::getContext().debug("Hook: input");
    for (const auto& i : input) Context::getContext().debug("  " + i);
  }

  std::string inputStr;
  for (const auto& i : input) inputStr += i + "\n";

  std::vector<std::string> args;
  buildHookScriptArgs(args);
  if (_debug >= 2) {
    Context::getContext().debug("Hooks: args");
    for (const auto& arg : args) Context::getContext().debug("  " + arg);
  }

  // Measure time for each hook if running in debug
  int status;
  std::string outputStr;
  if (_debug >= 2) {
    Timer timer;
    status = execute(script, args, inputStr, outputStr);
    Context::getContext().debugTiming(format("Hooks::execute ({1})", script), timer);
  } else
    status = execute(script, args, inputStr, outputStr);

  output = split(outputStr, '\n');

  if (_debug >= 2) {
    Context::getContext().debug("Hook: output");
    for (const auto& i : output)
      if (i != "") Context::getContext().debug("  " + i);

    Context::getContext().debug(format("Hook: Completed with status {1}", status));
    Context::getContext().debug(" ");  // Blank line
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
