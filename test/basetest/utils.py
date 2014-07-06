# -*- coding: utf-8 -*-
import os
import socket
from subprocess import Popen, PIPE, STDOUT
from .exceptions import CommandError

USED_PORTS = set()


def run_cmd_wait(cmd, input=None, stdout=PIPE, stderr=PIPE,
                 merge_streams=False, env=os.environ):
    "Run a subprocess and wait for it to finish"

    if input is None:
        stdin = None
    else:
        stdin = PIPE

    if merge_streams:
        stderr = STDOUT
    else:
        stderr = PIPE

    p = Popen(cmd, stdin=stdin, stdout=stdout, stderr=stderr, env=env)
    out, err = p.communicate(input)

    # In python3 we will be able use the following instead of the previous
    # line to avoid locking if task is unexpectedly waiting for input
    # try:
    #     out, err = p.communicate(input, timeout=15)
    # except TimeoutExpired:
    #     p.kill()
    #     out, err = proc.communicate()

    if p.returncode != 0:
        raise CommandError(cmd, p.returncode, out, err)

    return p.returncode, out, err


def run_cmd_wait_nofail(*args, **kwargs):
    "Same as run_cmd_wait but silence the exception if it happens"
    try:
        return run_cmd_wait(*args, **kwargs)
    except CommandError as e:
        return e.code, e.out, e.err


def port_used(addr="localhost", port=None):
    "Return True if port is in use, False otherwise"
    if port is None:
        raise TypeError("Argument 'port' may not be None")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = s.connect_ex((addr, port))
    s.close()
    # result == 0 if connection was successful
    return result == 0


def find_unused_port(addr="localhost", start=53589, track=True):
    """Find an unused port starting at `start` port

    If track=False the returned port will not be marked as in-use and the code
    will rely entirely on the ability to connect to addr:port as detection
    mechanism. Note this may cause problems if ports are assigned but not used
    immediately
    """
    maxport = 65535
    unused = None

    for port in xrange(start, maxport):
        if not port_used(addr, port):
            if track and port in USED_PORTS:
                continue

            unused = port
            break

    if unused is None:
        raise ValueError("No available port in the range {0}-{1}".format(
            start, maxport))

    if track:
        USED_PORTS.add(unused)

    return unused


def release_port(port):
    """Forget that given port was marked as'in-use
    """
    try:
        USED_PORTS.remove(port)
    except KeyError:
        pass

# vim: ai sts=4 et sw=4
