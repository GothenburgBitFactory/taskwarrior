#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestProjects(TestCase):
    def setUp(self):
        self.t = Task()

        self.STATUS = ("The project '{0}' has changed\.  "
                       "Project '{0}' is {1} complete \({2} remaining\)\.")

    def test_project_summary_count(self):
        """'task projects' shouldn't consider deleted tasks in summary.
        Reported in bug 1044
        """
        self.t("add project:A 1")
        self.t("add project:B 2")
        self.t("add project:B 3")
        self.t("3 delete", input="y\n")
        code, out, err = self.t("project:B projects")

        expected = "1 project \(1 task\)"
        self.assertRegexpMatches(out, expected)

    def test_project_progress(self):
        """project status/progress is shown and is up-to-date"""

        code, out, err = self.t("add one pro:foo")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "0%",
                                                         "1 task"))

        code, out, err = self.t("add two pro:foo")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "0%",
                                                         "2 of 2 tasks"))

        code, out, err = self.t("add three pro:foo")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "0%",
                                                         "3 of 3 tasks"))

        code, out, err = self.t("add four pro:foo")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "0%",
                                                         "4 of 4 tasks"))

        code, out, err = self.t("1 done")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "25%",
                                                         "3 of 4 tasks"))

        code, out, err = self.t("2 delete", input="y\n")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "33%",
                                                         "2 of 3 tasks"))

        code, out, err = self.t("3 modify pro:bar")
        self.assertRegexpMatches(err, self.STATUS.format("foo", "50%",
                                                         "1 of 2 tasks"))
        self.assertRegexpMatches(err, self.STATUS.format("bar", "0%",
                                                         "1 task"))

    def test_project_spaces(self):
        """projects with spaces are handled correctly"""

        self.t("add hello pro:bob")
        code, out, err = self.t('1 mod pro:"foo bar"')
        self.assertRegexpMatches(err, self.STATUS.format("foo bar", "0%",
                                                         "1 task"))

    def add_tasks(self):
        self.t("add testing project:existingParent")
        self.t("add testing project:existingParent.child")
        self.t("add testing project:abstractParent.kid")
        self.t("add testing project:.myProject")
        self.t("add testing project:myProject")
        self.t("add testing project:.myProject.")

    def validate_indentation(self, out):
        order = (
            ".myProject ",
            ".myProject. ",
            "abstractParent",   # No space at EOL because this line in the summary ends here.
            "  kid ",
            "existingParent ",
            "  child ",
            "myProject ",
        )

        lines = out.splitlines(True)  # True = keep newlines
        # position where project names start on the lines list
        position = 3

        for i, proj in enumerate(order):
            pos = position + i

            self.assertTrue(
                lines[pos].startswith(proj),
                msg=("Project '{0}' is not in line #{1} or has an unexpected "
                     "indentation.{2}".format(proj, pos, out))
            )

    def test_project_indentation(self):
        """check project/subproject indentation in 'task projects'

        Reported in bug 1056

        See also the tests of helper functions for CmdProjects in util.t.cpp
        """
        self.add_tasks()

        code, out, err = self.t("projects")

        self.validate_indentation(out)

    def test_project_indentation_in_summary(self):
        """check project/subproject indentation in 'task summary'

        Reported in bug 1056
        """
        self.add_tasks()

        code, out, err = self.t("summary")

        self.validate_indentation(out)

    def test_project_helper(self):
        """Verify _projects helper list projects"""
        self.t("add project:A one")
        self.t("add project:B two")
        self.t("2 delete", input="y\n")
        self.t("log project:C three")
        self.t("list")

        code, out, err = self.t("_projects")
        self.assertIn("A", out)
        self.assertNotIn("B", out)
        self.assertNotIn("C", out)

        code, out, err = self.t("_projects rc.list.all.projects:1")
        self.assertIn("A", out)
        self.assertIn("B", out)
        self.assertIn("C", out)


