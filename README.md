# Netproc

tool to monitor network traffic based on processes


![Alt Text](img/print.png)


#### [Dependences]
    [Debian based]
    $ sudo apt install libncurses5-dev

#### [Install]
    $ git clone https://github.com/berghetti/netproc.git
    $ cd netproc
    $ make
    $ sudo make install
   
##### Fast install
    $ git clone https://github.com/berghetti/netproc.git; cd netproc; make; sudo make install
    
#### [Options / usage]
    Usage: netproc [options]

    Options:
    -u            rastreia trafego udp, padrão é tcp
    -i <iface>    seleciona a interface, padrão é todas
    -B            visualização em bytes, padrão em bits
    -si           visualização com formato SI, com potências de 1000
                  padrão é IEC, com potências de 1024
    -h            exibe essa mensagem
    -v            exibe a versão
    
#### [uninstall]
    $ sudo make uninstall
