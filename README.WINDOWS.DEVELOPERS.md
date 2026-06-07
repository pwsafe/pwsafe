# Introduction
This file describes how to setup a build environment for compiling Password Safe on a Windows environment, based on the community (free) edition of Microsift Visual Studio.

Last update: April 2024, based on input from Jason W.

# Setup
## Getting source code
The source code for Password Safe may be downloaded as a [zip file](https://github.com/pwsafe/pwsafe/archive/master.zip), or retrieved from the git repository via "git clone https://github.com/pwsafe/pwsafe.git".

If you're going to commit code to the repository, please set your git client's configuration file as follows:

In the file ".gitconfig"
```
[core]
	autocrlf = true
	safecrlf = warn
```
(This file under "C:\Users\<username>\" on Windows 7 and later. If using TortoiseGit client under Windows, this can be accessed by right clicking a file, selecting TortoiseGit and then select Settings from the menu. Within the "Git" setting "Config source", select the "Global" radio button and then set the "AutoCrlf" checkbox and set "SafeCrlf" to "warn". Alternatively, you can click the button "Edit global .gitconfig" to change the file manually).

For Windows, when installing "Git for Windows" (required by TortoiseGit and most other Git clients), ensure that you select "Checkout as-is, Commit as-is" to prevent incorrect line endings.

## Development Environment
1. Install VS 2022 Community through winget: `winget install -e --id Microsoft.VisualStudio.2022.Community`
2.  Install cmake through winget: `winget install -e --id Kitware.CMake`
3.  Launch the VS 2022 IDE, skip account and open without code
4.  Select Tools and Get Tools and Features
5.  In Workloads, select:
    1. Desktop development with C++
6. In Individual Components search for and select:
    1. C++ CMake tools for Windows
    2. Windows 11 SDK
    3. C++ MFC for latest v143 build tools (x86 & x64)


### Notes:

- If you skip CMake tools you'll get an error when running CMake that it cannot find any instance of visual studio
- If you skip Windows 11 SDK you'll get an error when running CMake "no cmake_cxx_compiler could be found"
- If you skip Desktop development with C++ or MFC, you'll be prompted to install them later when you open the generated solution

# Build under cmake
Assuming you have already cloned the Password Safe repository to your machine, after starting Visual Studio, you can select "Open a local folder", then select the toplevel folder of the cloned repository. Visual Studio should detect the toplevel CMakeLists.txt file and allow you to configure it. After that, you should be able to select a build configuration, e.g., Windows x64 Debug, build and run the project. For more details, see [README.CMAKE.md](https://github.com/pwsafe/pwsafe/blob/master/README.CMAKE.md).

Alternately, you can generate a solution file as follows:

# Generating a Solution
Run cmake -S . -B _build to generate the build files from the pwsafe folder, or use cmake-gui as described in [README.CMAKE.md](https://github.com/pwsafe/pwsafe/blob/master/README.CMAKE.md).

Double click on the generated _build/passwordsafe.sln file to open Visual Studio.

Right click pwsafe project in the solution and pick "Set as startup project". Then pick "start without debugging" (ctrl-F5) or other desired option if you want debugging, change to Release build, etc. Once it starts, it will launch Password Safe. Note, it looks like in debug mode it uses default settings and closing app doesn't end the process but leaves it in system tray. I had to remember to exit it there before restarting app.

# Wix MSI Installer
MSI installer files for the x86 and x64 platforms can be created using the Wix Toolset. The current version is 3.10. The Wix Toolset is available at http://wixtoolset.org/releases/

To build the MSI files, open a command prompt and change to the root directory of your source tree. Run the following command.

 install\windows\BuildWixInstall.bat
This will build MSI files for both x86 and x64 platforms in the root directory. 
You can build the files individually by running:
```
 install\windows\BuildWixInstall.bat x86
 install\windows\BuildWixInstall.bat x64
 ```
If you need to work on the Wix script (.wxs file), you can find it in the install\windows directory.
```
 pwsafe-template.wxs
```

# Minidump Production
In order to help debug user problems when Windows just says "there has been an error", a "Windows Unhandled Fault Handler" has been added. This will create a minidump, which can then be used to find where the error took place and, hopefully, why.

An extra "extended" Command Line flag of "--testdump" has been introduced for two reasons. Firstly, it allows the developer to test the minidump process, which is only available in the Release version of the executable and, secondly, allows the translators to check that the error screen displays as they wish.

Note to Translators: the error screen is a basic Windows function and has a fixed width. This is the reason why the minidump file name and directory have been split onto separate lines.

Note: Extended flags start with two minus signs ('--') and must be specified in full - i.e. no abbreviations. They are not displayed in the Usage notes for the normal users and no error message or display is shown if they are not recognised.

With this flag set, an extra button will appear in the bottom right hand corner of the "About Password Safe" dialog. Click it, and a minidump should be produced.

In order to process the minidump, either the free Microsoft Windbg program can be used (see Microsoft's site for details), or Visual Studio (VS) can be used. In the latter case (and probably the former too!):

1. You need the source corresponding to the failing release version. This should be available from SourceForge web site either in the normal Download section or from the Subversion repository, based on the revision number associated with that release. Without the corresponding source, VS can only show Windows source and line numbers in Password Safe source. With the corresponding source, VS can show you the exact line in Password Safe where the error took place and also the other statements within Password Safe as you follow the stack trace.
2. You need the Program Debug Database (pwsafe.pdb) associated with the failing release. This is not normally uploaded to the SourceForge web site with the release package. It is large (~11.5MB or ~3MB compressed). Since this must correspond to the Password Safe executable that had the error, maybe this and, possibly, the associated Link Map can be uploaded to a Developers section on this web site to aid all developers.
3. You should probably maintain the directory structure of the project as described in the source downloaded from SourceForge and place the Program Debug Database file in the '..\build\bin\pwsafe\release' directory.
4. Save the user's minidump file on your PC. Use VS to Open this as a Project (e.g. File->Open->Project/Solution).
5. In the Debugging Options, specify the directory containing the corresponding PDB file. (Go to: Tools->Options->Debugging->Symbols and add this directory in the section entitled "Symbol file (.pdb) locations:").
6. Press F5 to start debugging, and away you go.