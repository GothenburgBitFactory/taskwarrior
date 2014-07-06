# -*- coding: utf-8 -*-


class CommandError(Exception):
    def __init__(self, cmd, code, out, err, msg=None):
        if msg is None:
            self.msg = ("Command '{0}' finished with unexpected exit code "
                        "'{1}':\nStdout: '{2}'\nStderr: '{3}'")
        else:
            self.msg = msg

        self.cmd = cmd
        self.out = out
        self.err = err
        self.code = code

    def __str__(self):
        return self.msg.format(self.cmd, self.code, self.out, self.err)

# vim: ai sts=4 et sw=4