class TestSubprojects(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t("add project:abc abc")
        cls.t("add project:ab ab")
        cls.t("add project:a a")
        cls.t("add project:b b")

    def test_project_exact1(self):
        """Verify single character exact"""
        code, out, err = self.t("list project:b")
        self.assertRegexpMatches(out, r"\bb\s")

    def test_project_top1(self):
        """Verify single character parent"""
        code, out, err = self.t("list project:a")
        self.assertRegexpMatches(out, r"\babc\s")
        self.assertRegexpMatches(out, r"\bab\s")
        self.assertRegexpMatches(out, r"\ba\s")

    def test_project_top2(self):
        """Verify double character parent"""
        code, out, err = self.t("list project:ab")
        self.assertRegexpMatches(out, r"\babc\s")
        self.assertRegexpMatches(out, r"\bab\s")

    def test_project_exact3(self):
        """Verify triple character exact"""
        code, out, err = self.t("list project:abc")
        self.assertRegexpMatches(out, r"\babc\s")

    def test_project_mismatch4(self):
        """Verify quad character mismatch"""
        code, out, err = self.t.runError("list project:abcd")
        self.assertIn("No matches", err)


class TestBug299(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add project:one foo")
        self.t("add project:ones faz")
        self.t("add project:phone boo")
        self.t("add project:bones too")
        self.t("add project:two bar")
        self.t("add project:three baz")

    def test_project_exclusion_isnt(self):
        """299: check project exclusion using project.isnt:<name>

        Reported in bug 299
        """
        code, out, err = self.t("list project.isnt:one pro.isnt:two")

        self.assertNotRegexpMatches(out, "one.*foo")
        self.assertRegexpMatches(out, "ones.*faz")
        self.assertRegexpMatches(out, "phone.*boo")
        self.assertRegexpMatches(out, "bones.*too")

        self.assertNotRegexpMatches(out, "two.*bar")
        self.assertRegexpMatches(out, "three.*baz")

    def test_project_exclusion_hasnt(self):
        """299: check project exclusion using project.hasnt:<name>

        Reported in bug 299
        """
        code, out, err = self.t("list project.hasnt:one pro.hasnt:two")

        self.assertNotRegexpMatches(out, "one.*foo")
        self.assertNotRegexpMatches(out, "ones.*faz")
        self.assertNotRegexpMatches(out, "phone.*boo")
        self.assertNotRegexpMatches(out, "bones.*too")

        self.assertNotRegexpMatches(out, "two.*bar")
        self.assertRegexpMatches(out, "three.*baz")


class TestBug555(TestCase):
    def setUp(self):
        self.t = Task()

    def test_log_with_project_segfault(self):
        """555: log with a project causes a segfault

        Reported in bug 555
        """
        code, out, err = self.t("log description project:p")

        self.assertNotIn("Segmentation fault", out)
        self.assertNotIn("Segmentation fault", err)
        self.assertIn("Logged task", out)


class TestBug605(TestCase):
    def setUp(self):
        self.t = Task()

    def test_delete_task_for_empty_project(self):
        """605: Project correctly reports % completion when empty or all tasks completed

        Reported in bug 605
        """
        self.t("add One project:p1")

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("is 0% complete", err)

        self.t("add Two project:p1")
        code, out, err = self.t("2 done")
        self.assertIn("is 100% complete", err)


class TestBug835(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_project_hierarchy_filter(self):
        """835: Verify filter on project hierarchy, plus parentheses"""
        self.t("add pro:main.subproject one")
        code, out, err = self.t("ls")
        self.assertIn("main.subproject", out)

        code, out, err = self.t("(pro:main.subproject) ls")
        self.assertIn("main.subproject", out)
        self.assertNotIn("Mismatched parentheses in expression", out)


class TestBug906(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_hierarchy_filter(self):
        """906: Test project hierarchy filters

           Bug 906
        """
        self.t("add zero")
        self.t("add one pro:a.b")
        self.t("add two pro:a")

        code, out, err = self.t("pro:a list")
        self.assertNotIn("zero", out)
        self.assertIn("one", out)
        self.assertIn("two", out)

        code, out, err = self.t("pro:a.b list")
        self.assertNotIn("zero", out)
        self.assertIn("one", out)
        self.assertNotIn("two", out)

        code, out, err = self.t("pro.not:a list")
        self.assertIn("zero", out)
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)

        code, out, err = self.t("pro.not:a.b list")
        self.assertIn("zero", out)
        self.assertNotIn("one", out)
        self.assertIn("two", out)


class TestBug856(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_hierarchy_filter(self):
        """856: Test project.none: works

           Bug 856: "task list project.none:" does not work.
        """
        self.t("add assigned project:X")
        self.t("add floating")

        code, out, err = self.t("project: ls")
        self.assertIn("floating", out)
        self.assertNotIn("assigned", out)

        code, out, err = self.t("project:\'\' ls")
        self.assertIn("floating", out)
        self.assertNotIn("assigned", out)

        code, out, err = self.t("project.none: ls")
        self.assertIn("floating", out)
        self.assertNotIn("assigned", out)


class TestBug1511(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_hierarchy_filter(self):
        """1511: Test project:one-two can be added and queried

           Bug 1511: Project titles not properly parsed if they contain hyphens
        """
        self.t("add zero")
        self.t("add one project:two-three")
        code, out, err = self.t("project:two-three list")
        self.assertIn("one", out)
        self.assertNotIn("zero", out)


class TestBug1455(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_hierarchy_filter(self):
        """1455: Test project:school)

           Bug 1455: Filter parser does not properly handle parentheses in attributes
        """
        self.t("add zero")
        self.t("add one project:two)")
        code, out, err = self.t("project:two) list")
        self.assertIn("one", out)
        self.assertNotIn("zero", out)


class TestBug899(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.config("verbose", "on")

    def test_log_project(self):
        """899: Verify task log behaves correctly when logging into a project"""
        code, out, err = self.t("add one pro:A")
        self.assertRegexpMatches(err, " 0% complete \(1 task ")

        code, out, err = self.t("add two pro:A")
        self.assertRegexpMatches(err, " 0% complete \(2 of 2 ")

        code, out, err = self.t("1 done")
        self.assertRegexpMatches(err, " 50% complete \(1 of 2 ")

        code, out, err = self.t("log three pro:A")
        self.assertRegexpMatches(err, " 66% complete \(1 of 3 ")


class TestBug1267(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add_task_no_project_with_default(self):
        """1267: Add a task without a project using direct rc change
        """
        project = "MakePudding"
        self.t("rc.default.project={0} add proj: 'Add cream'".format(project))
        code, out, err = self.t("ls")
        self.assertNotIn(project, out)

    def test_add_task_no_project_with_default_rcfile(self):
        """1267: Add a task without a project writing to rc file
        """
        project = "MakePudding"
        self.t.config("default.project", project)
        self.t("add proj: 'Add cream'")
        code, out, err = self.t("ls")
        self.assertNotIn(project, out)


class TestBug1430(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_names_with_dots(self):
        """1430: Check that filtering works for project names with dots"""
        self.t("add foo project:home.garden")
        code, out, err = self.t("_get 1.project")
        self.assertEqual("home.garden\n", out)

    def test_project_names_with_slashes(self):
        """1430: Check that filtering works for project names with slashes"""
        self.t("add foo project:home/garden")
        code, out, err = self.t("_get 1.project")
        self.assertEqual("home/garden\n", out)


class TestBug1617(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_multi_word_project(self):
        """1617: Verify search for multi-word project"""
        self.t("add one")
        self.t("add two project:'three four'")

        code, out, err = self.t("project:'three four' list")
        self.assertIn("two", out)
        self.assertNotIn("one", out)


class TestBug1627(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_project_eval(self):
        """1627: Verify that a value of 'mon' is not eval'd to 'monday' for a project"""
        self.t("add foo project:mon")
        code, out, err = self.t("_get 1.project")
        self.assertEqual("mon\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
