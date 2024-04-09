# Spicial
A SPICE-like circuit simulator, written in C++.

## Quick Start

### Install dependencies

仅在 WSL Ubuntu 22.04 环境经过测试，其他环境的适用性未知。

```bash
sudo apt update

# install Qt5
sudo apt install –y qtbase5-dev qt5-qmake

# install flex&bison
sudo apt install libfl-dev
sudo apt install libbison-dev

# install armadillo
sudo apt install libarmadillo-dev
```

### Clone this repo

```bash
git clone https://github.com/Imiloin/Spicial.git
cd Spicial
```

### Build executable

```bash
cd src/parser && make && cd -
qmake .
make
```

Or simply run

```bash
bash compile.sh
```

### Launch Spicial

```bash
bin/spicial
```

