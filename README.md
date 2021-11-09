# Cursor Locker
This is a simple program that confines your cursor to any game or application window, for games that do not natively do so and cause problems with multiple monitors where the cursor might escape onto the second or third monitor and issue unintended inputs. You can download a pre-compiled version in the [releases section](https://github.com/MisanthropicShayna/CursorLocker/releases) if you'd rather not compile it yourself. This project uses Qt5 which is cross-platform, however it is currently only compatible with Windows since it relies on WinAPI to function.

## Building
If you plan to build the project yourself using the intended method, you will need a [Qt5 Environment](https://www.qt.io/download-open-source) and a copy of the Qt runtime DLLs to make it portable, which you can acquire by running `windeployqt.exe C:\path\to\binary\<cursorlocker.exe>` that ships with Qt. You can also find the Qt runtime DLLs included in the pre-compiled release.

The repository includes a Qt `.pro` project file which is already pre-configured and should work straight out of the box. You will however need a compatible compiler such as MSVC15 or MSVC17 -- or really just any compiler that is compatible with Windows libraries.

## Screenshot
![](screenshots/screenshot-2.png?raw=true)
