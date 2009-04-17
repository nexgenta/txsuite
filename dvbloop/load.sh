# This script is largely based on the book "Linux Device Drivers - 3rd Edition" 
# where it is used to load and setup the 'scull' example char driver
#

#!/bin/sh
module="dvbloop"
device="dvblo"
mode="664"
# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1
echo "loaded module: ${dvbloop}"
# remove stale nodes
rm -f /dev/${device}[0-3]
major=`awk "\\$2==\"$device\" {print \\$1}" /proc/devices`
if [ $? -ne 0 ]; then exit 1; fi
echo "Detected major device number ${major} for device ${device}"
mknod /dev/${device}0 c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3
# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}[0-3]
chmod $mode /dev/${device}[0-3]

echo "created device files: /dev/${device}[0-3]"
