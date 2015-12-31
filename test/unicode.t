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


class TestUnicode(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_direct_utf8(self):
        """Verify that UTF8 can be directly entered"""
        self.t("add Çirçös")
        self.t("add Hello world ☺")
        self.t("add ¥£€¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯ ")
        self.t("add Pchnąć w tę łódź jeża lub ośm skrzyń fig")
        self.t("add ๏ เป็นมนุษย์สุดประเสริฐเลิศคุณค่า")
        self.t("add イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラムイ ロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム")
        self.t("add いろはにほへとちりぬるを")
        self.t("add D\\'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh")
        self.t("add Árvíztűrő tükörfúrógép")
        self.t("add Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa")
        self.t("add Sævör grét áðan því úlpan var ónýt")
        self.t("add Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen Wolther spillede på xylofon.")
        self.t("add Falsches Üben von Xylophonmusik quält jeden größeren Zwerg")
        self.t("add Zwölf Boxkämpfer jagten Eva quer über den Sylter Deich")
        self.t("add Heizölrückstoßabdämpfung")
        self.t("add Γαζέες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο")
        self.t("add Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία")

        # 17 tasks means none of the above failed.
        code, out, err = self.t("ls")
        self.assertIn("17", out)

    def test_utf8_project(self):
        """Verify that UTF8 can be used in a project name"""
        self.t("add project:Çirçös utf8 in project")
        code, out, err = self.t("_get 1.project")
        self.assertEqual("Çirçös\n", out)

    def test_utf8_tag(self):
        """Verify that UTF8 can be used in a tag"""
        self.t("add utf8 in tag +Zwölf");
        code, out, err = self.t("_get 1.tags")
        self.assertEqual("Zwölf\n", out);

    def test_unicode_escape1(self):
        """Verify U+NNNN unicode sequences"""
        self.t("add Price U+20AC4")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("Price €4\n", out);

    def test_unicode_escape2(self):
        """Verify \\uNNNN unicode sequences"""

        # The following can be used to prove that \\\\ --> \.
        # The Python string converts \\\\ --> \\.
        # Something in the launch code converts \\ --> \.
        # Taskwarrior sees \.
        #
        #code, out, err = self.t("add rc.debug.parser=2 Price \\\\u20A43")
        #self.tap(err)

        self.t("add Price \\\\u20A43")
        code, out, err = self.t("_get 1.description")
        self.assertEqual("Price ₤3\n", out);

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
