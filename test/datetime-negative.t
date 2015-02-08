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

import json
import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class BaseDateTimeNegativeTest(TestCase):
    """
    Base class for Datetime negative tests.
    """

    def setUp(self):
        self.t = Task()

    def assertInvalidDatetimeFormat(self, value):
        self.t.runError(('add', 'due:%s' % value, 'test1'))
        self.t.runError(('add', 'scheduled:%s' % value, 'test2'))
        self.t.runError(('add', 'wait:%s' % value, 'test3'))
        self.t.runError(('add', 'until:%s' % value, 'test4'))


class TestIncorrectDate(BaseDateTimeNegativeTest):
    """
    This test case makes sure various formats
    of incorrect datetimes do not get interpreted
    as valid timetamps. Covers TW-1499.
    """

    def test_set_incorrect_datetime_randomstring(self):
        self.assertInvalidDatetimeFormat('random')

    def test_set_incorrect_datetime_negative_in_YYYY_MM_DD(self):
        self.assertInvalidDatetimeFormat('-2014-07-07')

    def test_set_incorrect_datetime_missing_day_in_YYYY_MM_DD(self):
        self.assertInvalidDatetimeFormat('2014-07-')

#    def test_set_incorrect_datetime_month_zero_in_YYYY_MM_DD(self):
#        self.assertInvalidDatetimeFormat('2014-0-12')

    def test_set_incorrect_datetime_invalid_characters_in_YYYY_MM_DD(self):
        self.assertInvalidDatetimeFormat('abcd-ab-ab')

#    def test_set_incorrect_datetime_day_as_zeros_in_YYYY_DDD(self):
#        self.assertInvalidDatetimeFormat('2014-000')

#    def test_set_incorrect_datetime_overlap_day_in_nonoverlap_year_in_YYYY_DDD(self):
#        self.assertInvalidDatetimeFormat('2014-366')

#    def test_set_incorrect_datetime_medium_overlap_day_in_YYYY_DDD(self):
#        self.assertInvalidDatetimeFormat('2014-999')

    def test_set_incorrect_datetime_huge_overlap_day_in_YYYY_DDD(self):
        self.assertInvalidDatetimeFormat('2014-999999999')

#    def test_set_incorrect_datetime_week_with_the_number_zero_in_YYYY_Www(self):
#        self.assertInvalidDatetimeFormat('2014-W00')

    def test_set_incorrect_datetime_overflow_in_week_in_YYYY_Www(self):
        self.assertInvalidDatetimeFormat('2014-W54')

    def test_set_incorrect_datetime_day_zero_in_YYYY_WwwD(self):
        self.assertInvalidDatetimeFormat('2014-W240')

    def test_set_incorrect_datetime_day_eight_in_YYYY_WwwD(self):
        self.assertInvalidDatetimeFormat('2014-W248')

    def test_set_incorrect_datetime_day_two_hundred_in_YYYY_WwwD(self):
        self.assertInvalidDatetimeFormat('2014-W24200')

#    def test_set_incorrect_datetime_week_with_the_number_zero_in_YYYYWww(self):
#        self.assertInvalidDatetimeFormat('2014W00')

    def test_set_incorrect_datetime_overflow_in_week_in_YYYYWww(self):
        self.assertInvalidDatetimeFormat('2014W54')

#    def test_set_incorrect_datetime_week_zero_in_YYYYWwwD(self):
#        self.assertInvalidDatetimeFormat('2014W001')

#    def test_set_incorrect_datetime_fifth_day_of_week_zero_in_YYYYWwwD(self):
#        self.assertInvalidDatetimeFormat('2014W005')

    def test_set_incorrect_datetime_overflow_week_in_YYYYWwwD(self):
        self.assertInvalidDatetimeFormat('2014W541')

    def test_set_incorrect_datetime_huge_overflow_week_in_YYYYWwwD(self):
        self.assertInvalidDatetimeFormat('2014W991')

#    def test_set_incorrect_datetime_day_zero_in_YYYYWwwD(self):
#        self.assertInvalidDatetimeFormat('2014W240')

    def test_set_incorrect_datetime_day_eight_in_YYYYWwwD(self):
        self.assertInvalidDatetimeFormat('2014W248')

    def test_set_incorrect_datetime_day_two_hundred_in_YYYYWwwD(self):
        self.assertInvalidDatetimeFormat('2014W24200')

