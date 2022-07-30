
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <net/if.h>
#include <netinet/in.h>   // IPPROTO_TCP, IPPROTO_UDP
#include <netinet/tcp.h>  // TCP_ESTABLISHED, TCP_TIME_WAIT...

#include "config.h"
#include "packet.h"
#include "rate.h"
#include "processes.h"

// static bool
// conection_match_packet ( connection_t *conection, const struct packet *pkt )
// {
//   if ( conection->protocol != pkt->protocol )
//     return false;
//
//   // TCP must correspond exactly with the packet, if not, skip
//   if ( pkt->protocol == IPPROTO_TCP &&
//        ( conection->local_port != pkt->local_port ||
//          conection->remote_port != pkt->remote_port ||
//          conection->remote_address != pkt->remote_address ) )
//     {
//       return false;
//     }
//
//   if ( pkt->protocol == IPPROTO_UDP )
//     {
//       // the local port is the only parameter that the kernel exports
//       // in certain UDP situations (like a torrent),
//       // if that is different, skip
//       if ( conection->local_port != pkt->local_port )
//         return false;
//
//       // UDP connections with state TCP_ESTABLISHED must
//       // correspond exactly with the packet, if not, skip
//       else if ( conection->state == TCP_ESTABLISHED &&
//                 ( conection->remote_port != pkt->remote_port ||
//                   conection->remote_address != pkt->remote_address ) )
//         return false;
//
//       // udp connections have no state, however the kernel tries
//       // to correlate udp packets into connections, when it is able
//       // to assign the virtual state TCP_ESTABLISHED, when it is
//       // unable to assign the state TCP_CLOSE, and it ends up not
//       // exporting some data in /proc/net/udp, but we can "retrieve"
//       // that information from the captured packet
//       if ( conection->state == TCP_CLOSE )
//         {
//           conection->local_address = pkt->local_address;
//           conection->remote_address = pkt->remote_address;
//           conection->remote_port = pkt->remote_port;
//         }
//     }
//
//   return true;
// }

// static bool
// check_udp( struct tuple *tp1, struct tuple *tp2 )
// {
//   if ( ( tp1->l4.remote_port != tp2->l4.remote_port ||
//          tp1->l3.remote.ip != tp2->l3.remote.ip ) )
//     return false;
//
// }

bool
statistics_add ( const struct packet *pkt, bool view_conections )
{
  connection_t *conn;

  conn = connection_get_by_tuple ( ( struct tuple * ) &pkt->tuple );

  if ( conn )
    {
      process_t *proc = conn->proc;

      /* connection was added but at the time of associating to a process
         the process had already been closed (probably) */
      if ( !proc )
        return false;

      conn->if_index = pkt->if_index;

      switch ( pkt->direction )
        {
          case PKT_DOWN:
            rate_add_rx ( &proc->net_stat, pkt->lenght );

            if ( view_conections )
              rate_add_rx ( &conn->net_stat, pkt->lenght );

            break;
          case PKT_UPL:
            rate_add_tx ( &proc->net_stat, pkt->lenght );

            if ( view_conections )
              rate_add_tx ( &conn->net_stat, pkt->lenght );
        }

      return true;
    }

  return false;
}
