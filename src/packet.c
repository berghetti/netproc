
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

#include <arpa/inet.h>        // htons
#include <linux/if_packet.h>  // struct tpacket3_hdr
#include <linux/if_packet.h>  // struct sockaddr_ll
#include <linux/ip.h>         // struct iphdr
#include <time.h>             // time

#include "packet.h"

// masks header IP
#define IP_DF 0x4000       // dont fragment
#define IP_MF 0x2000       // more fragments
#define IP_OFFMASK 0x1FFF  // offset do fragmento

/* máximo de pacotes IP que podem estar fragmentados simultaneamente,
 os pacotes que chegarem alem desse limite não serão calculados
 e um log sera enviado na saida de erro padrão.
 reference cisco default value: 16 */
#define MAX_REASSEMBLIES 32

/* maximo de fragmentos que um pacote IP pode ter,
acima disso um erro sera emitido, e os demais fragmentos desse pacote
não serão computados e um log sera enviado na saida de erro padrão
reference cisco default value: 32 */
// #define MAX_FRAGMENTS 64

/* tempo de vida maximo de um pacote fragmentado em segundos,
caso não chegue todos os fragmentos do pacote nesse periodo,
o fragmento é descartado, cada pacote possui um contador unico
reference cisco default value: 3 seconds*/
#define LIFETIME_FRAG 3

// codigo de erro para buffer de fragmentos cheio ou fragmento não encontrado
#define ERR_FRAGMENT -2

// decrementa o total de pacotes fragmentados
#define DEC_REASSEMBLE( var ) ( ( var ) -= ( ( var ) != 0 ) )

/* Aproveitamos do fato dos cabeçalhos TCP e UDP
receberem as portas de origem e destino na mesma ordem,
e como atributos iniciais, assim podemos utilizar esse estrutura
simplificada para extrair as portas tanto de pacotes
TCP quanto UDP, lembrando que não utilizaremos outros campos
dos respectivos cabeçalhos. */
struct layer_4
{
  uint16_t source;
  uint16_t dest;
};

/* utilzado para identificar a camada de transporte (TCP, UDP)
dos fragmentos de um pacote e tambem ter controle de tempo de vida
do pacote e numero de fragmentos do pacote */
struct pkt_ip_fragment
{
  time_t ttl;  // lifetime of packet
  uint32_t saddr;
  uint32_t daddr;
  uint16_t id;           // IP header ID value
  uint16_t source_port;  // transport header source port
  uint16_t dest_port;    // transport header dest port
};

// armazena os dados da camada de transporte dos pacotes fragmentados
static struct pkt_ip_fragment pkt_ip_frag[MAX_REASSEMBLIES] = { 0 };

// contador de pacotes IP que estão fragmentados
static uint8_t count_reassemblies = 0;

/* store fragment if has slot available
return index in array of fragments or -1 if not slot free */
static inline int
store_fragment ( const struct iphdr *l3, const struct layer_4 *l4 )
{
  if ( count_reassemblies == MAX_REASSEMBLIES )
    return -1;

  count_reassemblies++;

  // busca posição livre para adicionar dados do pacote fragmentado
  for ( int i = 0; i < MAX_REASSEMBLIES; i++ )
    {
      if ( pkt_ip_frag[i].ttl == 0 )  // posição livre no array
        {
          pkt_ip_frag[i].saddr = l3->saddr;
          pkt_ip_frag[i].daddr = l3->daddr;
          pkt_ip_frag[i].id = l3->id;
          pkt_ip_frag[i].source_port = l4->source;
          pkt_ip_frag[i].dest_port = l4->dest;
          pkt_ip_frag[i].ttl = time ( NULL );

          return i;
        }
    }

  return -1;
}

// return index in array of fragments or -1 if not found
static inline int
search_fragment ( const struct iphdr *l3 )
{
  // percorre todo array de pacotes fragmentados...
  for ( int i = 0; i < MAX_REASSEMBLIES; i++ )
    {
      if ( !pkt_ip_frag[i].ttl || l3->id != pkt_ip_frag[i].id ||
           l3->saddr != pkt_ip_frag[i].saddr ||
           l3->daddr != pkt_ip_frag[i].daddr )
        continue;

      // se for o  ultimo fragmento
      // libera a posição do array de fragmentos
      if ( ( ntohs ( l3->frag_off ) & IP_MF ) == 0 )
        {
          pkt_ip_frag[i].ttl = 0;
          DEC_REASSEMBLE ( count_reassemblies );
        }

      return i;
    }

  return -1;
}

/* verifica se é um fragmento,
retorna
  o id do array de fragmentos se for um fragmento
  -1 se não for um fragmento
  ERR_FRAGMENT se for um fragmento mas não esta no buffer */
static int
get_fragment ( const struct iphdr *l3, const struct layer_4 *l4 )
{
  uint16_t frag_off = ntohs ( l3->frag_off );

  // bit não fragmente ligado, logo não pode ser um fragmento
  if ( frag_off & IP_DF )
    return -1;

  int ret = -1;
  // bit MF ligado e offset igual a 0,
  // it's first fragment
  if ( ( frag_off & IP_MF ) && ( frag_off & IP_OFFMASK ) == 0 )
    {
      ret = store_fragment ( l3, l4 );
      if ( ret == -1 )
        return ERR_FRAGMENT;
    }
  else if ( frag_off & IP_OFFMASK )
    {
      ret = search_fragment ( l3 );
      if ( ret == -1 )
        return ERR_FRAGMENT;
    }

  // it's not first fragment
  return ret;
}

