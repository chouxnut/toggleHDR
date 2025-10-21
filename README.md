# toggleHDR
Toggles HDR mode on Windows
Windows 11 has a Winkey+Alt+B shortcut to toggle HDR mode already, but there is an issue with loading monitor profiles (ICC files) from 24H2 onwards.  I noticed that after toggling HDR mode on or off you must load Display Settings in order to get the display profile to apply.  I got tired of doing this manually, so toggleHDR will turn HDR on or off and then briefly load Display Settings so the profile gets loaded, and then it exits.

Usage

Just run toggleHDR.exe and that's it.  Create a shortcut and pin it to your taskbar and/or start menu for quick access.  You can add the command line switch --noicc if you don't have custom monitor profiles.

toggleHDR.exe --no-icc

But if you're using HDR mode on Win11 you really go to the MS Store and get Microsoft's "Windows HDR Calibration" app to make sure your black level and white level are correct for your monitor.  Usually that's really just cranking the black level all the way down to 0 nits and the white level up to 1000 nits.  That will create a custom display profile for HDR mode that sets those correctly.  If you don't do that then your blacks and darks will be too bright, and your whites and brights won't be as bright as they're supposed to be.
