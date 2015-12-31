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


class BaseTestEnpassant(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        # No journal log which may contain the words we are looking for
        self.t.config("journal.info", "off")


class TestEnpassantMultiple(BaseTestEnpassant):
    def setUp(self):
        super(TestEnpassantMultiple, self).setUp()

        self.t("add foo")
        self.t("add foo bar")
        self.t("add baz foo baz")

    def validate_info(self, id, desc):
        code, out, err = self.t((id, "info"))

        self.assertRegexpMatches(
            out, "Status +Completed",
            msg="enpassant {0} status change".format(id),
        )
        self.assertRegexpMatches(
            out, "Priority +H",
            msg="enpassant {0} priority change".format(id),
        )
        self.assertRegexpMatches(
            out, "Tags +tag",
            msg="enpassant {0} tag change".format(id),
        )
        self.assertRegexpMatches(
            out, "Description +{0}".format(desc),
            msg="enpassant {0} description change".format(id),
        )

    def test_multiple(self):
        "Test enpassant in multiple tasks and with multiple changes at once"
        self.t("1,2,3 done /foo/FOO/ pri:H +tag", input="all\n")

        self.validate_info("1", desc="FOO")
        self.validate_info("2", desc="FOO bar")
        self.validate_info("3", desc="baz FOO baz")


class TestEnpassant(BaseTestEnpassant):
    def setUp(self):
        super(TestEnpassant, self).setUp()

        self.t.config("confirmation", "off")

        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("add five")

    def perform_action(self, action):
        self.t(("1", action, "oneanno"))
        code, out, err = self.t("1 info")
        self.assertRegexpMatches(out, "Description +one\n[0-9: -]+ oneanno",
                                 msg="{0} enpassant annotation".format(action))

        self.t(("2", action, "/two/TWO/"))
        code, out, err = self.t("2 info")
        self.assertRegexpMatches(out, "Description +TWO",
                                 msg="{0} enpassant modify".format(action))

        self.t(("3", action, "+threetag"))
        code, out, err = self.t("3 info")
        self.assertRegexpMatches(out, "Tags +threetag",
                                 msg="{0} enpassant tag".format(action))

        self.t(("4", action, "pri:H"))
        code, out, err = self.t("4 info")
        self.assertRegexpMatches(out, "Priority +H",
                                 msg="{0} enpassant priority".format(action))

        self.t(("5", action, "pro:PROJ"))
        code, out, err = self.t("5 info")
        self.assertRegexpMatches(out, "Project +PROJ",
                                 msg="{0} enpassant project".format(action))

    def test_done(self):
        """Test 'done' with en-passant changes"""
        self.perform_action("done")

    def test_delete(self):
        """Test 'delete' with en-passant changes"""
        self.perform_action("delete")

    def test_start(self):
        """Test 'start' with en-passant changes"""
        self.perform_action("start")

    def test_stop(self):
        """Test 'stop' with en-passant changes"""
        self.t("1-5 start", input="all\n")

        self.perform_action("stop")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
