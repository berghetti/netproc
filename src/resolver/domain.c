
#include <stdio.h>  // provisorio
#include <stdlib.h>
#include <arpa/inet.h>   // inet_ntop
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>

#include "domain.h"
#include "sock_util.h"
#include "../m_error.h"

#define MAX_CACHE_ENTRIES 1024

#define RESOLVED 1
#define RESOLVING 2

// circular index
#define UPDATE_INDEX_CACHE( idx, max ) ( ( idx ) = ( ( idx ) + 1 ) % (max) )

#define UPDATE_TOT_HOSTS_IN_CACHE( tot )                    \
  ( ( tot ) = ( ( tot ) < MAX_CACHE_ENTRIES ) ? ( tot ) + 1 \
                                              : MAX_CACHE_ENTRIES )


static int
check_name_resolved ( struct sockaddr_storage *ss,
                      struct hosts *hosts_cache,
                      const size_t tot_hosts_cache )
{
  for ( size_t i = 0; i < tot_hosts_cache; i++ )
    {
      if ( check_addr_equal ( ss, &hosts_cache[i].ss ) )
        {
          if ( hosts_cache[i].status == RESOLVED )
            return i;

          break;
        }
    }

  return -1;
}

void *
ip2domain_thread ( void *arg )
{
  char host_buff[NI_MAXHOST] = {0};

  struct hosts *host = (struct hosts *) arg;
  struct sockaddr_storage ss;
  memcpy (&ss, &(host->ss), sizeof(ss));

  int ret;

  // depois de copiar os dados do argumento, devolvemos a execução para thread
  // principal para não sofrer com os atrasos de "getnameinfo", e quando
  // chegar a vez dessa thread, terminamos de processar o ip.
  // se não houver esse sincronismo entre o fluxo principal e a thread, ocorre
  // sobreposição do ponteiro de argumento e dados acabam perdidos
  sched_yield ();

  // convert ipv4 and ipv6
  ret = getnameinfo ( ( struct sockaddr * ) &ss,
                      sizeof ( ss ),
                      host_buff,
                      NI_MAXHOST,
                      NULL,
                      0,
                      NI_DGRAM );

  if (!ret)
    strncpy ( host->fqdn, host_buff, sizeof(host->fqdn) );

  host->status = RESOLVED;

  pthread_exit ( NULL );  // close thread
}

int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len )
{
  static struct hosts hosts_cache[MAX_CACHE_ENTRIES] = { 0 };
  static unsigned int tot_hosts_cache = 0;
  static unsigned int index_cache_host = 0;
  static struct hosts *t_host;      // argument to thread

  // static struct thread_arg t_arg;
  pthread_t tid;

  int nr;
  if ( (nr = check_name_resolved ( ss, hosts_cache, tot_hosts_cache ) ) != -1 )
    {
      // cache hit
      strncpy ( buff, hosts_cache[nr].fqdn, buff_len );
      return 1;
    }
  else
    {
      // cache miss

      // if status equal resolving, a thread already working this slot,
      // go to next
      unsigned int count = 0;
      while(hosts_cache[index_cache_host].status == RESOLVING )
        {
          UPDATE_INDEX_CACHE(index_cache_host, tot_hosts_cache);

          // return and not create new thread
          if (count++ >= tot_hosts_cache)
            return sockaddr_ntop ( ss, buff, buff_len );
        }

      memcpy ( &hosts_cache[index_cache_host].ss,
               ss,
               sizeof ( struct sockaddr_storage ) );

      hosts_cache[index_cache_host].status = RESOLVING;

      // transform binary to text
      sockaddr_ntop ( ss, buff, buff_len );

      t_host = &hosts_cache[index_cache_host];

      if ( pthread_create ( &tid, NULL, ip2domain_thread, ( void * ) t_host ) )
        fatal_error ( "falha criar thread: %s", strerror ( errno ) );

      // wait thread copy data of argument
      if ( pthread_join ( tid, NULL ) )
        fatal_error ( "error thread join" );


      UPDATE_TOT_HOSTS_IN_CACHE ( tot_hosts_cache );
      UPDATE_INDEX_CACHE( index_cache_host, tot_hosts_cache );

      //
      // fprintf ( stderr, "tot_cache - %d\n", tot_hosts_cache );
      // fprintf ( stderr, "idx_cache - %d\n", index_cache_host );

      return 0;
    }
}
