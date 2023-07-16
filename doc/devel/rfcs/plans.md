---
title: "Plans"
---

There are many interconnected features and technologies in Taskwarrior, Taskserver, Tasksh and Timewarrior, each piece having it's own goals.

This matrix allows a simple reading of where things are, and where they are going.
This is a low-resolution time line.
It is subject to change.
It does not constitute a concrete plan.
This is an all-volunteer effort, and scheduling is difficult.

[Last updated 2016-08-08.]

<table class="table table-bordered table-striped">
<tr>
  <th>Taskwarrior<br />Technology/Feature</th>
  <th>
    <span class="label label-success">2.5.1</span><br />
    Current<br /><br />
    Released 2016-02-24
  </th>
  <th>
    <span class="label label-danger">2.6.0</span><br />
    Next<br /><br />
    2017
  </th>
  <th>
    <span class="label label-info">2.x</span><br />
    Future
  </th>
</tr>

<tr>
  <td>Core</td>
  <td>
    <a href="/docs/dom.html">DOM</a><br />
    Filters<br />
    Expressions<br />
    Color Rules<br />
    Custom Reports<br />
    Annotations<br />
    Tags / Virtual Tags<br />
    <a href="/docs/context.html">Context</a><br />
  </td>
  <td>
    <a href="/docs/design/recurrence.html">Recurrence</a><br />
    Shared library<br />
    <code>purge</code> command<br />
  </td>
  <td>
    True Color
  </td>
</tr>

<tr>
  <td>API</td>
  <td>
    <a href="/docs/design/task.html">JSON</a><br />
    Import<br />
    Export<br />
    <a href="/docs/hooks.html">Hooks</a><br />
    <a href="/docs/hooks2.html">Hooks v2</a><br />
    <a href="/docs/dom.html">DOM</a><br />
    Helper commands<br />
  </td>
  <td>
  </td>
  <td>
    <code>on-sync</code> hook<br />
    Full DOM<br />
    DOM access in rc<br />
    <code>$ENV</code> access in rc<br />
    Report columns as DOM refs<br />
  </td>
</tr>

<tr>
  <td>
    Attributes<br />
    <a href="/docs/udas.html">User Defined Attributes (UDA)</a>
  </td>
  <td>
    <code>modified</code><br />
    <code>priority</code> as a UDA<br />
  </td>
  <td>
    <code>template</code><br />
    <code>rtype</code><br />
    Remove <code>mask</code><br />
    Remove <code>imask</code><br />
    Remove <code>parent</code><br />
  </td>
  <td>
    <code>org</code><br />
    <code>group</code><br />
  </td>
</tr>

<tr>
  <td>Reports</td>
  <td>
    Improved layouts<br />
    Improved Themes
  </td>
  <td>
    Daily, Weekly reports (history, ghistory)<br />
  </td>
  <td>
  </td>
</tr>

<tr>
  <td>Synchronization</td>
  <td>
    <code>task sync</code><br />
    <code>task sync init</code> (all tasks)<br />
  </td>
  <td>
  </td>
  <td>
    <code>task sync reset</code><br />
  </td>
</tr>

<tr>
  <td>TDB (task database)</td>
  <td>
    Local file locking<br />
    Single file set<br />
    Single user
  </td>
  <td>
  </td>
  <td>
    Threaded file load<br />
    Read-only mode
  </td>
</tr>

<tr>
  <td>I18N / L10N</td>
  <td>
    UTF-8 support<br />
    <code>deu-DEU</code><br />
    <code>eng-USA</code><br />
    <code>epo-RUS</code><br />
    <code>esp-ESP</code><br />
    <code>fra-FRA</code><br />
    <code>ita-ITA</code><br />
    <code>pol-POL</code><br />
    <code>por-PRT</code><br />
  </td>
  <td>
    No I18N / L10N
  </td>
  <td>
    Migrate to <a href="https://www.gnu.org/software/gettext/">gettext</a><br />
  </td>
</tr>

<tr>
  <td>Documentation</td>
  <td>
    man: task<br />
    man: taskrc<br />
    man: task-color<br />
    man: task-sync<br />
    youtube: various<br />
    <a href="https://taskwarrior.org">taskwarrior.org</a><br />
    taskwarrior.com: Support Site<br />
  </td>
  <td>
  </td>
  <td>
    New video tutorials<br />
  </td>
</tr>

<tr>
  <td>Testing</td>
  <td>
    C++ tests<br />
    Python tests<br />
    Sync tests<br />
    Parallel tests<br />
  </td>
  <td>
    Migration to Flod2<br />
  </td>
  <td>
  </td>
</tr>

