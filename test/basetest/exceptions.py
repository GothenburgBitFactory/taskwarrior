# -*- coding: utf-8 -*-
import signal

sig_names = dict((k, v) for v, k in reversed(sorted(signal.__dict__.items()))
                 if v.startswith('SIG') and not v.startswith('SIG_'))


class CommandError(Exception):
    def __init__(self, cmd, code, out, err=None, msg=None):
        DEFAULT = ("Command '{{0}}' was {signal}'ed. "
                   "SIGABRT usually means task timed out.\n")
        if msg is None:
            msg_suffix = "\n*** Start STDOUT ***\n{2}\n*** End STDOUT ***\n"
            if err is not None:
                msg_suffix += (
                    "\n*** Start STDERR ***\n{3}\n*** End STDERR ***\n"
                )

            if code < 0:
                self.msg = DEFAULT.format(signal=sig_names[abs(code)])
            else:
                self.msg = ("Command '{0}' finished with unexpected exit "
                            "code '{1}'.\n")

            self.msg += msg_suffix
        else:
            self.msg = msg

        self.cmd = cmd
        self.out = out
        self.err = err
        self.code = code

    def __str__(self):
        return self.msg.format(self.cmd, self.code, self.out, self.err)


class HookError(Exception):
    pass


class TimeoutWaitingFor(object):
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return "*** Timeout reached while waiting for {0} ***".format(
            self.name)


class StreamsAreMerged(object):
    def __repr__(self):
        return "*** Streams are merged, STDERR is not available ***"

# vim: ai sts=4 et sw=4
