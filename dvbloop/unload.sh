#!/bin/sh
module="dvbloop"
device="dvblo"

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
rmmod ${module}
echo "unloaded module: ${dvbloop}"
# remove stale nodes
rm -f /dev/${device}[0-3]
echo "removed device files: /dev/${device}[0-3]"

