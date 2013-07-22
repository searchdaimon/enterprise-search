# -* wish *-
# fallback which allows me to run wordview.tcl without doing make
package require Tcl 8.3

if ![info exist charset_lib] {
  set charset_lib /usr/local/lib/catdoc
}
option add *Text.Font {Courier 11} widgetDefault
option add *Text.Background white widgetDefault
option add *Text.Foreground black widgetDefault
option add *Text.selectBackground black widgetDefault
option add *Text.selectForeground white widgetDefault
option add *Text.findMode exact widgetDefault
option add *Text.findCase no widgetDefault
option add *Menu.highlightBackground MidnightBlue widgetDefault
option add *Menu.highlightThickness 0 widgetDefault
option add *Menu.activeBackground MidnightBlue widgetDefault
option add *Menu.activeForeground white widgetDefault
option add *Menu.activeBorderWidth 0 widgetDefault
menu .mainmenu
. configure -menu .mainmenu
.mainmenu add cascade  -label File -menu [set m [menu .mainmenu.file]] -underline 0
$m add command -label Open... -command load_file -accelerator Ctrl-O
$m add command -label "Save As..." -command write_file -accelerator Ctrl-S -state disabled
$m add separator
$m add command -label Quit -command exit -accelerator Alt-F4
set m [menu .mainmenu.edit -postcommand EditEnable]
.mainmenu add cascade -label Edit -menu $m -underline 0 -state disabled
$m add command -label Copy -command CopySel -accelerator Ctrl-C
$m add separator
$m add command -label "Select All" -accelerator Ctrl-A -command \
 {.text tag add sel 0.0 end}
.mainmenu add cascade -label Find -menu .mainmenu.search -underline 1 -state disabled
set m [menu .mainmenu.search -postcommand EnableSearch]
$m add command -label "Find..." -command FindDialog -accelerator Ctrl-F
$m add command -label "Find Again" -accelerator F3 -command DoFind
#  
# build charset menu
# 

.mainmenu add cascade -state disabled -label Encoding -menu [set m [menu .mainmenu.encoding]]
$m add radio -label Default -value Default -var in_charset 
$m add radio -label unicode -value unicode -var in_charset 
foreach l [glob [file join $charset_lib *.txt]] {
    set n [file rootname [file tail $l]]
    $m add radio -label $n -value $n -var in_charset
}

set in_charset Default 

trace var in_charset w reread
set m [menu .mainmenu.help]
.mainmenu  add cascade -label Help -menu $m -underline 0
$m add command -label "Manual page" -command [list  show_help [file tail $argv0]]
$m add command -label "Regular expressions" -command {show_help re_syntax}
$m add separator
$m add command -label "About..." -command AboutDialog



text .text -width 80 -height 25  -xscrollcommand ".xs set" \
    -yscrollcommand ".ys set"   -wrap word \
     -spacing3 2m 
.text tag configure sel -relief flat -borderwidth 0
.text tag configure doc -lmargin1 0.2i -lmargin2 0
scrollbar .ys -orient vert -command ".text yview"
scrollbar .xs -orient horiz -command ".text xview"
bind .text <F3> { if [info exists FindPattern] DoFind}
bind .text <Control-O> load_file
bind .text <Control-o> load_file
bind .text <Control-S> {write_file}
bind .text <Control-s> {write_file}
bind .text <Control-F> FindDialog
bind .text <Control-f> FindDialog
grid .text .ys
grid .xs x  
grid .text -sticky news
grid .xs -sticky we
grid .ys -sticky ns
grid columnconfigure . 0 -weight 1 
grid columnconfigure . 1 -weight 0
grid rowconfigure . 0 -weight 1 
grid rowconfigure . 1 -weight 0

# Find options (All this can be tuned from dialog)
set FindMode -[option get .text findMode FindMode] ;# no -regexp for novices
set FindDir -forwards ;# Why not -backwards
set FindCase -nocase ;# Leave it empty if you want to be case sensitive
if {[option get .text findCase FindCase]} {
	set FindCase ""
}	


proc show_help {page} {
	global argv0
	if [winfo exists .man] {
		wm deiconify .man
		raise .man
		.man.text delete 0.0 end
	} else {	
		toplevel .man -class Man
		wm title .man "[file tail $argv0] help: $page"
		menu .man.menu 
		.man.menu add cascade -label File -menu [set m [menu .man.menu.file]]
		.man configure -menu .man.menu
		$m add command -label Close -command {destroy .man}
		text .man.text -yscrollcommand {.man.y set}
		scrollbar .man.y -command {.man.text yview} -orient vert
		grid .man.text .man.y -sticky news
		grid columnconfigure .man 0 -weight 1
		grid columnconfigure .man 1 -weight 0
	}
	.man.text insert end [exec man $page 2>/dev/null | col -b ]
}	

proc load_file {{name {}}} {
global filename
if ![string length $name] {set name [tk_getOpenFile -filetypes {
{{Msword files} .doc}
{{RTF files} .rtf}
{{MS Write files} .wri}
{{All files} *}} ]}
if ![string length $name] return
if ![file readable $name] {
  return -code error "Cannot open file $name"
}
set filename $name
.mainmenu entryconfigure Encoding -state normal
.mainmenu.file entryconfigure "Save As..." -state normal
.mainmenu  entryconfigure "Edit" -state normal
.mainmenu entryconfigure "Find"  -state normal
reread
}

