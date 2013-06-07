These files are needed to workaround a bug in VS2012 Update 2, as
described below. Once update 3 is out, this directory can be removed
from the code repository, and the project file can be reverted.

This is from
http://tedwvc.wordpress.com/2013/04/14/how-to-get-visual-c-2012-update-2-statically-linked-applications-to-run-on-windows-xp/:

Due to a regression in Visual C++ 2012 Update 2, applications using
ATL will no longer be able to run on XP, even if using built-in XP
targeting support.  This is due to a new dependency on
InitializeCriticalSectionEx.  Running an app on XP that was built with
Visual C++ 2012 Update 2 will give you the following error message: 
Entry Point Not Found
The procedure entry point InitializeCriticalSectionEx could not be
located in the dynamic link library KERNEL32.dll
 
To fix this, I’ve published a new version of the xpsupport tool,
namely XPSupport for VC11 Update 2 Version 1.02, on my skydrive.  It
simply wraps the InitializeCriticalSectionEx function with an XP
compatible replacement, using techniques I’ve used in previous blog
entries. 

To use this solution, simply add build customizations (masm) to your
project by right clicking on the project in solution explorer and
choosing “build customizations” and check masm (targets, props) on,
then add the three files (xpatl.cpp, xpatlwrap.asm, and
xpatlwrap64.asm) contained in the zip file named:
xpsupportvc11upd2v102.zip found here: 
https://skydrive.live.com/redir?resid=1B361BF333E9AB82!153&authkey=!AEAOQJ4-LCqWQiw

Build your app, and now your app will run under XP.

EDIT: despite the title of this blog post, I received reports that
this works also for dynamically linked apps (i.e. linking to MFC and
the CRT dynamically) 
More info about the cause found here:
http://social.msdn.microsoft.com/Forums/en-US/visualstudiogeneral/thread/f0477c9f-8a2c-4e6b-9f5a-cd2469e216c4
credits to VSBs and others for reporting and researching this issue.
