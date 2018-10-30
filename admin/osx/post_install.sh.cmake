#!/bin/sh

# Check if Finder is running (for systems with Finder disabled)
finder_status=`ps aux | grep "/System/Library/CoreServices/Finder.app/Contents/MacOS/Finder" | grep -v "grep"`
if ! [ "$finder_status" == "" ] ; then # Finder is running
   osascript << EOF
tell application "Finder"
   activate
   select the last Finder window
	reveal POSIX file "/Applications/@APPLICATION_EXECUTABLE@.app"
end tell
EOF
fi

exit 0
