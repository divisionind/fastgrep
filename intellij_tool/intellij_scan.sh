#!/bin/sh
fastgrep $1 | sed -e 's/^/C:\/Path\/To\/Your\/Sources\/craftbukkit-1.15.1-src\//'
