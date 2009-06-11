Name:           task
Version:        1.7.1
Release:        2%{?dist}
Summary:        A command-line to do list manager

Group:          Applications/Productivity
License:        GPLv2+
URL:            http://taskwarrior.org
Source0:        http://taskwarrior.org/download/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  ncurses-devel

%description
Task is a command-line to do list manager. It is
scope-limited to GTD functionality and features:
tags, colorful tabular output, reports and graphs,
lots of commands, low-level api, abbreviations for
all commands and options, multiuser file locking,
recurring tasks. Task is based on ideas presented
in the todo.sh script found on: http://todotxt.org

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING NEWS README task_completion.sh
%{_bindir}/task
%{_mandir}/man1/task.1.gz
%{_mandir}/man5/taskrc.5.gz


%changelog
* Tue Jun 08 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.1-2
- Fixed inclusion of manpages.
* Tue Jun 08 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.1-1
- Initial RPM for bugfix release 1.7.1.
- Updated references to new project homepage in spec file. 
* Tue May 19 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.0-2
- Changed license to GPLv2+ and removed Requires macro.
- See https://bugzilla.redhat.com/show_bug.cgi?id=501498
* Tue May 19 2009 Federico Hernandez <ultrafredde@gmail.com> - 1.7.0-1
- Initial RPM.
