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

##### Installing from binary release - Linux (Debian/Ubuntu)
```shell script
curl -fsSL https://raw.githubusercontent.com/divisionind/fastgrep/master/get_fastgrep.sh -o get_fastgrep.sh
sudo sh get_fastgrep.sh
```

##### Installing from source - Linux
1. Clone repo `git clone --recursive https://github.com/divisionind/fastgrep.git`
2. Enter directory `cd fastgrep`
3. Build locally and install `sudo sh install.sh`

##### Windows
There is a portable cygwin build for windows. However, I have not made an install script at this time.
If you really don't want to use WSL:
1. Download the latest zip from the releases page
2. Extract it somewhere
3. *optional:* add that directory to your path

_NOTE: The install script assumes apt as the package manager. If you are running a different package manager, install
the requirements and remove that section of the install script._
