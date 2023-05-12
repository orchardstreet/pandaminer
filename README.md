Pandaminer
========================

Mines PDN using a specified node

Installation
------------

### Debian-based systems
This program doesn't rely on any libraries that don't already come with most modern, Unix-like systems.  So there is no need to download anything new except maybe a C compiler.  The commands below are just a dummy-proof way to download and compile with cmake and the GCC C compiler.  You can instead compile manually with a 'gcc' command on BASH or through your own Makefile, and skip this section.


```bash
sudo apt install build-essential unzip wget cmake -y

wget https://github.com/orchardstreet/pandaminer/archive/refs/heads/master.zip

unzip master.zip

cd pandanite-master

cmake .

make
```

Usage
-----------

### Run interactively

```bash
./pandaminer
```

### Run with command-line options

```bash
./pandaminer --ip <ip_address_of_node> --port <port_number_of_node>
```
