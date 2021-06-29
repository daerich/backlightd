# backlightd
BacklightDaemon - to replace systemd-backlightd
## Installation 
```
./configure.sh
```

```
make 
```

```
make install
```

```
BACKLIGHTCTL(1)             General Commands Manual            BACKLIGHTCTL(1)


NAME
       backlightctl - control backlightd daemon

SYNOPSIS
       backlightctl [OPTION] [SCALE]

DESCRIPTION
       backlightctl pushes new configuration options to backlightd and signals
       it to reload.

OPTIONS
       p      Print current brightness scale in percent

       s      Set current brightness to SCALE ,where scale is an integer
              between 10 and 100 (naturally).

FILES
              /tmp/backlightd.pid - pid file for backlightctl

              /etc/backlightd_d.conf - backlightd's configuration file

NOTES
       Only the first letter of the command is checked.

       The rest is ignored(commands resemble "BSD-style")

AUTHOR
       Written by daerich and tracked on github.com

EXAMPLES
       Set brightness to 40%:

              backlightctl s 40

       Print brightness:

              backlightctl p

       Print help:

              backlightctl

       or

              backlightctl sp

       or

              backlightctl any unknown words



GNU/LINUX                         2021-06-22                   BACKLIGHTCTL(1)
```
