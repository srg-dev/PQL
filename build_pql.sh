
#!/bin/bash

# Install dependencies first
sudo apt update
sudo apt install -y libgtk2.0-dev libglib2.0-dev libtirpc-dev pkg-config build-essential

# Set up the build environment with all necessary flags
export CFLAGS="$(pkg-config --cflags gtk+-2.0 glib-2.0) -I/usr/include/tirpc"
export LDFLAGS="$(pkg-config --libs gtk+-2.0 glib-2.0) -ltirpc"

# Run the original makePQL script
./makePQL
