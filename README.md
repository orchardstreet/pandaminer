Pandaminer
========================

Mines PDN using a specified node

Installation
------------

### Debian-based systems

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
