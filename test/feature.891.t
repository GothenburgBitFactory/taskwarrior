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


class TestFeature891(TestCase):
    @classmethod
    def setUp(self):
        self.t = Task()
        self.t("add one")
        self.t("add two")

        # Sometimes this test fails because the 1.uuid starts with N hex digits
        # such that those digits are all in the range [0-9], and therefore the
        # UUID looks like an integer.
        #
        # The only solution that comes to mind is to repeat self.t("add one")
        # until 1.uuid contains at least one [a-f] in the first N digits.

        code, self.uuid, err = self.t("_get 1.uuid")
        self.uuid = self.uuid.strip()

    def test_uuid_filter(self):
        for i in range(35,7,-1):
            code, out, err = self.t(self.uuid[0:i] + " list")
            self.assertIn("one", out)
            self.assertNotIn("two", out)

        # TODO This should fail because a 7-character UUID is not a UUID, but
        #      instead it blindly does nothing, and succeeds. Voodoo.
        #code, out, err = self.t(self.uuid[0:6] + " list")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
