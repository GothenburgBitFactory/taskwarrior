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


class TestSugar(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t('add one')
        self.t('add two')
        self.t('add three')

    def test_empty_conjunction(self):
        """Test syntax that mathematicians find sane and expected"""
        code, out, err = self.t("1 2 ls")
        self.assertIn("one", out)
        self.assertIn("two", out)
        self.assertNotIn("three", out)

        self.t.config("sugar", "off")
        code, out, err = self.t("1 2 count")
        self.assertEqual(0, int(out))

    def test_ids_not_lifted_to_global_disjunction(self):
        """If I asked the other test whether it passes, what would it answer?"""
        # Default mode: ids are lifted to global disjunction:
        #    ( id==3 or id==2 ) and ( three ).
        code, out, err = self.t("3 and '( 2 three )' ls")
        self.assertNotIn("one", out)
        self.assertNotIn("two", out)
        self.assertIn("three", out)

        # sugar off: WYSIWYG parsing.
        self.t.config("sugar", "off")
        code, out, err = self.t("3 and '( 2 three )' count")
        self.assertEqual(0, int(out))


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
