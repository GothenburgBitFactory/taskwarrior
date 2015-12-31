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


class TestUrgency(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("urgency.uda.priority.H.coefficient",       "10")
        cls.t.config("urgency.uda.priority.M.coefficient",       "6.5")
        cls.t.config("urgency.uda.priority.L.coefficient",       "3")
        cls.t.config("urgency.active.coefficient",               "10")
        cls.t.config("urgency.project.coefficient",              "10")
        cls.t.config("urgency.due.coefficient",                  "10")
        cls.t.config("urgency.blocking.coefficient",             "10")
        cls.t.config("urgency.blocked.coefficient",              "10")
        cls.t.config("urgency.annotations.coefficient",          "10")
        cls.t.config("urgency.tags.coefficient",                 "10")
        cls.t.config("urgency.waiting.coefficient",              "-10")
        cls.t.config("urgency.user.tag.next.coefficient",        "10")
        cls.t.config("urgency.user.project.PROJECT.coefficient", "10")
        cls.t.config("urgency.user.tag.TAG.coefficient",         "10")
        cls.t.config("confirmation",                             "off")

        cls.t("add control")                     # 1

        cls.t("add 1a pri:H")                    # 2
        cls.t("add 1b pri:M")                    # 3
        cls.t("add 1c pri:L")                    # 4

        cls.t("add 2a project:P")                # 5

        cls.t("add 3a")                          # 6
        cls.t("6 start")

        cls.t("add 4a +next")                    # 7

        cls.t("add 5a +one")                     # 8
        cls.t("add 5b +one +two")                # 9
        cls.t("add 5c +one +two +three")         # 10
        cls.t("add 5d +one +two +three +four")   # 11

        cls.t("add 6a")                          # 12
        cls.t("12 annotate A")
        cls.t("add 6b")                          # 13
        cls.t("13 annotate A")
        cls.t("13 annotate B")
        cls.t("add 6c")                          # 14
        cls.t("14 annotate A")
        cls.t("14 annotate B")
        cls.t("14 annotate C")
        cls.t("add 6d")                          # 15

        cls.t("15 annotate A")
        cls.t("15 annotate B")
        cls.t("15 annotate C")
        cls.t("15 annotate D")

        cls.t("add 7a wait:10s")                 # 16

        cls.t("add 8a")                          # 17
        cls.t("add 8b depends:17")               # 18

        cls.t("add 9a due:-10d")                 # 19
        cls.t("add 9b due:-7d")                  # 20
        cls.t("add 9c due:-6d")                  # 21
        cls.t("add 9d due:-5d")                  # 22
        cls.t("add 9e due:-4d")                  # 23
        cls.t("add 9f due:-3d")                  # 24
        cls.t("add 9g due:-2d")                  # 25
        cls.t("add 9h due:-1d")                  # 26
        cls.t("add 9i due:now")                  # 27
        cls.t("add 9j due:25h")                  # 28
        cls.t("add 9k due:49h")                  # 29
        cls.t("add 9l due:73h")                  # 30
        cls.t("add 9m due:97h")                  # 31
        cls.t("add 9n due:121h")                 # 32
        cls.t("add 9o due:145h")                 # 33
        cls.t("add 9p due:169h")                 # 34
        cls.t("add 9q due:193h")                 # 35
        cls.t("add 9r due:217h")                 # 36
        cls.t("add 9s due:241h")                 # 37
        cls.t("add 9t due:265h")                 # 38
        cls.t("add 9u due:289h")                 # 39
        cls.t("add 9v due:313h")                 # 40
        cls.t("add 9w due:337h")                 # 41
        cls.t("add 9x due:361h")                 # 42

        cls.t("add 10a project:PROJECT")         # 43

        cls.t("add 11a +TAG")                    # 44

        cls.t("add 12a scheduled:30d")           # 45
        cls.t("add 12b scheduled:yesterday")     # 46

        cls.t("add 13 pri:H")                    # 47

    def assertApproximately(self, target, value):
        """Verify that the number in 'value' is within the range"""
        num = float(value.strip())
        self.assertEqual(target - 0.1 < num and num < target + 0.1, True)

    def test_urgency_priority(self):
        """Verify urgency calculations involving priority"""
        code, out, err = self.t("_get 1.urgency")
        self.assertIn("0\n", out)
        code, out, err = self.t("_get 2.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 3.urgency")
        self.assertIn("6.5\n", out)
        code, out, err = self.t("_get 4.urgency")
        self.assertIn("3\n", out)

    def test_urgency_project(self):
        """Verify urgency calculations involving project"""
        code, out, err = self.t("_get 5.urgency")
        self.assertIn("10\n", out)

    def test_urgency_active(self):
        """Verify urgency calculations involving active tasks"""
        code, out, err = self.t("_get 6.urgency")
        self.assertIn("10\n", out)

    def test_urgency_next(self):
        """Verify urgency calculations involving +next"""
        code, out, err = self.t("_get 7.urgency")
        self.assertIn("18\n", out)

    def test_urgency_tags(self):
        """Verify urgency calculations involving tags"""
        code, out, err = self.t("_get 8.urgency")
        self.assertIn("8\n", out)
        code, out, err = self.t("_get 9.urgency")
        self.assertIn("9\n", out)
        code, out, err = self.t("_get 10.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 11.urgency")
        self.assertIn("10\n", out)

    def test_urgency_annotations(self):
        """Verify urgency calculations involving annotations"""
        code, out, err = self.t("_get 12.urgency")
        self.assertIn("8\n", out)
        code, out, err = self.t("_get 13.urgency")
        self.assertIn("9\n", out)
        code, out, err = self.t("_get 14.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 15.urgency")
        self.assertIn("10\n", out)

    def test_urgency_waiting(self):
        """Verify urgency calculations involving waiting tasks"""
        code, out, err = self.t("_get 16.urgency")
        self.assertIn("-10\n", out)

    def test_urgency_dependencies(self):
        """Verify urgency calculations involving dependencies"""
        code, out, err = self.t("_get 17.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 18.urgency")
        self.assertIn("10\n", out)

    def test_urgency_due(self):
        """Verify urgency calculations involving due dates"""
        code, out, err = self.t("_get 19.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 20.urgency")
        self.assertIn("10\n", out)
        code, out, err = self.t("_get 21.urgency")
        self.assertApproximately(9.62, out)
        code, out, err = self.t("_get 22.urgency")
        self.assertApproximately(9.24, out)
        code, out, err = self.t("_get 23.urgency")
        self.assertApproximately(8.86, out)
        code, out, err = self.t("_get 24.urgency")
        self.assertApproximately(8.48, out)
        code, out, err = self.t("_get 25.urgency")
        self.assertApproximately(8.1, out)
        code, out, err = self.t("_get 26.urgency")
        self.assertApproximately(7.71, out)
        code, out, err = self.t("_get 27.urgency")
        self.assertApproximately(7.33, out)
        code, out, err = self.t("_get 28.urgency")
        self.assertApproximately(6.94, out)
        code, out, err = self.t("_get 29.urgency")
        self.assertApproximately(6.56, out)
        code, out, err = self.t("_get 30.urgency")
        self.assertApproximately(6.17, out)
        code, out, err = self.t("_get 31.urgency")
        self.assertApproximately(5.79, out)
        code, out, err = self.t("_get 32.urgency")
        self.assertApproximately(5.41, out)
        code, out, err = self.t("_get 33.urgency")
        self.assertApproximately(5.03, out)
        code, out, err = self.t("_get 34.urgency")
        self.assertApproximately(4.65, out)
        code, out, err = self.t("_get 35.urgency")
        self.assertApproximately(4.27, out)
        code, out, err = self.t("_get 36.urgency")
        self.assertApproximately(3.89, out)
        code, out, err = self.t("_get 37.urgency")
        self.assertApproximately(3.51, out)
        code, out, err = self.t("_get 38.urgency")
        self.assertApproximately(3.13, out)
        code, out, err = self.t("_get 39.urgency")
        self.assertApproximately(2.75, out)
        code, out, err = self.t("_get 40.urgency")
        self.assertApproximately(2.37, out)
        code, out, err = self.t("_get 41.urgency")
        self.assertApproximately(2, out)
        code, out, err = self.t("_get 42.urgency")
        self.assertApproximately(2, out)

    def test_urgency_user_project(self):
        """Verify urgency calculations involving user project"""
        code, out, err = self.t("_get 43.urgency")
        self.assertIn("20\n", out)

    def test_urgency_user_tag(self):
        """Verify urgency calculations involving user tag"""
        code, out, err = self.t("_get 44.urgency")
        self.assertIn("18\n", out)

    def test_urgency_scheduled(self):
        """Verify urgency calculations involving a scheduled task"""
        code, out, err = self.t("_get 45.urgency")
        self.assertIn("0\n", out)
        code, out, err = self.t("_get 46.urgency")
        self.assertIn("5\n", out)

    def test_urgency_coefficient_override(self):
        """Verify urgency coefficient override"""
        code, out, err = self.t("rc.urgency.uda.priority.H.coefficient:0.01234 _get 47.urgency")
        self.assertApproximately(0.01234, out)

    def test_urgency_no_task(self):
        """Verify no error when no tasks match"""
        code, out, err = self.t.runError("999 _urgency")
        self.assertEqual("", out)
        self.assertEqual("", err)

    def test_urgency_all_tasks(self):
        """Verify all tasks when no filter is specified"""
        code, out, err = self.t("_urgency")
        self.assertIn("task 10 ", out)
        self.assertIn("task 20 ", out)
        self.assertIn("task 30 ", out)
        self.assertIn("task 40 ", out)

    def test_urgency_uuid(self):
        """Verify _urgency using UUID lookup"""
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()

        code, out, err = self.t(uuid + " _urgency")
        self.assertEqual("task 1 urgency 0\n", out)


class TestBug837(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_unblocked_urgency(self):
        """837: Verify urgency goes to zero after unblocking

           Bug 837: When a task is completed, tasks that depended upon it do not
                    have the correct urgency and depend on 0 when edited
        """

        self.t("add one")
        self.t("add two dep:1")
        self.t("list") # GC/handleRecurrence

        code, out, err = self.t("_get 1.urgency")
        self.assertEqual("8\n", out)

        code, out, err = self.t("_get 2.urgency")
        self.assertEqual("-5\n", out)

        self.t("1 done")
        self.t("list") # GC/handleRecurrence

        code, out, err = self.t("_get 1.urgency")
        self.assertEqual("0\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
