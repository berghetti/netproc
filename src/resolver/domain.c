
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "domain.h"
#include "sock_util.h"
#include "../m_error.h"

// (2048 * sizeof(struct host)) == 2.26 MiB cache of domain
#define MAX_CACHE_ENTRIES 2048

#define RESOLVED 1
#define RESOLVING 2

// circular index
#define UPDATE_INDEX_CACHE( idx ) \
  ( ( idx ) = ( ( idx ) + 1 ) % MAX_CACHE_ENTRIES )

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
            return i;  // cache hit

          // already in cache, but not resolved
          return -2;
        }
    }

  // not found in cache
  return -1;
}

void *
ip2domain_thread ( void *arg )
{
  char host_buff[NI_MAXHOST] = { 0 };

  struct hosts *host = ( struct hosts * ) arg;
  int ret;

  // convert ipv4 and ipv6
  ret = getnameinfo ( ( struct sockaddr * ) &host->ss,
                      sizeof ( host->ss ),
                      host_buff,
                      NI_MAXHOST,
                      NULL,
                      0,
                      NI_DGRAM );

  if ( !ret )
    strncpy ( host->fqdn, host_buff, sizeof ( host->fqdn ) );

  host->status = RESOLVED;

  pthread_detach ( pthread_self () );  // free resources to system
  pthread_exit ( NULL );               // close thread
}

int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len )
{
  static struct hosts hosts_cache[MAX_CACHE_ENTRIES] = { 0 };
  static unsigned int tot_hosts_cache = 0;
  static unsigned int index_cache_host = 0;

  // static struct thread_arg t_arg;
  pthread_t tid;

  int nr = check_name_resolved ( ss, hosts_cache, tot_hosts_cache );
  if ( nr >= 0 )
    {
      // cache hit

      strncpy ( buff, hosts_cache[nr].fqdn, buff_len );
      return 1;
    }
  else if ( nr == -2 )
    {
      // already resolving
      sockaddr_ntop ( ss, buff, buff_len );
      return 0;
    }
  else
    {
      // cache miss

      // if status equal RESOLVING, a thread already working this slot,
      // go to next
      unsigned int count = 0;
      while ( hosts_cache[index_cache_host].status == RESOLVING )
        {
          UPDATE_INDEX_CACHE ( index_cache_host );

          // no buffer space currently available, return ip in format of text.
          // increase the buffer size if you want ;)
          if ( count++ == MAX_CACHE_ENTRIES - 1 )
            {
              sockaddr_ntop ( ss, buff, buff_len );
              return 0;
            }
        }

      memcpy ( &hosts_cache[index_cache_host].ss, ss, sizeof ( *ss ) );

      hosts_cache[index_cache_host].status = RESOLVING;

      // transform binary to text
      sockaddr_ntop ( ss, buff, buff_len );

      // passes buffer space for thread to work
      if ( pthread_create ( &tid,
                            NULL,
                            ip2domain_thread,
                            ( void * ) &hosts_cache[index_cache_host] ) )
        fatal_error ( "pthread_create: \"%s\"", strerror ( errno ) );

      UPDATE_TOT_HOSTS_IN_CACHE ( tot_hosts_cache );
      UPDATE_INDEX_CACHE ( index_cache_host );
    }

  return 0;
}
