fastgrep - a faster "grep -r"
--------
![](fastgrep_intro.gif)

A utility similar to `grep -r ...` but focused on raw speed and utilization of system resources. This tool
is useful for quickly scanning large directories for files containing a specific string.

The intro gif is fastgrep scanning the entire, un-indexed, `1.15.1 craftbukkit` sources on a Kali Linux install
running on an 4-core laptop CPU (the `i7-8550U @ 1.8GHz`) for the string "createInventory". That is around 1 million
lines of code searched in under 100ms. The same scan would take grep several minutes.

```
$ cd ~/craftbukkit-1.15.1-src
$ cloc .
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Java                          8099         200332           4926         959246
-------------------------------------------------------------------------------
SUM:                          8099         200332           4926         959246
-------------------------------------------------------------------------------
```

This functionality can be very useful for developing or modding large, documented, codebases 
(e.g. open-source or reverse engineered games, operating systems, etc.).

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

_NOTE: The install script assumes apt as the package manager. If you are running a different package manager, install
the requirements and remove that section of the install script._

##### Windows
1. Download the latest windows installer from the [releases page](https://github.com/divisionind/fastgrep/releases)
2. Run the installer, make sure you select `Add fastgrep to the system PATH for all users`, complete installation
