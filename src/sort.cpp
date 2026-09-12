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

#include <Context.h>
#include <Duration.h>
#include <Task.h>
#include <format.h>
#include <shared.h>
#include <stdlib.h>
#include <util.h>

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

static const std::vector<Task>* global_data = nullptr;
static unsigned int sort_random_seed = 0;
static bool sort_compare(int, int);

struct SortKey {
  std::string field;
  bool ascending;
};

static std::vector<SortKey> global_sort_keys;

// Pre-computed values to avoid repeated parsing. Duration fields are parsed
// once into time_t and stored by field name (recur and duration UDAs).
static std::unordered_map<std::string, std::vector<time_t>> global_durations;

// Pre-computed dependency UUIDs to avoid repeated sort calls.
static std::vector<std::vector<std::string>> global_sorted_dep_uuids;

// UDA types to avoid repeated lookups.
static std::unordered_map<std::string, std::string> global_uda_types;
static std::vector<std::string> global_random_keys;

////////////////////////////////////////////////////////////////////////////////
void sort_tasks(const std::vector<Task>& data, std::vector<int>& order, const std::string& keys) {
  Timer timer;
  const auto load_before = Context::getContext().time_load_us;
  global_data = &data;

  // Split the key defs.
  auto key_defs = split(keys, ',');

  // Pre-computing of sorting values.
  global_durations.clear();
  global_uda_types.clear();
  global_sorted_dep_uuids.clear();
  global_random_keys.clear();
  global_sort_keys.clear();
  global_sort_keys.reserve(key_defs.size());
  for (auto& k : key_defs) {
    std::string field;
    bool ascending, breakIndicator;
    Context::getContext().decomposeSortField(k, field, ascending, breakIndicator);
    global_sort_keys.push_back({field, ascending});

    // Generate a random seend for sorting by "random".
    if (field == "random") {
      if (sort_random_seed == 0) {
        // For testing purposes, allow the seed to be specified in an undocumented configuration
        // setting.
        std::string seed_str = Context::getContext().config.get("debug.random.seed");
        if (seed_str.empty()) {
          std::random_device rd;
          sort_random_seed = rd();
        } else {
          sort_random_seed = std::stoul(seed_str);
        }
      }

      auto seed = std::to_string(sort_random_seed);
      global_random_keys.resize(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        global_random_keys[i] =
            std::to_string(std::hash<std::string>{}(data[i].get_ref("uuid") + seed));
      }
      continue;
    }

    if (field == "depends") {
      global_sorted_dep_uuids.resize(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        auto deps = data[i].getDependencyUUIDs();
        std::sort(deps.begin(), deps.end());
        global_sorted_dep_uuids[i] = std::move(deps);
      }
      continue;
    }

    if (field == "recur") {
      auto& cache = global_durations[field];
      cache.resize(data.size(), 0);
      for (size_t i = 0; i < data.size(); ++i) {
        auto s = data[i].get_ref("recur");
        if (!s.empty()) cache[i] = Duration(s).toTime_t();
      }
      continue;
    }

    auto col_it = Context::getContext().columns.find(field);
    if (col_it != Context::getContext().columns.end()) {
      auto type = col_it->second->type();
      global_uda_types[field] = type;
      if (type == "duration") {
        auto& cache = global_durations[field];
        cache.resize(data.size(), 0);
        for (size_t i = 0; i < data.size(); ++i) {
          auto s = data[i].get_ref(field);
          if (!s.empty()) cache[i] = Duration(s).toTime_t();
        }
      }
    }
  }

  // Only sort if necessary.
  if (order.size()) std::stable_sort(order.begin(), order.end(), sort_compare);

  Context::getContext().time_sort_us +=
      timer.total_us() - (Context::getContext().time_load_us - load_before);
}

void sort_projects(std::list<std::pair<std::string, int>>& sorted,
                   std::map<std::string, int>& allProjects) {
  for (auto& project : allProjects) {
    const std::vector<std::string> parents = extractParents(project.first);
    if (parents.size()) {
      // if parents exist: store iterator position of last parent
      std::list<std::pair<std::string, int>>::iterator parent_pos;
      for (auto& parent : parents) {
        parent_pos = std::find_if(
            sorted.begin(), sorted.end(),
            [&parent](const std::pair<std::string, int>& item) { return item.first == parent; });

        // if parent does not exist yet: insert into sorted view
        if (parent_pos == sorted.end()) sorted.emplace_back(parent, 1);
      }

      // insert new element below latest parent
      sorted.insert((parent_pos == sorted.end()) ? parent_pos : ++parent_pos, project);
    } else {
      // if has no parents: simply push to end of list
      sorted.push_back(project);
    }
  }
}

void sort_projects(std::list<std::pair<std::string, int>>& sorted,
                   std::map<std::string, bool>& allProjects) {
  std::map<std::string, int> allProjectsInt;
  for (auto& p : allProjects) allProjectsInt[p.first] = (int)p.second;

  sort_projects(sorted, allProjectsInt);
}

