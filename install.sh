#!/bin/sh
echo "installing fastgrep..."

# install depends
sudo apt update
sudo apt install -y cmake gcc

# build the project using cmake
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make

# mv fastgrep bin for use in cmd line
echo "copying program to /bin/fastgrep..."
strip fastgrep
sudo mv fastgrep /bin/fastgrep
sudo chmod +x /bin/fastgrep