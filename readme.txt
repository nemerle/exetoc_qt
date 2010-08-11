About:
This is a port of exetoc project to Qt framework.
Since Qt is multi platform, it's possible to compile and run exetoc_qt under
windows and linux.
Since this is pretty much WIP, the UI is not very user friendly, also almost
no bugs in the original underlying decompilation engine were fixed.

Author:
orignal project's author : LiuTaoTao，bookaa at rorsoft.com
Qt port author : Artur K. nemerle5 [dwelling at] gmail.com

License:
All changes to the original project are licensed under BSD license.
The original project's license applies to everything else.

Prerequisites:
Qt>=4.6.2
Boost >= 1.40.0 (earlier versions should work too )
stdint.h if using visual studio version lower then 2008

Usage:
Copy the built application into BIN directory.
1. Run the application, select File/New, select a file ( the BIN/petest.exe is
a good first target ).
2. After the file is loaded, the function list will display all currently
'known' functions. Clicking on any function from this list will select it as
'current function', and pressing 'optim' will advance it's decompilation by
1 step.
3. Double-clicking a function form the list selects it, and displays current
code in the asm/internal code windows.

Warning:
Some original comments were googlated ( or google translated ) into something
vaguely resembling English, anyone reading Chinese/writing English is
very welcome to take a stab at translating original comments ( those can be
found in original sources ).


