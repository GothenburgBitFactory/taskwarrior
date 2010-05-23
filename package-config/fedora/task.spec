Name:           task
Version:        1.9.1
Release:        1%{?dist}
Summary:        A command-line to do list manager

Group:          Applications/Productivity
License:        GPLv2+
URL:            http://taskwarrior.org
Source0:        http://taskwarrior.org/download/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  ncurses-devel

%description
Task is a command-line to do list manager. It has
support for GTD functionality and includes the
following features: tags, colorful tabular output,
reports and graphs, lots of manipulation commands,
low-level API, abbreviations for all commands and
options, multi-user file locking, recurring tasks.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/bash_completion.d
install -m 644 -T scripts/bash/task_completion.sh $RPM_BUILD_ROOT%{_sysconfdir}/bash_completion.d/task

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING NEWS README scripts i18n doc/rc
%{_bindir}/task
%{_mandir}/man1/task.1.gz
%{_mandir}/man5/taskrc.5.gz
%{_mandir}/man5/task-tutorial.5.gz
%{_mandir}/man5/task-color.5.gz
%{_mandir}/man5/task-faq.5.gz
%config(noreplace) %{_sysconfdir}/bash_completion.d


%changelog
* Mon May 22 2010 Federico Hernandez <ultrafredde@gmail.com> - 1.9.1-1
  Intial RPM for task beta release 1.9.1
* Mon Feb 22 2010 Federico Hernandez <ultrafredde@gmail.com> - 1.9.0-1
  Intial RPM for task beta release 1.9.0
* Mon Feb 15 2010 Federico Hernandez <ultrafredde@gmail.com> - 1.9.0.beta3-1
  Intial RPM for task beta release 1.9.0.beta3
* Mon Feb 08 2010 Federico Hernandez <ultrafredde@gmail.com> - 1.9.0.beta2-1
  Intial RPM for task beta release 1.9.0.beta2
* Wed Feb 03 2010 Federico Hernandez <ultrafredde@gmail.com> - 1.9.0.beta1-1
  Intial RPM for task beta release 1.9.0.beta1
* Sat Dec 05 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.5-2
  Fixed wrong ChangeLog file
* Sat Dec 05 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.5-1
  Intial RPM for task bugfix release 1.8.5
* Tue Nov 17 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.4-1
  Intial RPM for task bugfix release 1.8.4
* Wed Oct 21 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.3-1
  Intial RPM for task bugfix release 1.8.3
* Mon Sep 07 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.2-1
  Intial RPM for task bugfix release 1.8.2
* Thu Aug 20 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.1-1
  Intial RPM for task bugfix release 1.8.1
* Tue Jul 21 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.0-1
  Intial RPM for task release 1.8.0
* Mon Jul 13 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.0.beta3-1
  Intial RPM for task beta release 1.8.0.beta3
* Wed Jul 08 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.0.beta2-1
  Intial RPM for task beta release 1.8.0.beta2
* Tue Jul 07 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.8.0.beta1-1
  Intial RPM for task beta release 1.8.0.beta1
* Tue Jun 08 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.1-2
  Fixed inclusion of manpages.
* Tue Jun 08 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.1-1
  Initial RPM for bugfix release 1.7.1.
  Updated references to new project homepage in spec file. 
* Tue May 19 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.0-2
  Changed license to GPLv2+ and removed Requires macro.
  See https://bugzilla.redhat.com/show_bug.cgi?id=501498
* Tue May 19 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.0-1
  Initial RPM.
