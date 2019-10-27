# Cursor Locker Redux

This is the repository for a program hosted on NexusMods https://www.nexusmods.com/skyrim/mods/95474

Compared to the original Mouselock utility, the redux version has a graphical user interface, as well as bug fixes, and all around cleaner and more efficient programming. You can find pre-compiled binaries in the releases section if you'd rather not compile it yourself. This project uses Qt5 which is cross-platform, however it is currently only compatible with Windows since it relies on WinAPI to function.


## Building
If you plan on building this yourself, you will need a [Qt5 Environment](https://www.qt.io/download-open-source) as well as a copy of the dynamic link libraries needed for Qt to function, since by default Qt does not statically link its own libraries. You can also find the necessary libraries included in the pre-compiled versions in the releases section.

The repository includes a Qt `.pro` project file which is already pre-configured and should work straight out of the box. You will however need a compatible compiler such as MSVC15 or MSVC17 -- or really just any compiler that is compatible with Windows' libraries.

## Screenshot
![](screenshots/screenshot-1.png?raw=true)