#    def test_set_incorrect_datetime_month_zero_in_YYYY_MM(self):
#        self.assertInvalidDatetimeFormat('2014-00')

#    def test_set_incorrect_datetime_overflow_month_in_YYYY_MM(self):
#        self.assertInvalidDatetimeFormat('2014-13')

#    def test_set_incorrect_datetime_huge_overflow_month_in_YYYY_MM(self):
#        self.assertInvalidDatetimeFormat('2014-99')


class TestIncorrectTime(BaseDateTimeNegativeTest):
    """
    This test class makes sure invalid time formats are not getting
    accepted by TaskWarrior parser.
    """

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('24:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('99:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:60')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:99')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:ab')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:12')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:cd')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('-12:12')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:-12')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mmZ(self):
#        self.assertInvalidDatetimeFormat('24:00Z')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('99:00Z')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mmZ(self):
#        self.assertInvalidDatetimeFormat('12:60Z')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mmZ(self):
#        self.assertInvalidDatetimeFormat('12:99Z')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('12:abZ')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('ab:12Z')

    def test_set_incorrect_datetime_invalid_time_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('ab:cdZ')

    def test_set_incorrect_datetime_negative_hours_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('-12:12Z')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mmZ(self):
        self.assertInvalidDatetimeFormat('12:-12Z')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('24:00+01:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('99:00+01:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:60+01:00')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:99+01:00')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:ab+01:00')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:12+01:00')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:cd+01:00')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('-12:12+01:00')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:-12+01:00')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('24:00-01:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('99:00-01:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:60-01:00')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:99-01:00')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:ab-01:00')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:12-01:00')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:cd-01:00')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('-12:12-01:00')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:-12-01:00')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('24:00:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('99:00:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:60:00')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:99:00')

#    def test_set_incorrect_datetime_second_overflow_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:60')

#    def test_set_incorrect_datetime_huge_second_overflow_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:99')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:ab:00')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('ab:12:00')

    def test_set_incorrect_datetime_invalid_seconds_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:ab')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('ab:cd:ef')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('-12:12:12')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:-12:12')

#    def test_set_incorrect_datetime_negative_seconds_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:-12')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_ssZ(self):
#        self.assertInvalidDatetimeFormat('24:00:00Z')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('99:00:00Z')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_ssZ(self):
#        self.assertInvalidDatetimeFormat('12:60:00Z')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_ssZ(self):
#        self.assertInvalidDatetimeFormat('12:99:00Z')

#    def test_set_incorrect_datetime_second_overflow_in_hh_mm_ssZ(self):
#        self.assertInvalidDatetimeFormat('12:12:60Z')

#    def test_set_incorrect_datetime_huge_second_overflow_in_hh_mm_ssZ(self):
#        self.assertInvalidDatetimeFormat('12:12:99Z')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('12:ab:00Z')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('ab:12:00Z')

    def test_set_incorrect_datetime_invalid_seconds_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('12:12:abZ')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('ab:cd:efZ')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('-12:12:12Z')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('12:-12:12Z')

    def test_set_incorrect_datetime_negative_seconds_in_hh_mm_ssZ(self):
        self.assertInvalidDatetimeFormat('12:12:-12Z')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_ss_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('24:00:00+01:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('95:00:00+01:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_ss_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:60:00+01:00')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_ss_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:99:00+01:00')

#    def test_set_incorrect_datetime_second_overflow_in_hh_mm_ss_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12:60+01:00')

#    def test_set_incorrect_datetime_huge_second_overflow_in_hh_mm_ss_plus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12:99+01:00')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:ab:00+01:00')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:12:00+01:00')

    def test_set_incorrect_datetime_invalid_seconds_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12:ab+01:00')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:cd:ef+01:00')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('-12:12:12+01:00')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:-12:12+01:00')

    def test_set_incorrect_datetime_negative_seconds_in_hh_mm_ss_plus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12:-12+01:00')

#    def test_set_incorrect_datetime_hour_overflow_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('24:00:00-01:00')

    def test_set_incorrect_datetime_huge_hour_overflow_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('95:00:00-01:00')

#    def test_set_incorrect_datetime_minute_overflow_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:60:00-01:00')

