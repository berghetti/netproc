#ifndef CYCLE_COUNTING_INCLUDED__
#define CYCLE_COUNTING_INCLUDED__

#ifndef __GNUC__
#error Works only on GCC
#endif

#pragma GCC diagnostic ignored "-Wignored-qualifiers"

// https://bitismyth.wordpress.com/2019/09/24/contador-do-homem-pobre-agora-com-suporta-ao-arm/

/* ==========================================
    Quick & Dirty cycle counting...

    As funções usadas para contar a quantidade de ciclos
    de clock gastos num bloco de código.

    Exemplo de uso:

      counter_T cnt;

      cnt = BEGIN_TSC();
      f();
      cnt = END_TSC(cnt);

    Defina SYNC_MEM se quiser uma serialização mais completa (mfence).
   ========================================== */
#include <stdint.h>

// É necessário usar esse "tipo" para os contadores
// porque o compilador vai tentar otimizar qualquer variável
// não volátil.
typedef volatile uint64_t counter_T;

#if defined( __x86_64__ )
static inline counter_T
BEGIN_TSC ( void )
{
  uint32_t a, d;

  __asm__ __volatile__(
#ifdef SYNC_MEM
          "mfence\n\t"
#endif
          // "xorl %%eax,%%eax\n\t"
          "cpuid\n\t"
          "rdtsc"
          : "=a"( a ), "=d"( d )
          : "a"( 0 )
          : "rbx" );

  return a | ( ( uint64_t ) d << 32 );
}

static inline counter_T
END_TSC ( const counter_T cptr )
{
  uint32_t a, d;

  __asm__ __volatile__( "rdtsc" : "=a"( a ), "=d"( d ) );

  return ( a | ( ( uint64_t ) d << 32 ) ) - cptr;
}

#elif defined( __i386__ )
static inline counter_T
BEGIN_TSC ( void )
{
  uint32_t a, d;

  __asm__ __volatile__(
// Só usa mfence se o processador suportar SSE2.
#ifdef __SSE2__
#ifdef SYNC_MEM
          "mfence\n\t"
#endif
#endif
          "xorl %%eax,%%eax\n\t"
          "cpuid\n\t"
          "rdtsc"
          : "=a"( a ), "=d"( d )::"%ebx", "%ecx" );

  return a | ( ( uint64_t ) d << 32 );
}

static inline void
END_TSC ( counter_T *cptr )
{
  uint32_t a, d;

  __asm__ __volatile__( "rdtscp" : "=a"( a ), "=d"( d )::"%ecx" );

  *cptr = ( a | ( ( uint64_t ) d << 32 ) ) - *cptr;
}
#elif __arm__
#if __ARM_32BIT_STATE == 1
// This works only on ARMv8
#if __ARM_ARCH__ > 7
static inline counter_T
BEGIN_TSC ( void )
{
  unsigned int r0, r1;

  __asm__ __volatile__(
#ifdef SYNC_MEM
          "dmb\n\t"
#endif
          "mrrc p15,1,%0,%1,c14"  // FIXME: Must check this.
                                  //        Get the virtual counter.
          : "=r"( r0 ), "=r"( r1 ) );

  return ( ( uint64_t ) r1 << 32 ) | r0;
}

static inline void
END_TSC ( counter_T *cptr )
{
  unsigned int r0, r1;

  __asm__ __volatile__(
          "mrrc p15,1,%0,%1,c14"  // FIXME: Must check this.
                                  //        get the virtual counter.
          : "=r"( r0 ), "=r"( r1 ) );

  *cptr = ( ( ( uint64_t ) r1 << 32 ) | r0 ) - *cptr;
}
#else
#error ARMv8 or superior only.
#endif
#else  // otherwise we are in aarch64 mode.
static inline counter_T
BEGIN_TSC ( void )
{
  uint64_t count;

  __asm__ __volatile__(
#ifdef SYNC_MEM
          "dmb\n\t"
#endif
          "mrs %0,cntvct_el0"
          : "=r"( count ) );

  return count;
}

static inline void
END_TSC ( counter_T *cptr )
{
  uint64_t count;

  __asm__ __volatile__( "mrs %0,cntvct_el0" : "=r"( count ) );

  *cptr = count - *cptr;
}
#endif
#else
#error i386, x86-64 and ARM only.
#endif

#endif