////////////////////////////////////////////////////////////////////////////////
// Re-implementation, using direct Task access instead of data copies that
// require re-parsing.
//
// Essentially a static implementation of a dynamic operator<.
static bool sort_compare(int left, int right) {
  for (const auto& key : global_sort_keys) {
    const auto& field = key.field;
    bool ascending = key.ascending;

    // Random.
    if (field == "random") {
      const auto& left_scrambled = global_random_keys[left];
      const auto& right_scrambled = global_random_keys[right];

      if (left_scrambled == right_scrambled) continue;

      return ascending ? (left_scrambled < right_scrambled) : (left_scrambled > right_scrambled);
    }

    // Urgency.
    else if (field == "urgency") {
      auto left_real = (*global_data)[left].urgency();
      auto right_real = (*global_data)[right].urgency();

      if (left_real == right_real) continue;

      return ascending ? (left_real < right_real) : (left_real > right_real);
    }

    // Number.
    else if (field == "id") {
      auto left_number = (*global_data)[left].id;
      auto right_number = (*global_data)[right].id;

      if (left_number == right_number) continue;

      return ascending ? (left_number < right_number) : (left_number > right_number);
    }

    // String.
    else if (field == "description" || field == "project" || field == "status" || field == "tags" ||
             field == "uuid" || field == "parent" || field == "imask" || field == "mask") {
      const auto& left_string = (*global_data)[left].get_ref(field);
      const auto& right_string = (*global_data)[right].get_ref(field);

      if (left_string == right_string) continue;

      return ascending ? (left_string < right_string) : (left_string > right_string);
    }

    // Due Date.
    else if (field == "due" || field == "end" || field == "entry" || field == "start" ||
             field == "until" || field == "wait" || field == "modified" || field == "scheduled") {
      const auto& left_string = (*global_data)[left].get_ref(field);
      const auto& right_string = (*global_data)[right].get_ref(field);

      if (left_string != "" && right_string == "") return true;

      if (left_string == "" && right_string != "") return false;

      if (left_string == right_string) continue;

      return ascending ? (left_string < right_string) : (left_string > right_string);
    }

    // Depends string.
    else if (field == "depends") {
      const auto& left_deps = global_sorted_dep_uuids[left];
      const auto& right_deps = global_sorted_dep_uuids[right];

      if (left_deps == right_deps) continue;

      if (left_deps.empty() && !right_deps.empty()) return ascending;

      if (!left_deps.empty() && right_deps.empty()) return !ascending;

      // Sort on the first dependency.
      auto left_number = Context::getContext().tdb2.id(left_deps[0]);
      auto right_number = Context::getContext().tdb2.id(right_deps[0]);

      if (left_number == right_number) continue;

      return ascending ? (left_number < right_number) : (left_number > right_number);
    }

    // Duration.
    else if (field == "recur") {
      auto it = global_durations.find(field);
      if (it != global_durations.end()) {
        auto left_dur = it->second[left];
        auto right_dur = it->second[right];
        if (left_dur == right_dur) continue;
        return ascending ? (left_dur < right_dur) : (left_dur > right_dur);
      }
    }

    // UDAs.
    else {
      auto type_it = global_uda_types.find(field);
      if (type_it == global_uda_types.end())
        throw format("The '{1}' column is not a valid sort field.", field);

      const auto& type = type_it->second;

      if (type == "numeric") {
        auto left_real = strtof(((*global_data)[left].get_ref(field)).c_str(), nullptr);
        auto right_real = strtof(((*global_data)[right].get_ref(field)).c_str(), nullptr);

        if (left_real == right_real) continue;

        return ascending ? (left_real < right_real) : (left_real > right_real);
      } else if (type == "string") {
        const auto& left_string = (*global_data)[left].get_ref(field);
        const auto& right_string = (*global_data)[right].get_ref(field);

        if (left_string == right_string) continue;

        // UDAs of the type string can have custom sort orders, which need to be considered.
        auto order = Task::customOrder.find(field);
        if (order != Task::customOrder.end()) {
          // Guaranteed to be found, because of ColUDA::validate ().
          auto posLeft = std::find(order->second.begin(), order->second.end(), left_string);
          auto posRight = std::find(order->second.begin(), order->second.end(), right_string);
          return ascending ? (posLeft < posRight) : (posLeft > posRight);
        } else {
          // Empty values are unconditionally last, if no custom order was specified.
          if (left_string == "")
            return false;
          else if (right_string == "")
            return true;

          return ascending ? (left_string < right_string) : (left_string > right_string);
        }
      }

      else if (type == "date") {
        const auto& left_string = (*global_data)[left].get_ref(field);
        const auto& right_string = (*global_data)[right].get_ref(field);

        if (left_string != "" && right_string == "") return true;

        if (left_string == "" && right_string != "") return false;

        if (left_string == right_string) continue;

        return ascending ? (left_string < right_string) : (left_string > right_string);
      } else if (type == "duration") {
        auto it = global_durations.find(field);
        if (it != global_durations.end()) {
          auto left_dur = it->second[left];
          auto right_dur = it->second[right];
          if (left_dur == right_dur) continue;
          return ascending ? (left_dur < right_dur) : (left_dur > right_dur);
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