<tr>
  <td>Tool Chain</td>
  <td>
    GCC 4.7 / Clang 3.3<br />
    C++11 support<br />
    CMake<br />
  </td>
  <td>
    GCC 4.9 / Clang 3.4<br />
    Full C++11 support<br />
  </td>
  <td>
    Full C++14 support<br />
    Full C++17 support<br />
  </td>
</tr>
</table>

<table class="table table-bordered table-striped">
<tr>
  <th>Tasksh<br />Technology/Feature</th>
  <th>
    <span class="label label-success">1.1.0</span><br />
    Current<br /><br />
    Released 2016-09-05
  </th>
  <th>
    <span class="label label-danger">1.2.0</span><br />
    Next<br /><br />
    2017
  </th>
  <th>
    <span class="label label-info">1.x</span><br />
    Future
  </th>
</tr>

<tr>
  <td>Core</td>
  <td>
    <a href="/docs/review.html">Review</a><br />
    libreadline<br />
    Shared library<br />
  </td>
  <td>
  </td>
  <td>
    Pomodoro timer<br />
  </td>
</tr>

<tr>
  <td>Tool Chain</td>
  <td>
    CMake<br />
    GCC 4.7 / Clang 3.3<br />
  </td>
  <td>
    GCC 4.9 / Clang 3.4<br />
    Full C++11 support<br />
  </td>
  <td>
    Full C++14 support<br />
    Full C++17 support<br />
  </td>
</tr>
</table>

<table class="table table-bordered table-striped">
<tr>
  <th>Taskserver<br />Technology/Feature</th>
  <th>
    <span class="label label-success">1.1.0</span><br />
    Current<br /><br />
    Released 2015-05-10
  </th>
  <th>
    <span class="label label-danger">1.2.0</span><br />
    Next<br /><br />
    2017
  </th>
  <th>
    <span class="label label-info">1.x</span><br />
    Future
  </th>
</tr>

<tr>
  <td>Core</td>
  <td>
    Serial server
  </td>
  <td>
    Shared library<br />
  </td>
  <td>
    Threaded server
  </td>
</tr>

<tr>
  <td>Protocol</td>
  <td>
    v1
  </td>
  <td>
    v1.1 - client reset request<br />
  </td>
  <td>
    v1.2
  </td>
</tr>

<tr>
  <td>DB (Data Storage)</td>
  <td>
  </td>
  <td>
  </td>
  <td>
    GC
  </td>
</tr>

<tr>
  <td>Security</td>
  <td>
    Validation
  </td>
  <td>
  </td>
  <td>
    UUID:Cert Verification<br />
    Combined Certs
  </td>
</tr>

<tr>
  <td>Tool Chain</td>
  <td>
    GCC 4.7 / Clang 3.3<br />
    CMake<br />
  </td>
  <td>
    GCC 4.9 / Clang 3.4<br />
    Full C++11 support<br />
  </td>
  <td>
    Full C++14 support<br />
    Full C++17 support<br />
  </td>
</tr>
</table>


<table class="table table-bordered table-striped">
<tr>
  <th>Timewarrior<br />Technology/Feature</th>
  <th>
    <span class="label label-success">1.0.0</span><br />
    Current<br /><br />
    Released 2016-08-20
  </th>
  <th>
    <span class="label label-danger">1.1.0</span><br />
    Next<br /><br />
    2017
  </th>
  <th>
    <span class="label label-info">1.x</span><br />
    Future
  </th>
</tr>

<tr>
  <td>Core</td>
  <td>
    Shared library<br />
  </td>
  <td>
  </td>
  <td>
    True Color
  </td>
</tr>

<tr>
  <td>Reports</td>
  <td>
    <code>summary</code> report<br />
    <code>gaps</code> report<br />
    <code>day</code> chart<br />
    <code>week</code> chart<br />
    <code>month</code> chart<br />
    <code>totals.py</code> extension<br />
  </td>
  <td>
  </td>
  <td>
  </td>
</tr>

<tr>
  <td>Rules</td>
  <td>
    Simple configuration rules
  </td>
  <td>
  </td>
  <td>
    Rule System<br />
  </td>
</tr>

<tr>
  <td>Integration</td>
  <td>
    Taskwarrior <code>on-modify</code> hook script
  </td>
  <td>
  </td>
  <td>
  </td>
</tr>

<tr>
  <td>Tool Chain</td>
  <td>
    CMake<br />
    GCC 4.7 / Clang 3.3<br />
    C++11 support<br />
  </td>
  <td>
    GCC 4.9 / Clang 3.4<br />
    Full C++11 support<br />
  </td>
  <td>
    Full C++14 support<br />
    Full C++17 support<br />
  </td>
</tr>
</table>
