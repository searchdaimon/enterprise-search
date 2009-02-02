%define name mp3info
%define version 0.8.5
%define release 2

Summary: An MP3 technical info viewer and ID3 v1.x tag editor
Name: %{name}
Version: %{version}
Release: %{release}
Packager: %{packager}
Vendor: %{vendor}
License: GPL
Group: Utilities/file
Source: ftp://ftp.ibiblio.org/pub/linux/apps/sound/mp3-utils/mp3info/%{name}-%{version}.tgz
BuildRoot: /tmp/%{name}

%description
MP3Info is an MP3 technical info viewer and ID3 1.x tag editor.
MP3Info has an interactive mode (using curses) and a command line mode.
A separate executable includes a GTK-based GUI version.  MP3Info can
display ID3 tag information as well as various techincal aspects of
an MP3 file including playing time, bit-rate, sampling frequency
and other attributes in a pre-defined or user-specifiable output format.

%prep

%setup -q 
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/%{name}-%{version}
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/X11R6/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1

%build
make
strip mp3info
strip gmp3info

%install
mv mp3info $RPM_BUILD_ROOT/usr/bin/mp3info
mv gmp3info $RPM_BUILD_ROOT/usr/X11R6/bin/gmp3info
mv ChangeLog README INSTALL LICENSE mp3info.txt mp3info.html $RPM_BUILD_ROOT/usr/share/doc/%{name}-%{version}
mv mp3info.1 $RPM_BUILD_ROOT/usr/man/man1

%post

%postun

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
/usr/bin/mp3info
/usr/X11R6/bin/gmp3info
/usr/man/man1/mp3info.1.gz
%doc /usr/share/doc/%{name}-%{version}

%changelog
* Mon Nov 6 2006 Cedric Tefft <cedric@phreaker.net>
- Added 'make install-mp3info' and 'make install-gmp3info' options to
  Makefile (Felix Kronlage)
- Fixed a bug in the windows version that caused it to blow up or give
  erroneous output on certain MP3 files
- Added handling of 'free form' bitrate frames, the lack of which was
  causing segfaults on AMD 64-bit sytems
- Tweaked code to eliminate various gcc warnings and errors
- Improved detection of invalid MP3 frames (Ben Bennett)
- Updated gmp3info for GTK 2 (Eric Lassauge)
- Miscellaneous documentation updates and corrections

* Mon Jul 16 2001 Cedric Tefft <cedric@phreaker.net>
- Added %k format specifier to allow printing of the file size
  in formatted text output (-p option)
- Rearranged some items in the man page and quick help (-h)
  to make them more readable.
- Fixed minor logic bug in mp3tech
- Now compiles under CYGWIN32
- Manual page typos fixed
- Now correctly recognizes and reports MPEG version 2.5 files
- Clearing individual ID3 fields can now be accomplished
  by passing a blank argument ("") to any tag setting
  switch (-t, -a, etc.)
