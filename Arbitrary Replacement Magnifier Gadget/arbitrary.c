#include <cpuid.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <stdio.h>
#include <x86intrin.h>
#include <malloc.h>

//4kB/8B = 512k
#define PAGESIZE 0x80000
#define SETMAX 32
// the mask is set to mod(32)
#define SETMASK (0x1F)
#define CACHELINESIZE 8
#define WAYSIZE 8
#define LOOPSIZE (WAYSIZE - 1)
#define PRE_DIST 22
//64B/8B = 8

// the offset might need to change after reboot or after a while to avoid those sets used during
// the attack being affected by other parts of the code
#define OFFSET 0x180

uint64_t mem[0x1000000];

static uint32_t __attribute__((always_inline)) force_read(const char *p) {
  return (uint32_t)*(const volatile uint32_t*)(p);
}

inline static uint32_t time_access(void *addr)
{
	uint32_t timing;

  asm volatile(
      "mov %1, %%rsi\n"
      "lfence\n"
      "rdtsc\n"
      "lfence\n"
      "mov %%eax, %%edi\n"    /* store low order bits of tsc into edi */
      "movq (%%rsi), %%rsi\n" /* LOAD */
      "lfence\n"
      "rdtscp\n" /* RDTSC */
      "lfence\n"
      "sub %%edi, %%eax\n" /* get timing difference */
      : "=a"(timing)       /*output*/
      : "r"(addr)
      : "rcx", "rdx", "edi", "rsi", "memory");

  return timing;
}

static void traverseEV_before(int setnum, uint64_t *cacheSet[SETMAX][LOOPSIZE], uint64_t *clearSet[SETMAX][WAYSIZE]) {
  register uint64_t trash = 0;
  for (int rep = 0; rep < setnum; rep++) {
    for (int repetition = 0; repetition < 2; repetition++)
      for (int i = 0; i < WAYSIZE; i++) {
        trash = force_read(clearSet[rep][i]+trash);
      }
  }
  for (int rep = 0; rep < setnum; rep++) {
    // It is important to serialize the memory requests, no idea why
    for (int repetition = 0; repetition < 2; repetition++)
      for (int i = 0; i < LOOPSIZE; i++)
        trash = force_read(cacheSet[rep][i]+trash);
  }

  //deliberately create l1 tlb miss
  for (int i = 0; i < 256; i++) {
    force_read(&mem[OFFSET + (i << 9)]);
  }
  
}

static uint64_t inline __attribute__((always_inline)) rdtsc_begin() {
  uint32_t high, low;
  asm volatile(
      "lfence\n"
      "CPUID\n\t"
      "RDTSC\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      : "=r"(high), "=r"(low)
      :
      : "rax", "rbx", "rcx", "rdx");
  return ((uint64_t)high << 32) | low;
}

static uint64_t inline __attribute__((always_inline)) rdtsc_end() {
  uint32_t high, low;
  asm volatile(
      "RDTSCP\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      "CPUID\n\t"
      : "=r" (high), "=r" (low)
      :
      : "rax", "rbx", "rcx", "rdx");
  return ((uint64_t)high << 32) | low;
}

uint64_t __attribute__ ((noinline)) magnify(int round_num, int setnum, int flush_value, int bit_no, uint64_t *cacheSet[SETMAX][LOOPSIZE], uint64_t *clearSet[SETMAX][WAYSIZE])
{
  int mask = 1 << bit_no;
  uint64_t start = 0, end = 0;
  volatile int trash_a = 0, trash_b = 0; // the volatile is necessary for the attack to succeed. No idea why...
  uint32_t trash = 0;
  for (volatile int i = 0; i < 2048; i++);
  traverseEV_before(setnum, cacheSet, clearSet);
  asm volatile("mfence");
  _mm_clflush(cacheSet[flush_value][0]);
  asm volatile("mfence");
  for (int volatile i = 0; i < 100; i++)
    asm volatile("mfence");
  start = rdtsc_begin();
  asm volatile("lfence");
  // register int trash_ncritial = 0;
  for (int round = 0; round < round_num; round++) {
    for (uint8_t rep = 0; rep < setnum - 1; rep += 2) {
      int prefetch_b = (rep + PRE_DIST + 1) & SETMASK;
      int clear_b = (rep + 1) & SETMASK;
      for (int i = 0; i < LOOPSIZE; i++) {
        trash_a = *((u_int32_t *)(cacheSet[rep][i] + trash_a));
        trash_b = *((u_int32_t *)(cacheSet[rep + 1][i] + trash_b));
        // force_read(cacheSet[prefetch_b][i] + temp_b);
        force_read(cacheSet[prefetch_b][i]);
      }
      for (int i = 0; i < WAYSIZE - 4; i++)
        force_read(clearSet[clear_b][i] + trash_a);
    }
  }
  // set trash_b as the critical path
  for (int i = 0; i < 400; i++) {
    trash_b = 1 / (1 + trash_b) - 1;
  }
  asm volatile("lfence");
  end = rdtsc_end();
  return (uint64_t)(end - start + trash_a + trash_b);
}

