# -*- coding: utf-8 -*-
import signal


class CommandError(Exception):
    def __init__(self, cmd, code, out, err, msg=None):
        if msg is None:
            msg_suffix = (
                "\n*** Start STDOUT ***\n{2}\n*** End STDOUT ***\n"
                "\n*** Start STDERR ***\n{3}\n*** End STDERR ***\n"
                )
            if code == signal.SIGABRT:
                self.msg = ("Command '{0}' was aborted, likely due to not "
                            "finishing in due time. The exit code was '{1}'.\n"
                            ) + msg_suffix
            else:
                self.msg = ("Command '{0}' finished with unexpected exit "
                            "code '{1}'.\n"
                            ) + msg_suffix
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


class TimeoutWaitingForStream(object):
    def __init__(self, name):
        self.stream = name

    def __repr__(self):
        return "*** Timeout reached while waiting for %s ***".format(self.name)


class StreamsAreMerged(object):
    def __repr__(self):
        return "*** Streams are merged, STDERR is not available ***"

# vim: ai sts=4 et sw=4
