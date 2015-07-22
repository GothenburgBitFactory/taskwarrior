# -*- coding: utf-8 -*-

from __future__ import print_function, division


class MetaTest(type):
    """Helper metaclass to simplify dynamic test creation

    Creates test_methods in the TestCase class dynamically named after the
    arguments used.
    """
    @staticmethod
    def make_function(classname, *args, **kwargs):
        def test(self):
            # ### Body of the usual test_testcase ### #
            # Override and redefine this method #
            pass

        # Title of test in report
        test.__doc__ = "{0}".format(args[0])

        return test

    def __new__(meta, classname, bases, dct):
        tests = dct.get("TESTS")
        kwargs = dct.get("EXTRA", {})

        for i, args in enumerate(tests):
            func = meta.make_function(classname, *args, **kwargs)

            # Rename the function after a unique identifier
            # Name of function must start with test_ to be ran by unittest
            func.__name__ = "test_{0}".format(i)

            # Attach the new test to the testclass
            dct[func.__name__] = func

        return super(MetaTest, meta).__new__(meta, classname, bases, dct)

# vim: ai sts=4 et sw=4
