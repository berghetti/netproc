
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <sys/ioctl.h>

int main(int argc, char **argv){
  // char interface = argv[1];

  char buff[1024];
  struct ifconf ifc;
  ifc.ifc_len = 1024;
  ifc.ifc_buf = buff;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
    puts("eroo");

  struct ifreq *firstInterface = ifc.ifc_req;
  const struct ifreq const *endInterface = firstInterface + ifc.ifc_len / sizeof(struct ifreq);

  if(ifc.ifc_len == 1024)
    puts("msm tamanho");

  for(struct ifreq *interface = firstInterface; interface != endInterface; interface++){
    // struct rtnl_link_stats *statistics = (struct rtnl_link_stats *) interface->ifr_data;
    printf("name: %s\n", interface->ifr_name);
    printf("data: %d\n", interface->ifr_data);

  }
  // 
  // printf("len: %ld\n", ifc.ifc_len / sizeof(ifc.ifc_req));
  // printf("name: %s\n", firstInterface->ifr_name);
  // printf("MTU: %d\n", ifc.ifc_req->ifr_mtu);

  return 0;

}
