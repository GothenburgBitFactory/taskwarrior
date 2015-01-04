#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class Test1430(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_names_with_dots(self):
        """Check that filtering works for project names with dots"""
        pro = "home.garden"
        self.t(('add', 'foo', 'project:%s' % pro))
        code, out, err = self.t(('list', 'project:%s' % pro))
        # We expect a clean exit
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))

    def test_project_names_with_slashes(self):
        """Check that filtering works for project names with slashes"""
        pro = "home/garden"
        self.t(('add', 'foo', 'project:%s' % pro))

        # TODO Restore this test and fix it.
        # The form 'name:a/b' does not work, while 'name.is:a/b' does.
        #code, out, err = self.t(('list', 'project:%s' % pro))
        code, out, err = self.t(('list', 'project.is:%s' % pro))
        # We expect a clean exit
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
