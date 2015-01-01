#! /usr/bin/env perl
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

use strict;
use warnings;
use Test::More tests => 3;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

# Create the rc file.
if (open my $fh, '>', 'utf8.rc')
{
  print $fh "data.location=.\n";
  close $fh;
}

# Add a task with UTF8 in the description.
qx{../src/task rc:utf8.rc add Çirçös 2>&1};
qx{../src/task rc:utf8.rc add Hello world ☺ 2>&1};
qx{../src/task rc:utf8.rc add ¥£€¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯  2>&1};
qx{../src/task rc:utf8.rc add Pchnąć w tę łódź jeża lub ośm skrzyń fig 2>&1};
qx{../src/task rc:utf8.rc add ๏ เป็นมนุษย์สุดประเสริฐเลิศคุณค่า 2>&1};
qx{../src/task rc:utf8.rc add イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラムイ ロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム 2>&1};
qx{../src/task rc:utf8.rc add いろはにほへとちりぬるを 2>&1};
qx{../src/task rc:utf8.rc add D\\'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh 2>&1};
qx{../src/task rc:utf8.rc add Árvíztűrő tükörfúrógép 2>&1};
qx{../src/task rc:utf8.rc add Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa 2>&1};
qx{../src/task rc:utf8.rc add Sævör grét áðan því úlpan var ónýt 2>&1};
qx{../src/task rc:utf8.rc add Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen Wolther spillede på xylofon. 2>&1};
qx{../src/task rc:utf8.rc add Falsches Üben von Xylophonmusik quält jeden größeren Zwerg 2>&1};
qx{../src/task rc:utf8.rc add Zwölf Boxkämpfer jagten Eva quer über den Sylter Deich 2>&1};
qx{../src/task rc:utf8.rc add Heizölrückstoßabdämpfung 2>&1};
qx{../src/task rc:utf8.rc add Γαζέες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο 2>&1};
qx{../src/task rc:utf8.rc add Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία 2>&1};

my $output = qx{../src/task rc:utf8.rc ls 2>&1};
diag ($output);
like ($output, qr/17/, 'all 17 tasks shown');

qx{../src/task rc:utf8.rc add project:Çirçös utf8 in project 2>&1};
$output = qx{../src/task rc:utf8.rc ls project:Çirçös 2>&1};
like ($output, qr/Çirçös.+utf8 in project/, 'utf8 in project works');

qx{../src/task rc:utf8.rc add utf8 in tag +Zwölf 2>&1};
$output = qx{../src/task rc:utf8.rc ls +Zwölf 2>&1};
like ($output, qr/utf8 in tag/, 'utf8 in tag works');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data utf8.rc);
exit 0;