#    def test_set_incorrect_datetime_huge_minute_overflow_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:99:00-01:00')

#    def test_set_incorrect_datetime_second_overflow_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12:60-01:00')

#    def test_set_incorrect_datetime_huge_second_overflow_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12:99-01:00')

    def test_set_incorrect_datetime_invalid_minutes_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:ab:00-01:00')

    def test_set_incorrect_datetime_invalid_hours_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:12:00-01:00')

    def test_set_incorrect_datetime_invalid_seconds_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12:ab-01:00')

    def test_set_incorrect_datetime_invalid_time_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('ab:cd:ef-01:00')

    def test_set_incorrect_datetime_negative_hours_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('-12:12:12-01:00')

    def test_set_incorrect_datetime_negative_minutes_in_hh_mm_ss_minus_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:-12:12-01:00')

#    def test_set_incorrect_datetime_negative_seconds_in_hh_mm_ss_minus_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12:-12-01:00')

#    def test_set_incorrect_datetime_invalid_hour_positive_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12+13:00')

#    def test_set_incorrect_datetime_invalid_medium_hour_positive_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12+24:00')

    def test_set_incorrect_datetime_invalid_huge_hour_positive_offset_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12+99:00')

#    def test_set_incorrect_datetime_invalid_minute_positive_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12+03:60')

#    def test_set_incorrect_datetime_invalid_huge_minute_positive_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12+03:99')

    def test_set_incorrect_datetime_invalid_hour_positive_offset_length_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12+3:20')

    def test_set_incorrect_datetime_invalid_minute_positive_offset_lentgh_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12+03:2')

    def test_set_incorrect_datetime_invalid_positive_offset_length_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12+3:2')

#    def test_set_incorrect_datetime_invalid_hour_negative_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12-13:00')

#    def test_set_incorrect_datetime_invalid_medium_hour_negative_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12-24:00')

    def test_set_incorrect_datetime_invalid_huge_hour_negative_offset_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12-99:00')

#    def test_set_incorrect_datetime_invalid_minute_negative_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12-03:60')

#    def test_set_incorrect_datetime_invalid_huge_minute_negative_offset_in_hh_mm_ss(self):
#        self.assertInvalidDatetimeFormat('12:12:12-03:99')

    def test_set_incorrect_datetime_invalid_hour_negative_offset_length_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12-3:20')

    def test_set_incorrect_datetime_invalid_minute_negative_offset_lentgh_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12-03:2')

    def test_set_incorrect_datetime_invalid_negative_offset_length_in_hh_mm_ss(self):
        self.assertInvalidDatetimeFormat('12:12:12-3:2')

#    def test_set_incorrect_datetime_invalid_hour_positive_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12+13:00')

#    def test_set_incorrect_datetime_invalid_medium_hour_positive_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12+24:00')

    def test_set_incorrect_datetime_invalid_huge_hour_positive_offset_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12+99:00')

#    def test_set_incorrect_datetime_invalid_minute_positive_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12+03:60')

#    def test_set_incorrect_datetime_invalid_huge_minute_positive_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12+03:99')

    def test_set_incorrect_datetime_invalid_hour_positive_offset_length_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12+3:20')

    def test_set_incorrect_datetime_invalid_minute_positive_offset_lentgh_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12+03:2')

    def test_set_incorrect_datetime_invalid_positive_offset_length_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12+3:2')

#    def test_set_incorrect_datetime_invalid_hour_negative_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12-13:00')

#    def test_set_incorrect_datetime_invalid_medium_hour_negative_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12-24:00')

    def test_set_incorrect_datetime_invalid_huge_hour_negative_offset_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12-99:00')

#    def test_set_incorrect_datetime_invalid_minute_negative_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12-03:60')

#    def test_set_incorrect_datetime_invalid_huge_minute_negative_offset_in_hh_mm(self):
#        self.assertInvalidDatetimeFormat('12:12-03:99')

    def test_set_incorrect_datetime_invalid_hour_negative_offset_length_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12-3:20')

    def test_set_incorrect_datetime_invalid_minute_negative_offset_lentgh_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12-03:2')

    def test_set_incorrect_datetime_invalid_negative_offset_length_in_hh_mm(self):
        self.assertInvalidDatetimeFormat('12:12-3:2')

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