// remove fragments with expired lifetime
static void
clear_frag ( void )
{
  time_t now = time ( NULL );

  for ( size_t i = 0; count_reassemblies && i < MAX_REASSEMBLIES; i++ )
    {
      if ( !pkt_ip_frag[i].ttl )
        continue;

      if ( ( now - pkt_ip_frag[i].ttl ) >= LIFETIME_FRAG )
        {
          pkt_ip_frag[i].ttl = 0;
          DEC_REASSEMBLE ( count_reassemblies );
        }
    }
}

static void
insert_data_packet ( struct packet *pkt,
                     const uint32_t if_index,
                     const uint8_t direction,
                     const uint8_t protocol,
                     const uint32_t local_address,
                     const uint32_t remote_address,
                     const uint16_t local_port,
                     const uint16_t remote_port,
                     const uint32_t len )
{
  pkt->if_index = if_index;
  pkt->direction = direction;
  pkt->tuple.l3.local.ip = local_address;
  pkt->tuple.l3.remote.ip = remote_address;
  pkt->tuple.l4.protocol = protocol;
  pkt->tuple.l4.local_port = ntohs ( local_port );
  pkt->tuple.l4.remote_port = ntohs ( remote_port );
  pkt->lenght = len;
}

/* parse ppd in layers 3 and 4, store in 'struct packet',
return 1 on sucess or 0 */
int
parse_packet ( struct packet *pkt, struct tpacket3_hdr *ppd )
{
  struct sockaddr_ll *ll;
  // struct ethhdr *l2;
  struct iphdr *l3;
  struct layer_4 *l4;

  ll = ( struct sockaddr_ll * ) ( ( uint8_t * ) ppd + TPACKET3_HDRLEN -
                                  sizeof ( struct sockaddr_ll ) );
  // l2 = ( struct ethhdr * ) ( ( uint8_t * ) ppd + ppd->tp_mac );
  l3 = ( struct iphdr * ) ( ( uint8_t * ) ppd + ppd->tp_net );
  l4 = ( struct layer_4 * ) ( ( uint8_t * ) ppd + ppd->tp_net +
                              ( l3->ihl * 4 ) );

  // ERROR_DEBUG( "ll->sll_pkttype - %d\n", ll->sll_pkttype );
  // ERROR_DEBUG( "ll->sll_hatype - %d\n", ll->sll_hatype );
  // ERROR_DEBUG( "l2->h_proto - %x\n", ntohs(l2->h_proto) );
  // ERROR_DEBUG( "l3->proto - %d\n", l3->protocol  );
  // ERROR_DEBUG( "l4->dest - %x\n", ntohs(l4->dest) );

  int ret = 1;
  int id_frag = get_fragment ( l3, l4 );
  if ( id_frag == ERR_FRAGMENT )
    {
      ret = 0;
      goto END;
    }

  switch ( ll->sll_pkttype )
    {
      case PACKET_OUTGOING:  // upload
        if ( id_frag == -1 )
          {
            // não é um fragmento, assume que isso é maioria dos casos
            insert_data_packet ( pkt,
                                 ll->sll_ifindex,
                                 PKT_UPL,
                                 l3->protocol,
                                 l3->saddr,
                                 l3->daddr,
                                 l4->source,
                                 l4->dest,
                                 ppd->tp_snaplen );
          }
        else
          {
            // é um fragmento, pega dados da camada de transporte
            // no array de pacotes fragmentados
            insert_data_packet ( pkt,
                                 ll->sll_ifindex,
                                 PKT_UPL,
                                 l3->protocol,
                                 l3->saddr,
                                 l3->daddr,
                                 pkt_ip_frag[id_frag].source_port,
                                 pkt_ip_frag[id_frag].dest_port,
                                 ppd->tp_snaplen );
          }
        break;
      case PACKET_HOST:  // download
        if ( id_frag == -1 )
          {
            // não é um fragmento, assumi que isso é maioria dos casos
            insert_data_packet ( pkt,
                                 ll->sll_ifindex,
                                 PKT_DOWN,
                                 l3->protocol,
                                 l3->daddr,
                                 l3->saddr,
                                 l4->dest,
                                 l4->source,
                                 ppd->tp_snaplen );
          }
        else
          {
            // é um fragmento, pega dados da camada de transporte
            // no array de pacotes fragmentados
            insert_data_packet ( pkt,
                                 ll->sll_ifindex,
                                 PKT_DOWN,
                                 l3->protocol,
                                 l3->daddr,
                                 l3->saddr,
                                 pkt_ip_frag[id_frag].dest_port,
                                 pkt_ip_frag[id_frag].source_port,
                                 ppd->tp_snaplen );
          }
        break;
      default:
        ret = 0;  // fail parse
    }

END:
  clear_frag ();

  return ret;
}
