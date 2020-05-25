
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>     // strlen
#include <term.h>       // variable columns

#include "process.h"
#include "terminal.h"  // clear_cmd
#include "translate.h"

// defined in main.c
extern bool view_conections;

// tamanho representação textual porta da camada de transporte
#define PORTLEN 6  // 65535 + '\0'

// "ddd.ddd.ddd.ddd:ppppp <-> ddd.ddd.ddd.ddd:ppppp"
#define LEN_TUPLE ( ( INET_ADDRSTRLEN + PORTLEN ) * 2 ) + 7

// espaçamento entre as colunas
#define PID -5
#define PPS -8
#define RATE -14
#define PROGRAM 54  // tamanho fixo de caracteres até a coluna program

// maximo de caracteres que sera exibido no nome de um processo
#define LEN_NAME_PROGRAM 44

static void
print_conections ( const process_t *const process );

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  // limpa a tela e o scrollback
  clear_cmd ();

  printf ( "%*s %*s %*s %*s %*s %.*s\n",
           PID,
           "PID",
           PPS,
           "PPS TX",
           PPS,
           "PPS RX",
           RATE,
           "RATE UP",
           RATE,
           "RATE DOWN",
           columns - PROGRAM,
           "PROGRAM" );

  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.avg_Bps_rx ||
           processes[i].net_stat.avg_Bps_tx ||
           processes[i].net_stat.avg_pps_rx ||
           processes[i].net_stat.avg_pps_tx )
        {
          printf ( "%*d %*ld %*ld %*s %*s %.*s\n",
                   PID,
                   processes[i].pid,
                   PPS,
                   processes[i].net_stat.avg_pps_tx,
                   PPS,
                   processes[i].net_stat.avg_pps_rx,
                   RATE,
                   processes[i].net_stat.tx_rate,
                   RATE,
                   processes[i].net_stat.rx_rate,
                   columns - PROGRAM,
                   processes[i].name );

          if ( view_conections )
            print_conections ( &processes[i] );
        }
    }
}

static void
print_conections ( const process_t *const process )
{
  // tuple ip:port <-> ip:port
  char *tuple;

  for ( size_t i = 0; i < process->total_conections; i++ )
    {
      // exibi somente as conexões que estão ativas, com pacotes trafegando
      // na rede
      if ( process->conection[i].net_stat.avg_Bps_tx ||
           process->conection[i].net_stat.avg_Bps_rx ||
           process->conection[i].net_stat.avg_pps_tx ||
           process->conection[i].net_stat.avg_pps_rx )
        {
          // faz a tradução de ip:porta para nome:serviço
          tuple = translate ( &process->conection[i] );

          printf ( "%*s %*ld %*ld %*s %*s %s %.*s\n",
                   PID,
                   "",
                   PPS,
                   process->conection[i].net_stat.avg_pps_tx,
                   PPS,
                   process->conection[i].net_stat.avg_pps_rx,
                   RATE,
                   process->conection[i].net_stat.tx_rate,
                   RATE,
                   process->conection[i].net_stat.rx_rate,
                   " |_",
                   columns - PROGRAM - 4,  // 4 = strlen (" |_")
                   tuple );
        }
    }
  putchar ( '\n' );
}
