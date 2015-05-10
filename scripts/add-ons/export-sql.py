#! /usr/bin/python
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
"""
export-sql.py -- Export the taskwarrior database as a series of SQL commands.

Example usage::

    $ PYTHONIOENCODING=UTF-8 ./export-sql.py | sqlite3 mytasks.db
    $ /usr/bin/sqlite3 mytasks.db "select * from annotations;"

This script has only been tested with sqlite3, but in theory, it could be
easily modified to supported mysql, postgres or whatever you choose.

Author:  Ralph Bean
"""

import sys
import commands
import json

from datetime import datetime

# Note that you may want to modify the field sizes to suit your usage.
table_definitions = """
CREATE TABLE tasks (
    uuid VARCHAR(255) NOT NULL,
    description VARCHAR(255) NOT NULL,
    entry DATETIME NOT NULL,
    end DATETIME,
    priority VARCHAR(32),
    project VARCHAR(32),
    status VARCHAR(32),
    PRIMARY KEY (uuid)
);

CREATE TABLE annotations (
    uuid VARCHAR(255) NOT NULL,
    description VARCHAR(255) NOT NULL,
    entry DATETIME NOT NULL,
    FOREIGN KEY(uuid) REFERENCES tasks(uuid)
);
"""


replacements = {
    '"': '&dquot;',
    "'": '&quot;',
    '[': '&open;',
    ']': '&close;',
    '/': '\\/',
}


def escape(s):
    """ Escape a string in the taskwarrior style """

    for unsafe, safe in replacements.iteritems():
        s = s.replace(unsafe, safe)
    return s


# A lookup table for how to convert various values by type to SQL
conversion_lookup = {
    # Tack on an extra set of quotes
    unicode: lambda v: "'%s'" % escape(v),
    # Do the same as for unicode
    str: lambda v: convert(unicode(v)),
    # Convert to ISO format and do the same as for unicode
    datetime: lambda v: convert(v.isoformat(' ')),
    # Replace python None with SQL NULL
    type(None): lambda v: 'NULL',
}

# Compose a value with its corresponding function in conversion_lookup
convert = lambda v: conversion_lookup.get(type(v), lambda v: v)(v)


def parse_datetime(task):
    """ Parse the datetime strings given to us by `task export` """

    for key in ['entry', 'end']:
        if key in task:
            task[key] = datetime.strptime(task[key], "%Y%m%dT%H%M%SZ")
    return task


def to_sql(task):
    """ Create a list of SQL INSERT statements out of a task python dict """

    def make_annotation(annot):
        """ Create a list of SQL INSERT statements for an annotation """

        annot['uuid'] = task['uuid']
        template = "{uuid}, {description}, {entry}"
        annot = dict(zip(annot.keys(), map(convert, annot.values())))
        values = template.format(**annot)
        return "INSERT INTO \"annotations\" VALUES(%s)" % values

    template = u"{uuid}, {description}, {entry}, {end}, " + \
           u"{priority}, {project}, {status}"

    nullables = ['end', 'priority', 'project', 'status']
    defaults = dict([(key, None) for key in nullables])
    defaults['annotations'] = []
    defaults.update(task)

    defaults = dict(zip(defaults.keys(), map(convert, defaults.values())))

    values = template.format(**defaults)
    annotations = map(make_annotation, defaults['annotations'])

    return ["INSERT INTO \"tasks\" VALUES(%s)" % values] + annotations


def main():
    """ Return a list of SQL statements. """

    # Use the taskwarrior 2.0+ export command to filter and return JSON
    command = "task rc.verbose=nothing rc.json.array=yes " + " ".join(sys.argv[1:]) + " export"

    # Load each task from json to a python dict
    tasks = json.loads(commands.getoutput(command))

    # Mangle datetime strings into python datetime objects
    tasks = map(parse_datetime, tasks)

    # Produce formatted SQL statements for each task
    inserts = sum(map(to_sql, tasks), [])

    return inserts


if __name__ == '__main__':
    # Get the INSERT statements
    lines = main()

    # Combine them with semicolons
    sql = table_definitions + ";\n".join(lines) + ';'

    # Print them out, decorated with sqlite3 trappings
    print u"""
BEGIN TRANSACTION;
{sql}
COMMIT;""".format(sql=sql)

###############################################################################
