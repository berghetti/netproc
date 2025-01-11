# Netproc

tool to monitor network traffic based on processes


![netproc print](img/print.png)


#### Dependences
    [Debian based]
    $ sudo apt install gcc make libncurses-dev

    [Red Hat Based]
    $ sudo yum install gcc make ncurses-devel

#### Install
    $ git clone https://github.com/berghetti/netproc.git
    $ cd netproc
    $ make
    $ sudo make install

#### Install single command
    $ git clone https://github.com/berghetti/netproc.git; cd netproc; make; sudo make install

#### Options / Usage
    Usage: netproc [options]

    Options:
     -B, --bytes             view in bytes, default in bits
     -c                      visualization each active connection of the process
     -f, --file "filename"   save statistics in file, file name is optional,
                             default is 'netproc.log'
     -h, --help              show this message
     -i, --interface iface   specifies an interface, default is all
                             (except interface with network 127.0.0.0/8)
     -n                      numeric host and service, implicit '-c', try '-nh' to no
                             translate only host or '-np' to not translate only service
     -p, --protocol tcp|udp  specifies a protocol, the default is tcp and udp
     -v, --verbose           verbose mode, alse show process without traffic
     --si                    show SI format, with powers of 10, default is IEC,
                             with powers of 2
     -V, --version           show version

    when running press:
     arrow keys    scroll
     s             change column-based sort
     q             exit

#### Running without root
    $ sudo setcap "cap_net_admin,cap_net_raw+pe" /usr/local/sbin/netproc

#### Uninstall
    $ sudo make uninstall

#### Debug
    $ make clean
    $ DEBUG=1 make
    $ sudo ./bin/netproc 2> log.txt

    $ sudo DESTDIR=/tmp make install

### Netproc in Linux Distributions

[![Packaging status](https://repology.org/badge/vertical-allrepos/netproc.svg)](https://repology.org/project/netproc/versions)
