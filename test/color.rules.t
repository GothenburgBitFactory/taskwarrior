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
import platform
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestColorRules(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed only once, during class setup"""
        cls.t = Task()

        # Controlling peripheral color.
        cls.t.config('_forcecolor',           'on')
        cls.t.config('fontunderline',         'off')        # label underlining complicates the tests.
        cls.t.config('color.alternate',       '')           # alternating color complicateѕ the tests.
        cls.t.config('default.command',       'list')
        cls.t.config('uda.xxx.type',          'numeric')
        cls.t.config('uda.xxx.label',         'XXX')

        # Color rules.
        cls.t.config('color.active',          'red')
        cls.t.config('color.blocked',         'red')
        cls.t.config('color.blocking',        'blue')
        cls.t.config('color.due',             'red')
        cls.t.config('color.overdue',         'blue')
        cls.t.config('color.error',           'blue')
        cls.t.config('color.header',          'blue')
        cls.t.config('color.footnote',        'red')
        cls.t.config('color.debug',           'green')
        cls.t.config('color.project.x',       'red')
        cls.t.config('color.project.none',    '')
        cls.t.config('color.uda.priority.H',  'red')
        cls.t.config('color.uda.priority.M',  'blue')
        cls.t.config('color.uda.priority.L',  'green')
        cls.t.config('color.keyword.keyword', 'red')
        cls.t.config('color.tagged',          '')
        cls.t.config('color.tag.none',        '')
        cls.t.config('color.tag.x',           'red')
        cls.t.config('color.recurring',       'red')
        cls.t.config('color.uda.xxx',         'red')
        cls.t.config('color.uda.xxx.4',       'blue')

        cls.t('add control task')                         # 1
        cls.t('add active task')                          # 2
        cls.t('2 start')
        cls.t('add blocked task')                         # 3
        cls.t('add blocking task')                        # 4
        cls.t('3 modify depends:4')
        cls.t('add tomorrow  due:tomorrow')               # 5
        cls.t('add yesterday due:yesterday')              # 6
        cls.t('add someday   due:yesterday')              # 7
        cls.t('add project_x project:x')                  # 8
        cls.t('add pri_h     priority:H')                 # 9
        cls.t('add pri_m     priority:M')                 # 10
        cls.t('add pri_l     priority:L')                 # 11
        cls.t('add keyword')                              # 12
        cls.t('add tag_x     +x')                         # 13
        cls.t('add uda_xxx_1 xxx:1')                      # 14
        cls.t('add uda_xxx_4 xxx:4')                      # 15
        cls.t('add recurring due:tomorrow recur:1week')   # 16 # Keep this last

    def test_control(self):
        """No color on control task."""
        code, out, err = self.t('1 info')
        self.assertNotIn('\x1b[', out)

    def test_disable_in_pipe(self):
        """No color in pipe unless forced."""
        code, out, err = self.t('2 info rc._forcecolor:off')
        self.assertNotIn('\x1b[', out)

    def test_active(self):
        """Active color rule."""
        code, out, err = self.t('/active/ info')
        self.assertIn('\x1b[31m', out)

    def test_blocked(self):
        """Blocked color rule."""
        code, out, err = self.t('/blocked/ info')
        self.assertIn('\x1b[31m', out)

    def test_blocking(self):
        """Blocking color rule."""
        code, out, err = self.t('/blocking/ info')
        self.assertIn('\x1b[34m', out)

    def test_due_yesterday(self):
        """Overdue color rule."""
        code, out, err = self.t('/yesterday/ info')
        self.assertIn('\x1b[34m', out)

    def test_due_tomorrow(self):
        """Due tomorrow color rule."""
        code, out, err = self.t('/tomorrow/ info')
        self.assertIn('\x1b[31m', out)

    def test_due_someday(self):
        """Due someday color rule."""
        code, out, err = self.t('/someday/ info')
        self.assertIn('\x1b[', out)

    def test_color_error(self):
        """Error color."""
        code, out, err = self.t.runError('add error priority:X')
        self.assertIn('\x1b[34m', err)

    def test_color_header(self):
        """Header color."""
        code, out, err = self.t('rc.verbose=header /control/')
        self.assertIn('\x1b[34m', err)

    def test_color_footnote(self):
        """Footnote color."""
        code, out, err = self.t('rc.verbose=footnote /control/')
        self.assertIn('\x1b[31mConfiguration override', err)

    def test_color_debug(self):
        """Debug color."""
        code, out, err = self.t('rc.debug=1 /control/')
        self.assertIn('\x1b[32mTimer', err)

    def test_project_x(self):
        """Project x color rule."""
        code, out, err = self.t('/project_x/ info')
        self.assertIn('\x1b[31m', out)

    def test_project_none(self):
        """Project none color rule."""
        code, out, err = self.t('/control/ rc.color.project.none=red info')
        self.assertIn('\x1b[31m', out)

    def test_priority_h(self):
        """Priority H color rule."""
        code, out, err = self.t('/pri_h/ info')
        self.assertIn('\x1b[31m', out)

    def test_priority_m(self):
        """Priority M color rule."""
        code, out, err = self.t('/pri_m/ info')
        self.assertIn('\x1b[34m', out)

    def test_priority_l(self):
        """Priority L color rule."""
        code, out, err = self.t('/pri_l/ info')
        self.assertIn('\x1b[32m', out)

    def test_keyword(self):
        """Keyword color rule."""
        code, out, err = self.t('/keyword/ info')
        self.assertIn('\x1b[31m', out)

    def test_tag_x(self):
        """Tag x color rule."""
        code, out, err = self.t('/tag_x/ info')
        self.assertIn('\x1b[31m', out)

    def test_tag_none(self):
        """Tag none color rule."""
        code, out, err = self.t('/control/ rc.color.tag.none=red info')
        self.assertIn('\x1b[31m', out)

    def test_tagged(self):
        """Tagged color rule."""
        code, out, err = self.t('/tag_x/ rc.color.tag.x= rc.color.tagged=blue info')
        self.assertIn('\x1b[34m', out)

    def test_recurring(self):
        """Recurring color rule."""
        code, out, err = self.t('/recurring/ info')
        self.assertIn('\x1b[31m', out)

    def test_uda(self):
        """UDA color rule."""
        code, out, err = self.t('/uda_xxx_1/ info')
        self.assertIn('\x1b[31m', out)

    def test_uda_value(self):
        """UDA Value color rule."""
        code, out, err = self.t('/uda_xxx_4/ rc.color.uda.xxx= info')
        self.assertIn('\x1b[34m', out)

class TestColorRulesMerging(TestCase):

    def setUp(self):
        """Executed before every test in the class"""
        self.t = Task()

        # Controlling peripheral color.
        self.t.config('_forcecolor',           'on')
        self.t.config('fontunderline',         'off')        # label underlining complicates the tests.
        self.t.config('color.alternate',       '')           # alternating color complicateѕ the tests.
        self.t.config('default.command',       'list')

        # Color rules. Only due and tagged affect resulting color.
        self.t.config('color.due', 'red')
        self.t.config('color.tagged','on white')
        self.t.config('rule.color.precedence', 'due,tagged')

        self.t('add due:today +home hometask')  # Task that matches both color rules

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping color merge test for Cygwin')
    @unittest.skipIf('FreeBSD' in platform.system(), 'Skipping color merge test for FREEBSD')
    @unittest.expectedFailure
    def test_colors_merge(self):
        """Tests whether colors merge"""
        code, out, err = self.t('1 info')
        self.assertIn('\x1b[31;47mhometask', out)  # Red on white

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping color merge test for Cygwin')
    @unittest.skipIf('FreeBSD' in platform.system(), 'Skipping color merge test for FREEBSD')
    @unittest.expectedFailure
    @unittest.expectedFailure
    def test_colors_merge_off(self):
        """No color merge behaviour with rule.color.merge=no"""
        self.t.config('rule.color.merge', 'no')

        code, out, err = self.t('1 info')
        self.assertIn('\x1b[31mhometask', out)  # Red


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
