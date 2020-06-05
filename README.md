fastgrep - a faster "grep -r"
--------
![](fastgrep_intro.gif)

A utility similar to `grep -r ...` but focused on raw speed and utilization of system resources. This tool
is useful for quickly scanning large directories for files containing a specific string.

This is mainly for fun as I already have a similar tool written in Java. I am interested to see how fast
I can get this compared to that one. Right now (and ignoring jvm startup), they are around the same (maybe 
this one is a little faster). I want to see if I can make it noticeable.

The intro gif is fastgrep scanning the entire, un-indexed, 1.15.2 craftbukkit sources on a Debian WSL install
running an i7-8700k for the string "createInventory". This functionality can be very useful for developing mods 
on large codebases.

### IntelliJ IDEA Tool Integration
For more info see [this](intellij_tool/README.md).

### Building
##### Requirements
- Building: cmake, gcc
- Running:  argp, pthread

##### Installing from binary release - Linux
1. Run 
`sudo curl -fsSL https://github.com/divisionind/fastgrep/releases/download/LATEST_VERSION/fastgrep -o /usr/bin/fastgrep && sudo chmod +x /usr/bin/fastgrep`
2. where `LATEST_VERSION` is the latest version of fastgrep as seen from the releases tab (e.g. `v1.3`), NOT `fastgrep v1.3`

##### Installing from source - Linux
1. Clone repo `git clone https://github.com/divisionind/fastgrep.git`
2. Enter directory `cd fastgrep`
3. Build locally and install `sudo ./fastgrep`

##### Windows
There is a portable cygwin build for windows. However, I have not made an install script at this time.
If you really don't want to use WSL:
1. Download the latest zip from the releases page
2. Extract it somewhere
3. *optional:* add that directory to your path

_NOTE: The install script assumes apt as the package manager. If you are running a different package manager, install
the requirements and remove that section of the install script._
