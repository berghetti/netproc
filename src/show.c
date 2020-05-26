
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

// #include <stdio.h>
#include <stdbool.h>
#include <string.h>  // strlen
// #include <term.h>    // variable columns
#include <ncurses.h>

#include "process.h"
#include "conection.h"
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
print_conections ( const process_t *process );

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  // limpa a tela e o scrollback
  clear_cmd ();

  printw ( "%*s %*s %*s %*s %*s %.*s\n",
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
           COLS - PROGRAM,
           "PROGRAM" );

  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.avg_Bps_rx ||
           processes[i].net_stat.avg_Bps_tx ||
           processes[i].net_stat.avg_pps_rx ||
           processes[i].net_stat.avg_pps_tx )
        {
          printw ( "%*d %*ld %*ld %*s %*s %.*s\n",
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
                   COLS - PROGRAM - 1,
                   processes[i].name );

          if ( view_conections )
            print_conections ( &processes[i] );
        }
    }
  refresh ();
}

static void
print_conections ( const process_t *process )
{
  // tuple ip:port <-> ip:port
  char *tuple;

  size_t index_act[process->total_conections];
  size_t tot_con_act;
  // pega o total de conexões no processo que estão ativas, e seus respectivos
  // indices
  tot_con_act = get_con_active_in_process (index_act,
                          process->conection, process->total_conections );

  for ( size_t i = 0; i < tot_con_act; i++ )
    {
      // faz a tradução de ip:porta para nome:serviço
      tuple = translate ( &process->conection[index_act[i]] );

      printw ( "%*s %*ld %*ld %*s %*s  ",
               PID,
               "",
               PPS,
               process->conection[index_act[i]].net_stat.avg_pps_tx,
               PPS,
               process->conection[index_act[i]].net_stat.avg_pps_rx,
               RATE,
               process->conection[index_act[i]].net_stat.tx_rate,
               RATE,
               process->conection[index_act[i]].net_stat.rx_rate );
      attron(A_BOLD | COLOR_PAIR(1));
      if ( i + 1 != tot_con_act )
        {
          addch ( ACS_LTEE );   // ├
          addch ( ACS_HLINE );  // ─
        }
      else
        {                          // ultima conexão
          addch ( ACS_LLCORNER );  // └
          addch ( ACS_HLINE );     // ─
        }
      attroff(A_BOLD | COLOR_PAIR(1));

      printw ( " %s\n", tuple );
    }
  addch ( '\n' );
}
