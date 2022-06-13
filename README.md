# Cursor Locker
This is a simple program that confines your cursor to any game or application window, for games that do not natively do so and cause problems with multiple monitors where the cursor might escape onto the second or third monitor and issue unintended inputs. 

![code-quality-score](https://api.codiga.io/project/33791/score/svg)

## Downloads
You can download a pre-compiled build in the [releases section](https://github.com/MisanthropicShayna/CursorLocker/releases), which has been compiled with MSVC19 using Qt version 6.2.1. While Qt is cross-platform, the project relies on WinAPI to function, therefore only a Windows build is available.

## Building
If you plan to build the project yourself using the intended method, you will need a [Qt6 Environment](https://www.qt.io/download-open-source) and a copy of the needed Qt runtime DLLs to make it portable, which you can acquire by running the `windeployqt.exe` binary that ships with Qt, like so: `windeployqt.exe C:\path\to\binary\cursor-locker.exe` -- you can also find the Qt runtime DLLs included in the pre-compiled release, or in the Qt install directory.

The repository includes a Qt `.pro` project file which you can use to compile the project, assuming you have a Qt/qmake environment, and a compiler compatible with Windows libraries, such as MSVC17 or MSVC19. 

## Demo Gif
![](screenshots/demo_10fps.gif?raw=true)
