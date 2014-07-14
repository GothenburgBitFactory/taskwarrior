# -*- coding: utf-8 -*-

import unittest
import sys


class TestCase(unittest.TestCase):
    def diag(self, out):
        sys.stdout.write("# --- diag start ---\n")
        for line in out.split("\n"):
            sys.stdout.write("#" + line + "\n")
        sys.stdout.write("# ---  diag end  ---\n")


# vim: ai sts=4 et sw=4
