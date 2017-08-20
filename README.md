First Time Setup
=================

Make sure that the following packages are installed (the command might differ
depending upon your linux distro):

```
sudo apt-get install build-essential automake libssl-dev
```
Generating the configure script (if the configure script is not already present).
1. `autoconf`
If you get `possibly undefined macro: AM_INIT_AUTOMAKE` error, try after running
 `autoreconf --install`
2. `aclocal`
3. `automake` (Sometimes, `autoheader` might need to be run if an error is encountered)

Installation
=============
```
./configure [--prefix=<prefix_path>]
make
make install
```