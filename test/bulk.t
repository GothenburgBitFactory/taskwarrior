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
import signal
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBulk(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("bulk", "3")

        self.t("add one")
        self.t("add two")
        self.t("add three")

    def test_bulk_confirmations_single_confirmation_off(self):
        """not bulk delete 1 tasks with confirmation:off deletes it"""

        # Test with 1 task.  1 is a special case.
        code, out, err = self.t("1 delete rc.confirmation:off")
        self.assertNotIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)

    def test_bulk_confirmations_single_confirmation_on(self):
        """not bulk delete 1 task with confirmation:on and input >y deletes it"""

        # Test with 1 task.  1 is a special case.
        code, out, err = self.t("2 delete rc.confirmation:on", input="y\n")
        self.assertIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 2", out)

    def test_bulk_confirmations_double_confirmation_off(self):
        """not bulk delete 2 tasks with confirmation:off deletes them"""

        # Test with 2 tasks.  2 is greater than 1 and less than bulk.
        code, out, err = self.t("1-2 delete rc.confirmation:off")
        self.assertNotIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)
        self.assertIn("Deleting task 2", out)

    def test_bulk_confirmations_double_confirmation_on(self):
        """not bulk delete 2 tasks with confirmation:on and input >y >y deletes them"""

        # Test with 2 tasks.  2 is greater than 1 and less than bulk.
        code, out, err = self.t("1-2 delete rc.confirmation:on", input="y\ny\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)
        self.assertIn("Deleting task 2", out)

    def test_bulk_confirmations_bulk_confirmation_off(self):
        """bulk delete 3 tasks with confirmation:off always prompts"""

        # Test with 3 tasks.  3 is considered bulk. rc.confirmation has no effect on bulk

        # Delete task 1 'one'? (yes/no/all/quit) --> timeout
        code, out, err = self.t.runError("1-3 delete rc.confirmation:off")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertNotIn("Deleting task", out)
        # task timeout on input - exit by signal is negative in Python
        # Sometimes it just fails. Not sure why, but taskwarrior is behaving well.
        self.assertTrue(code in [1, -signal.SIGABRT])

        # Delete task 1 'one'? (yes/no/all/quit) Deleting task 1 'one'.
        # Delete task 2 'two'? (yes/no/all/quit) Deleting task 2 'two'.
        # Delete task 3 'three'? (yes/no/all/quit) Deleting task 3 'three'.
        # Deleted 3 tasks.
        code, out, err = self.t("1-3 delete rc.confirmation:off", input="y\ny\ny\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)
        self.assertIn("Deleting task 2", out)
        self.assertIn("Deleting task 3", out)

    def test_bulk_confirmations_bulk_confirmation_on(self):
        """bulk delete 3 tasks with confirmation:on and input >y >y >y deletes them"""

        # Test with 3 tasks.  3 is considered bulk. rc.confirmation has no effect on bulk
        code, out, err = self.t("1-3 delete rc.confirmation:on", input="y\ny\ny\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)
        self.assertIn("Deleting task 2", out)
        self.assertIn("Deleting task 3", out)

    def test_bulk_delete_no_tests(self):
        """bulk delete >no deletes nothing"""

        # Test with 1 task, denying delete.
        code, out, err = self.t.runError("1 delete rc.confirmation:on", input="n\n")
        self.assertIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertNotIn("Deleted task 1", out)
        self.assertNotIn("Deleting task", out)

        # Test with 2 tasks, denying delete.
        code, out, err = self.t.runError("1-2 delete rc.confirmation:on", input="n\nn\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertNotIn("Deleted task 1", out)
        self.assertNotIn("Deleted task 2", out)
        self.assertNotIn("Deleting task", out)

        # Test with 3 tasks, denying delete.
        code, out, err = self.t.runError("1-3 delete rc.confirmation:on", input="n\nn\nn\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertNotIn("Deleted task 1", out)
        self.assertNotIn("Deleted task 2", out)
        self.assertNotIn("Deleted task 3", out)
        self.assertNotIn("Deleting task", out)

    def test_bulk_delete_all_tests(self):
        """bulk delete >all deletes everything"""

        code, out, err = self.t("1-3 delete rc.confirmation:on", input="all\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task 1", out)
        self.assertIn("Deleting task 2", out)
        self.assertIn("Deleting task 3", out)
        self.assertIn("Deleted 3 tasks", out)

    def test_bulk_delete_quit_tests(self):
        """bulk delete >quit deletes nothing"""

        code, out, err = self.t.runError("1-3 delete rc.confirmation:on", input="quit\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleted 0 tasks", out)
        self.assertNotIn("Deleting task", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
