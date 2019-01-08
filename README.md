# Mouselock

This is the source code for a Skyrim utility published on the Nexus. https://www.nexusmods.com/skyrim/mods/95474

If you're like me, and you constantly have other windows open on your other monitors, you probably experienced a phenomena at some point where if you scroll your mouse in game, it can scroll down a window on a completely separate monitor. This is due to the fact that Skyrim does not actually lock the cursor while you're playing, instead the cursor is actually just invisibly moving around in the background, which means your cursor can also wander onto other monitors and perform inputs that were never intended. 

To solve this pet peeve of mine, I wrote a small C++ program that essentially just locks your cursor to the center of your main monitor upon pressing a key. This works universally, so if there are any other games that don't lock the cursor, this program will work on them as well, since it's not really program specific. Optionally, you can also set it so that it automatically starts and stops when another program starts and stops.

