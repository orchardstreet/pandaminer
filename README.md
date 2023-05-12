Pandaminer
========================

Mines PDN using a specified node

Installation
------------

### Debian-based systems

```bash
sudo apt install build-essential git cmake -y

git clone https://github.com/orchardstreet/pandaminer

cmake .

make
```

Usage
-----------

### Run interactively

```bash
./miner
```

### Run with command-line options

```bash
./miner --ip <ip_address_of_node> --port <port_number_of_node>
```
