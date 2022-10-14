#include <cpuid.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <malloc.h>
#include <stdlib.h>

uint64_t mem[0x10000];

static uint64_t inline __attribute__((always_inline)) rdtsc_begin() {
  uint32_t high, low;
  asm volatile (
      "lfence\n"
      "CPUID\n\t"
      "RDTSC\n\t"
      "mov %%edx, %0\n\t"
      "mov %%eax, %1\n\t"
      : "=r" (high), "=r" (low)
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

uint64_t __attribute__ ((noinline)) magnify(int magnify_rounds)
{
  uint64_t start = 0, end = 0;
  uint32_t trash = 0, start1, start2, result1, result2;
  for(int i = 0; i < 2; i++) {
    trash = mem[trash];
  }
  start = rdtsc_begin();

  /* To transmit 0, both start1 and start2 are cache misses so that two paths (pathA & pathB) will
  start at the same time. To transmit 1, start1 is a cache miss while start2 is a hit, enabling pathB
  to start earlier.
  Note that at least one cache miss is necessary to eliminate the effect of fetch/decode/renaming/dispatch stage.
  */
  start1 = mem[trash];          // miss if transmit 0, hit if transmit 1
  start2 = mem[trash + 0x1000]; // make sure this is always a miss
  // start1 with muls, start2 with divs
  asm volatile(
      "mov %4, %%esi\n\t" // set loop number here(input from magnify_rounds)

      // take the input
      "mov %3, %1\n\t" // result2 = start2
      "mov %2, %0\n\t" // result1 = start1.

      //prepare initial state for div and mul operations
      "xor %%ebx, %%ebx\n\t"
      "add $3, %%ebx\n\t" //ebx = 3

      ".MYLOOP%=:\n\t"
      // div path (pathB), start with start2
      "xor %%rax, %%rax\n\t"
      "sar $31, %1\n\t" //reset value to 0
      "mov %1, %%eax\n\t"
      "add $1, %%rax\n\t"
      "cvtsi2sdq %%rax, %%xmm0\n\t"

      // mul path (pathA), start with start1
      //ebx = start1 + 1 = 1
      "sar $31, %2\n\t" //reset value to 0
      "add $1, %2\n\t"

      // racing stage
      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      "divsd %%xmm0, %%xmm0\n\t"

      // add more imull here to increase the execution time of the imull path to that of the divsd path


      "divsd %%xmm0, %%xmm0\n\t"
      "divsd %%xmm0, %%xmm0\n\t"
      "divsd %%xmm0, %%xmm0\n\t"
      "divsd %%xmm0, %%xmm0\n\t"

      // make sure mul path is a little bit longer than div path at this point, so
      // that if no delay added to the mul path, it will issue those divs later, so
      // no contention is added to the div path
      // however, if latency is added, then make sure the latency isn't too large, so
      // that when the mul path arrives this point, the div path should already starts,
      // so that the contention could be detected. This can be achieved by adding enough
      // instructions to this path, so that either the execution time is larger than the
      // latency, or those in-flight instructions stalls the ROB.

      // //create div contention from the mul path
      "xor %%rcx, %%rcx\n\t"
      "mov %2, %%ecx\n\t" //make sure it happens at the end of mul path
      "sar $31, %%ecx\n\t"
      "add $3, %%rcx\n\t" //rcx = 3
      "cvtsi2sdq %%rcx, %%xmm1\n\t"
      "movaps %%xmm1, %%xmm2\n\t"
      "movaps %%xmm2, %%xmm3\n\t"
      "movaps %%xmm3, %%xmm4\n\t"
      "movaps %%xmm4, %%xmm5\n\t"
      "movaps %%xmm5, %%xmm6\n\t"
      "movaps %%xmm6, %%xmm7\n\t"
      "movaps %%xmm7, %%xmm8\n\t"
      "movaps %%xmm8, %%xmm9\n\t"
      "movaps %%xmm9, %%xmm10\n\t"
      "movaps %%xmm10, %%xmm11\n\t"
      "movaps %%xmm11, %%xmm12\n\t"
      // ensure those div almost start at the same time, otherwise contention doesn't exist
      "divsd %%xmm12, %%xmm1\n\t"
      "divsd %%xmm12, %%xmm2\n\t"
      "divsd %%xmm12, %%xmm3\n\t"
      "divsd %%xmm12, %%xmm4\n\t"
      "divsd %%xmm12, %%xmm5\n\t"
      "divsd %%xmm12, %%xmm6\n\t"
      "divsd %%xmm12, %%xmm7\n\t"
      "divsd %%xmm12, %%xmm8\n\t"
      "divsd %%xmm12, %%xmm9\n\t"
      "divsd %%xmm12, %%xmm10\n\t"
      "divsd %%xmm12, %%xmm11\n\t"
      "divsd %%xmm12, %%xmm12\n\t"

      //end result2
      "cvttsd2siq	%%xmm0, %%rax\n\t"
      "mov %%eax, %1\n\t"

      //end result1
      "mov %2, %0\n\t"

      // if there is no delay, make two paths' length to be equal here,
      // so that they can starts at the same time next time
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %2\n\t"

      // buffer stage
      // imull can be replaced by any other sequential arithmetic operations,
      // such as add illustrated in the paper. Note that imull operations here in the buffer stage
      // will not interfere with those in the racing stage
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"
      "imull %%ebx, %1\n\t"
      "imull %%ebx, %2\n\t"

      "subl $1, %%esi\n\t"
      "jne .MYLOOP%=\n\t"

      "sar $31, %1\n\t"
      "sar $31, %0\n\t"

      : "+r"(result1), "+r"(result2)
      : "r"(start1), "r"(start2), "r"(magnify_rounds)
      : "esi", "rax", "eax", "ebx", "rcx", "ecx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12");

  end = rdtsc_end();
  return (uint64_t)(end - start + result1 + result2);
}


#define N_TESTS 150
static int total[2][N_TESTS];

static int comparator(const void *p, const void *q)
{
  return *(int *)p > *(int *)q;
}

static int __attribute__((optimize("-O2"), noinline)) test_this_magnify_rounds(int magnify_rounds) {
  uint64_t dur = 0;

  for (int j = 0; j < N_TESTS; j++) {
      for (volatile int i = 0; i < 200; i++); //this is important to let the instruction finish
      asm volatile("mfence");
      // two path starts that the same time as each depends on a cache miss
      _mm_clflush(&mem[0x1]);
      _mm_clflush(&mem[0x1000]);
      for (volatile int i = 0; i < 200; i++); //this is important to let the instruction finish
      dur = magnify(magnify_rounds);
      total[0][j] = dur;

      // only one path starts later as it depends on a cache miss (mem[0])
      _mm_clflush(&mem[0x1]);
      for (volatile int i = 0; i < 200; i++); //this is important to let the instruction finish
      dur = magnify(magnify_rounds);
      total[1][j] = dur;
  }

  for (int i = 0; i < 2; i++) {
    qsort((void *)total[i], N_TESTS, sizeof(total[i][0]), comparator);
  }
  return total[0][N_TESTS/2] - total[1][N_TESTS/2];
}

int main(int argc, char *argv[])
{
  printf("start testing:\n");
  int result[3];
  for (int round_num = 2; round_num >= 0; round_num-=1) {
    result[round_num] = test_this_magnify_rounds(400);
  }
  qsort((void *)result, 3, sizeof(int), comparator);
  if (result[1] < 1000 || result[1] > 20000) {
    printf("Failed. Keeping modifying the path length...\n");
    return -1; 
  }
  int result_400 = result[1];

  for (int round_num = 2; round_num >= 0; round_num-=1) {
    result[round_num] = test_this_magnify_rounds(800);
  }
  qsort((void *)result, 3, sizeof(int), comparator);
  if (result[1] < result_400*1.5 || result[1] > result_400*2.5) {
    printf("Failed. Keeping modifying the path length...\n");
    return -1; 
  }
  int result_800 = result[1];

  for (int round_num = 2; round_num >= 0; round_num-=1) {
    result[round_num] = test_this_magnify_rounds(1600);
  }
  qsort((void *)result, 3, sizeof(int), comparator);
  if (result[1] < result_800*1.5 || result[1] > result_800*2.5) {
    printf("Failed. Keeping modifying the path length...\n");
    return -1; 
  }
  printf("Test succeeds. Start collecting data.\n");

  int start = 100, end = 6000, step = 400; // default values
  printf("start = %d, end = %d, step = %d\n", start, end, step);
  mem[0x0] = 0x0;
  mem[0x1000] = 0x0;

  // each round of magnify process will accumulate the same amount of timing differences
  for (int magnify_rounds = start; magnify_rounds < end; magnify_rounds += step) {
    printf("magnify_rounds = %d, ", magnify_rounds);
    for (int round_num = 2; round_num >= 0; round_num-=1) {
      result[round_num] = test_this_magnify_rounds(magnify_rounds);
    }
    qsort((void *)result, 3, sizeof(int), comparator);
    printf("diff = %d cycles\n", result[1]);
  }
  return 0;
}