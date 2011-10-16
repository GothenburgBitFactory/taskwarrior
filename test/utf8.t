#! /usr/bin/perl
################################################################################
## taskwarrior - a command line task list manager.
##
## Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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
use Test::More tests => 5;

# Create the rc file.
if (open my $fh, '>', 'utf8.rc')
{
  print $fh "data.location=.\n";
  close $fh;
  ok (-r 'utf8.rc', 'Created utf8.rc');
}

# Add a task with UTF8 in the description.
qx{../src/task rc:utf8.rc add Çirçös};
qx{../src/task rc:utf8.rc add Hello world ☺};
qx{../src/task rc:utf8.rc add ¥£€\$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯};
qx{../src/task rc:utf8.rc add Pchnąć w tę łódź jeża lub ośm skrzyń fig};
qx{../src/task rc:utf8.rc add ๏ เป็นมนุษย์สุดประเสริฐเลิศคุณค่า};
qx{../src/task rc:utf8.rc add イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム};
qx{../src/task rc:utf8.rc add いろはにほへとちりぬるを};
qx{../src/task rc:utf8.rc add D\\'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh};
qx{../src/task rc:utf8.rc add Árvíztűrő tükörfúrógép};
qx{../src/task rc:utf8.rc add Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa};
qx{../src/task rc:utf8.rc add Sævör grét áðan því úlpan var ónýt};
qx{../src/task rc:utf8.rc add Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen Wolther spillede på xylofon.};
qx{../src/task rc:utf8.rc add Falsches Üben von Xylophonmusik quält jeden größeren Zwerg};
qx{../src/task rc:utf8.rc add Zwölf Boxkämpfer jagten Eva quer über den Sylter Deich};
qx{../src/task rc:utf8.rc add Heizölrückstoßabdämpfung};
qx{../src/task rc:utf8.rc add Γαζέες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο};
qx{../src/task rc:utf8.rc add Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία};

my $output = qx{../src/task rc:utf8.rc ls};
diag ($output);
like ($output, qr/17/, 'all 17 tasks shown');

qx{../src/task rc:utf8.rc add project:Çirçös utf8 in project};
$output = qx{../src/task rc:utf8.rc ls project:Çirçös};
like ($output, qr/Çirçös.+utf8 in project/, 'utf8 in project works');

qx{../src/task rc:utf8.rc add utf8 in tag +Zwölf};
$output = qx{../src/task rc:utf8.rc ls +Zwölf};
like ($output, qr/utf8 in tag/, 'utf8 in tag works');

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data synch.key utf8.rc);
ok (! -r 'pending.data'   &&
    ! -r 'completed.data' &&
    ! -r 'undo.data'      &&
    ! -r 'backlog.data'   &&
    ! -r 'synch.key'      &&
    ! -r 'utf8.rc', 'Cleanup');

exit 0;

