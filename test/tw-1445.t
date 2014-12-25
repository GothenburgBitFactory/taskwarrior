#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
from datetime import datetime
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1445(TestCase):
    def setUp(self):
        self.t = Task()

    def test_alias_single_word(self):
        """Verify single-word aliases"""
        self.t.config('alias.when', 'execute date')
        code, out, err = self.t(('when',))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn(str(datetime.now().year), out)

    def test_alias_multi_word(self):
        """Verify multi-word aliases"""
        self.t.config('alias.worktasks', 'list +work')
        self.t(('add', 'one', '+work'))
        self.t(('add', 'two'))
        code, out, err = self.t(('worktasks',))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('one', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
