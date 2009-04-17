README for dvbloop - A Virtual DVB Adapter 

 Version of this document: 0.1 

 Maintainer:
 	Christian Praehauser
	<cpraehaus@cpn.dyndns.org>

Compiling
=========
(1) Get the sources for you kernel version (KSRC).

(2) Configure your kernel
E.g. under Debian, the current kernel configuration is stored in the directory /boot.
So, if you have a 2.6.18 Linux kernel running, type the following:
	cp /boot/config-2.6.18 <KSRC>/.config
	
(3) Change into the kernel source directory and prepare it for building modules:
	cd <KSRC>
	make modules_prepare
	
(4) Change into the directory of dvbloop (DLOOPSRC) and build the module.
	cd <DLOOPSRC>
	make KDIR=<KSRC>
	
(5) You should now have a file named dvbloop.ko in the <DLOOPSRC> directory
You can now try to load the dvbloop module:
	modprobe dvb_core
	insmod dvbloop.ko
	
(6) You can also install the module by using
	make KDIR=<KSRC> install && depmod
Now, it should be possible to load the module with the modprobe tool:
	modprobe dvbloop
