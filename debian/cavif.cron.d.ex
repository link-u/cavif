#
# Regular cron jobs for the cavif package
#
0 4	* * *	root	[ -x /usr/bin/cavif_maintenance ] && /usr/bin/cavif_maintenance
