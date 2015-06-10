# -*- coding: utf-8 -*-

import unittest
import sys
from .utils import TASKW_SKIP, TASKD_SKIP
from .taskd import Taskd


class BaseTestCase(unittest.TestCase):
    def tap(self, out):
        sys.stderr.write("--- tap output start ---\n")
        for line in out.splitlines():
            sys.stderr.write(line + '\n')
        sys.stderr.write("---  tap output end  ---\n")


@unittest.skipIf(TASKW_SKIP, "TASKW_SKIP set, skipping task tests.")
class TestCase(BaseTestCase):
    """Automatically skips tests if TASKW_SKIP is present in the environment
    """
    pass


@unittest.skipIf(TASKD_SKIP, "TASKD_SKIP set, skipping taskd tests.")
@unittest.skipIf(Taskd.not_available(), "Taskd binary not available at '{0}'"
                                        .format(Taskd.DEFAULT_TASKD))
class ServerTestCase(BaseTestCase):
    """Automatically skips tests if TASKD_SKIP is present in the environment
    """
    pass


# vim: ai sts=4 et sw=4
