#ifndef SERVICE_H
#define SERVICE_H

int
port2serv ( unsigned short int port,
            char *proto,
            char *buf,
            const size_t buf_len );

#endif  // SERVICE_H
