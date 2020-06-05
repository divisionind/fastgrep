#!/bin/sh

# get release info from github
echo "checking for latest version..."
LATEST_VERSION=$(curl -s "https://api.github.com/repos/divisionind/fastgrep/releases/latest" |
  grep '"tag_name":' |
  sed -E 's/.*"([^"]+)".*/\1/')

# remove previous version which used to be stored here
if [ -f /bin/fastgrep ]; then
  sudo rm /bin/fastgrep
fi

# download and install latest binary release
echo "installing fastgrep $LATEST_VERSION..."
sudo curl -fsSL https://github.com/divisionind/fastgrep/releases/download/"$LATEST_VERSION"/fastgrep -o /usr/bin/fastgrep
sudo chmod +x /usr/bin/fastgrep

echo "done."