#define N_TESTS 100
static int total[2][N_TESTS];

static int comparator(const void *p, const void *q)
{
  return *(int *)p > *(int *)q;
}

static void __attribute__((optimize("-O2"), noinline)) test_this_round_number(
  int round_num, int setnum, uint64_t *cacheSet[SETMAX][LOOPSIZE],uint64_t *clearSet[SETMAX][WAYSIZE])
{
  uint64_t dur = 0;
  double overall = N_TESTS;

  for (int j = 0; j < N_TESTS; j++) {
    for (volatile int iter = 0; iter < 13; iter++); // IMPORTANT: add delay (> 13).
    asm volatile("mfence");
    dur = magnify(round_num, setnum, 0, 0, cacheSet, clearSet); // send secret when (i%8==7).                    
    total[0][j] = dur;
    dur = magnify(round_num, setnum, 1, 0, cacheSet, clearSet); // send secret when (i%8==7).                    
    total[1][j] = dur;
  }

  for (int i = 0; i < 2; i++) {
    qsort((void *)total[i], N_TESTS, sizeof(total[i][0]), comparator);
  }
  printf("%d %d %d %d\n",
  total[1][N_TESTS/2] - total[0][N_TESTS/2],
  total[1][N_TESTS/4] - total[0][N_TESTS/4], total[1][(3*N_TESTS)/4] - total[0][(3*N_TESTS)/4], total[1][N_TESTS/2]);
}

uint64_t *getRandomElement(int rep) {
  int pageIndex = rand() % 1000;
  mem[pageIndex*0x1000 + OFFSET + rep*CACHELINESIZE] = 0;// initialize
  return &(mem[pageIndex*0x1000 + OFFSET + rep*CACHELINESIZE]);
}

int main(int argc, char *argv[])
{
  uint64_t *cacheSet[SETMAX][LOOPSIZE];
  uint64_t *clearSet[SETMAX][WAYSIZE];

  // find the EV set
  // first find the CacheSet
  int succeed;
  for (int rep = 0; rep < SETMAX; rep++) {
    for (int i = 0; i < LOOPSIZE; i++) {
      succeed = 0;
      while (succeed == 0) {
        cacheSet[rep][i] = getRandomElement(rep+1);
        succeed = 1;
        for (int j = 0; j < i; j++) {
          if (cacheSet[rep][i] == cacheSet[rep][j]) {
            succeed = 0;
            break;
          }
        }
      }
    }
  }

  // then find the ClearSet. There should be no overlap between CacheSet and ClearSet
  for (int rep = 0; rep < SETMAX; rep++) {
    for (int i = 0; i < WAYSIZE; i++) {
      succeed = 0;
      while (succeed == 0) {
        clearSet[rep][i] = getRandomElement(rep+1);
        succeed = 1;
        for (int j = 0; j < i; j++) {
          if (clearSet[rep][i] == clearSet[rep][j]) {
            succeed = 0;
            break;
          }
        }
        if (succeed == 1) {
          for (int j = 0; j < LOOPSIZE; j++) {
            if (clearSet[rep][i] == cacheSet[rep][j]) {
              succeed = 0;
              break;
            }
          }
        }
      }
    }
  }

  for (int round_num = 20; round_num < 1200; round_num += 30) {;
    printf("%d\t", round_num);
    test_this_round_number(round_num, SETMAX, cacheSet, clearSet);
  }
}
