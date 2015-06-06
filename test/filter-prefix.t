#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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


class TestFilterPrefix(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("verbose", "nothing")

        cls.t(('add', 'project:foo.uno',  'priority:H', '+tag', 'one foo'      ))
        cls.t(('add', 'project:foo.dos',  'priority:H',         'two'          ))
        cls.t(('add', 'project:foo.tres',                       'three'        ))
        cls.t(('add', 'project:bar.uno',  'priority:H',         'four'         ))
        cls.t(('add', 'project:bar.dos',                '+tag', 'five'         ))
        cls.t(('add', 'project:bar.tres',                       'six foo'      ))
        cls.t(('add', 'project:bazuno',                         'seven bar foo'))
        cls.t(('add', 'project:bazdos',                         'eight bar foo'))

    def test_list_all(self):
        """No filter shows all tasks."""
        code, out, err = self.t(('list',))
        self.assertIn('one', out)
        self.assertIn('two', out)
        self.assertIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_project_foo(self):
        """Filter on project name."""
        code, out, err = self.t(('list', 'project:foo'))
        self.assertIn('one', out)
        self.assertIn('two', out)
        self.assertIn('three', out)
        self.assertNotIn('four', out)
        self.assertNotIn('five', out)
        self.assertNotIn('six', out)
        self.assertNotIn('seven', out)
        self.assertNotIn('eight', out)

    def test_list_project_not_foo(self):
        """Filter on not project name."""
        code, out, err = self.t(('list', 'project.not:foo'))
        self.assertIn('one', out)
        self.assertIn('two', out)
        self.assertIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_project_startswith_bar(self):
        """Filter on project name start."""
        code, out, err = self.t(('list', 'project.startswith:bar'))
        self.assertNotIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertNotIn('seven', out)
        self.assertNotIn('eight', out)

    def test_list_project_ba(self):
        """Filter on project partial match."""
        code, out, err = self.t(('list', 'project:ba'))
        self.assertNotIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertIn('four', out)
        self.assertIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)

    def test_list_description_has_foo(self):
        """Filter on description pattern."""
        code, out, err = self.t(('list', 'description.has:foo'))
        self.assertIn('one', out)
        self.assertNotIn('two', out)
        self.assertNotIn('three', out)
        self.assertNotIn('four', out)
        self.assertNotIn('five', out)
        self.assertIn('six', out)
        self.assertIn('seven', out)
        self.assertIn('eight', out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
