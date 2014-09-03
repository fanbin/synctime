synctime
========

As its name suggests, synctime is used to sync linux hardware time between two computers. This is generally useful if one machine is isolated from internet, which is stypical in financial trading scenarios.


sync-client: program located on isolated linux box.
sync-server: program located on already-synced box, will try to connect sync-client to sync its time.
