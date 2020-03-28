
#Cliente TCP
import socket
# Endereco IP do Servidor
SERVER = '172.217.172.142'
# Porta que o Servidor esta escutando
PORT = 80
dest = (SERVER, PORT)


#while 1:

#print ('Para sair use CTRL+X\n')
#msg = input()
while True:
    tcp = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    tcp.connect(dest)
    #msg = ('GET /')
    #tcp.send('GET /')
    #tcp.send(msg.encode())
    #msg = input()
    #resp = tcp.recv(4096)
    #print()
    #tcp.close()