proc make_opt {var flag} {
  upvar #0 $var charset
  switch $charset {
	"Default" {return ""}
	"unicode" {return "-u"}
        default {return "$flag $charset"}
  }
}	
proc reread {args} {
global filename in_charset out_charset

set inopt [make_opt in_charset -s]
set f [open "|catdoc -w $inopt -d utf-8 \"$filename\"" r]
fconfigure $f -encoding utf-8
.text configure -state normal
.text delete 0.0 end
.text insert 0.0 [read $f] doc
.text mark set insert 1.0
.text configure -state disabled
.text see 1.0
if [catch {close $f} msg] {
 tk_messageBox -icon error -title error -message $msg -type ok
 return
}
}
proc write_file {{name {}}} {
    global filename 
    if ![string length $name] {
       set name [tk_getSaveFile -filetypes {
      {{Text files} .txt}
      {{LaTeX files} .tex}}]
    }
    if ![string length $name] return
    if {[file extension $name]==".tex"} {
       eval exec catdoc -t [make_opt in_charset -s] [make_opt out_charset -d]\
		[list $filename] > [list $name]
    } else {
       eval exec catdoc [make_opt in_charset -s] [make_opt out_charset -d]\
		[list $filename]  > [list $name]
    }
}
# -postcommand for Edit menu
proc EditEnable {} {
if [llength [.text tag ranges sel]] {
  .mainmenu.edit entryconfigure Copy -state normal
} else {
  .mainmenu.edit entryconfigure Copy -state disabled
}
}
proc CopySel {} {
clipboard clear
clipboard append -- [.text get sel.first sel.last]
}
proc FindDialog {} {
make_transient .find "Find" 
frame .find.top
label .find.top.l -text "Find"
entry .find.top.e -width 30 -textvar FindPattern
bind .find.top.e <Key-Return> ".find.b.find invoke"
pack .find.top.l .find.top.e -side left
FindOptionFrame
frame .find.b
button .find.b.find -text "Search" -command DoFind
button .find.b.close -text "Close" -command "destroy .find"
pack .find.b.find .find.b.close -side left -padx 20
pack .find.top -pady 5 -anchor w -padx 10
pack .find.opt -pady 10
pack .find.b
focus .find.top.e
}
proc EnableSearch {} {
global FindPattern ReplaceString
if ![info exists FindPattern] {
  .mainmenu.search entryconfigure "Find Again" -state disabled
} else {
  .mainmenu.search entryconfigure "Find Again" -state normal
}
}
proc make_transient {wpath title} {
set x [expr [winfo rootx .]+[winfo width .]/3]
set y [expr [winfo rooty .]+[winfo height .]/3]
catch {destroy $wpath}
toplevel $wpath
wm transient $wpath .
wm positionfrom $wpath program
wm geometry $wpath +$x+$y
wm title  $wpath $title
}
proc FindOptionFrame {} {
frame .find.opt
checkbutton .find.opt.dir -variable FindDir -onvalue -backwards\
   -offvalue -forwards  -text Backward
checkbutton .find.opt.regex -variable FindMode -onvalue\
      -regex -offvalue -exact  -text RegExp
checkbutton .find.opt.case -variable FindCase -onvalue -nocase -offvalue {}\
  -text "Ignore case"
pack .find.opt.dir .find.opt.regex .find.opt.case -side left
}
proc DoFind {{quiet 0}} {
global FindPattern FindMode FindDir FindCase
if ![string length $FindPattern] {return 0}
if {$FindMode=="-backwords"} {  
    set stopindex 0.0
} else {
  set stopindex end
} 
set index [eval ".text search $FindCase $FindMode $FindDir -- \
  [list $FindPattern] insert $stopindex"] 
if ![string length $index] {
  if !$quiet {
   tk_messageBox -type ok -title "Not found" -message "Pattern not found"
  }
 return 0
} else {
.text tag remove sel 0.0 end
if {$FindMode=="-exact"} {
.text tag add sel $index "$index + [string length $FindPattern] chars"
} else {
eval "regexp $FindCase --" [list $FindPattern [.text get "$index linestart"\
   "$index lineend"] match]
.text tag add sel $index "$index + [string length $match] chars"
}
.text mark set insert sel.last 
.text see $index
.text see insert
focus .text
return 1
}
}
proc AboutDialog {} {
make_transient .about "About WordView"
message .about.m -aspect 250 -text "MS-Word viewer for UNIX
Copyright (c) by Victor B. Wagner 1997-98
This program is distributed under
GNU General Public License Version 2 or above
Check http://www.gnu.org/copyleft/gpl.html for copying
and warranty conditions" -justify center
button .about.ok -text Ok -command {destroy .about}
pack .about.m .about.ok
}
if [llength $argv] {
 if {![file exist [lindex $argv 0]]} {
    puts stderr "No such file: [lindex $argv 0]"
    exit 1
 }   
load_file [lindex $argv 0]
}
focus .text
