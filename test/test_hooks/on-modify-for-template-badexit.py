#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import json

original_task = sys.stdin.readline()
modified_task = sys.stdin.readline()

task = json.loads(modified_task)
task["description"] = "This is an example modify hook"

# A random message
sys.stdout.write("Hello from the template hook\n")

sys.stdout.write(json.dumps(task, separators=(',', ':')) + '\n')
sys.exit(1)

# vim: ai sts=4 et sw=4
