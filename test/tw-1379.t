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
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class TestBug1379(TestCase):
    def setUp(self):
        self.t = Task()
        # Themes are a special case that cannot be set via "task config"
        with open(self.t.taskrc, 'a') as fh:
            fh.write("include " + REPO_DIR + "/doc/rc/no-color.theme\n")

        self.t.config("color.alternate", "")
        self.t.config("_forcecolor", "1")
        self.t.config("color.label", "")
        self.t.config("color.label.sort", "")

        # For use with regex
        self.RED = "\033\[31m"
        self.CLEAR = "\033\[0m"

    def test_color_BLOCKED(self):
        """color.tag.BLOCKED changes color of BLOCKED tasks"""
        self.t.config("color.tag.BLOCKED", "red")

        self.t("add Blocks")
        self.t("add dep:1 Blocked")

        code, out, err = self.t("all +BLOCKED")
        self.assertRegexpMatches(out, self.RED + r".*Blocked.*" + self.CLEAR)

    def test_color_UNBLOCKED(self):
        """color.tag.UNBLOCKED changes color of UNBLOCKED tasks"""
        self.t.config("color.tag.UNBLOCKED", "red")

        self.t("add Blocks")
        self.t("add dep:1 Blocked")

        code, out, err = self.t("all +UNBLOCKED")
        self.assertRegexpMatches(out, self.RED + r".*Blocks.*" + self.CLEAR)

    def test_color_BLOCKING(self):
        """color.tag.BLOCKING changes color of BLOCKING tasks"""
        self.t.config("color.tag.BLOCKING", "red")

        self.t("add Blocks")
        self.t("add dep:1 Blocked")

        code, out, err = self.t("all +BLOCKING")
        self.assertRegexpMatches(out, self.RED + r".*Blocks.*" + self.CLEAR)

    def test_color_SCHEDULED(self):
        """color.tag.SCHEDULED changes color of SCHEDULED tasks"""
        self.t.config("color.tag.SCHEDULED", "red")

        self.t("add scheduled:tomorrow Have fun")

        code, out, err = self.t("all +SCHEDULED")
        self.assertRegexpMatches(out, self.RED + r".*Have fun.*" + self.CLEAR)

    def test_color_UNTIL(self):
        """color.tag.UNTIL changes color of UNTIL tasks"""
        self.t.config("color.tag.UNTIL", "red")

        self.t("add until:tomorrow Urgent")

        code, out, err = self.t("all +UNTIL")
        self.assertRegexpMatches(out, self.RED + r".*Urgent.*" + self.CLEAR)

    def test_color_WAITING(self):
        """color.tag.WAITING changes color of WAITING tasks"""
        self.t.config("color.tag.WAITING", "red")

        self.t("add wait:tomorrow Tomorrow")

        code, out, err = self.t("all +WAITING")
        self.assertRegexpMatches(out, self.RED + r".*Tomorrow.*" + self.CLEAR)

    def test_color_PARENT(self):
        """color.tag.PARENT changes color of PARENT tasks"""
        self.t.config("color.tag.PARENT", "red")

        self.t("add recur:daily due:tomorrow Email")

        code, out, err = self.t("all +PARENT")
        self.assertRegexpMatches(out, self.RED + r".*Email.*" + self.CLEAR)

    def test_color_CHILD(self):
        """color.tag.CHILD changes color of CHILD tasks"""
        self.t.config("color.tag.CHILD", "red")

        self.t("add recur:daily due:tomorrow Email")

        code, out, err = self.t("all +CHILD")
        self.assertRegexpMatches(out, self.RED + r".*Email.*" + self.CLEAR)

    def test_color_PENDING(self):
        """color.tag.PENDING changes color of PENDING tasks"""
        self.t.config("color.tag.PENDING", "red")

        self.t("add Pending")

        code, out, err = self.t("all +PENDING")
        self.assertRegexpMatches(out, self.RED + r".*Pending.*" + self.CLEAR)

    def test_color_COMPLETED(self):
        """color.tag.COMPLETED changes color of COMPLETED tasks"""
        self.t.config("color.tag.COMPLETED", "red")
        self.t.config("color.completed", "")

        self.t("add Complete")
        self.t("1 done")

        code, out, err = self.t("all +COMPLETED")
        self.assertRegexpMatches(out, self.RED + r".*Complete.*" + self.CLEAR)

    def test_color_DELETED(self):
        """color.tag.DELETED changes color of DELETED tasks"""
        self.t.config("color.tag.DELETED", "red")
        self.t.config("color.deleted", "")

        self.t("add Delete")
        self.t("1 delete", input="y\n")

        code, out, err = self.t("all +DELETED")
        self.assertRegexpMatches(out, self.RED + r".*Delete.*" + self.CLEAR)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
