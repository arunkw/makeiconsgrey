#  Project Idea Requirement
#1. Run indefinitely in the background, the app icon should be running in the “show hidden icons” area. (to make it easy to kill or exit)
#2. Any program which is running in windows, whose shortcut is also pinned to the taskbar keep its icon as-is (original default) whereas all other icons pinned to the taskbar should be changed to grey colour. 
#3. Example if VLC player and paint brush shortcuts are pinned to taskbar, but only VLC player is running in windows. Then this application should only change paintbrush icon to grey colour

# Forum Question
How to turn pinned taskbar icons to grey
I wanted to get the windows applications pinned on my taskbar to grey colour, except for those which are currently running. With help of ChatGPT, I tried autoit scripting using registry writing, I also tried Dev C++ using DLLs but it seems Windows does not provide an official API to directly change the appearance of pinned taskbar icons. The taskbar is managed by Windows Explorer, and there's no direct method to force a change in the taskbar icon's appearance for inactive applications.
(my git repository link - https://github.com/arunkw/makeiconsgrey)

How do I go about now, or should I give up on this project idea? 
