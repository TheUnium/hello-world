unsigned char rtHello[16];

__attribute__((section(".data"))) unsigned char hello[128] = {
    0xd8, 0xeb, 0x28, 0x69, 0xae, 0xef, 0x2c, 0x6d, 0xa2, 0xfa, 0x20, 0x61,
    0xa6, 0xe7, 0x24, 0x65, 0xba, 0xfb, 0xc1, 0x79, 0xbe, 0xff, 0x3c, 0x7d,
    0xb2, 0xf3, 0x30, 0xbd, 0xb6, 0xf7, 0x34, 0x75, 0x8a, 0xcb, 0x08, 0x49,
    0x5b, 0xcf, 0x0c, 0x4d, 0x82, 0xc3, 0x00, 0x41, 0x86, 0xe4, 0x04, 0x45,
    0x9a, 0xdb, 0x18, 0x59, 0x9e, 0xdf, 0xfc, 0x5d, 0x92, 0xd3, 0x10, 0x51,
    0x96, 0xd7, 0x14, 0x4e, 0xea, 0x2b, 0x68, 0xa9, 0xee, 0x2f, 0x6c, 0xad,
    0x1e, 0x23, 0x60, 0xa1, 0xe6, 0x27, 0x64, 0xa5, 0xfa, 0x97, 0x78, 0xb9,
    0xfe, 0x3f, 0x7c, 0xbd, 0xf2, 0x33, 0xcf, 0xb1, 0xf6, 0x37, 0x74, 0xb5,
    0xca, 0x0b, 0x48, 0x02, 0xce, 0x0f, 0x4c, 0x8d, 0xc2, 0x03, 0x40, 0x81,
    0x81, 0x07, 0x44, 0x85, 0xda, 0x1b, 0x58, 0x99, 0xde, 0x5b, 0x5c, 0x9d,
    0xd2, 0x13, 0x50, 0x91, 0xd6, 0x17, 0x54, 0x95};

extern unsigned char ccStart[];
extern unsigned char ccEnd[];

__attribute__((naked)) void _sSC(void) {
  __asm__ volatile("syscall\n"
                   "ret\n");
}

void print_hex(unsigned int val) {
  char hex[] = "0123456789ABCDEF";
  char buf[16];
  int idx = 0;
  buf[idx++] = '\n';
  for (int i = 0; i < 8; i++) {
    buf[idx++] = hex[val & 0xF];
    val >>= 4;
  }
  buf[idx++] = 'x';
  buf[idx++] = '0';
  buf[idx++] = ' ';
  buf[idx++] = ':';
  buf[idx++] = 'd';
  buf[idx++] = 'e';
  buf[idx++] = 't';
  buf[idx++] = 'c';
  buf[idx++] = 'e';
  buf[idx++] = 'p';
  buf[idx++] = 'x';
  buf[idx++] = 'E';

  for (int i = idx - 1; i >= 0; i--) {
    __asm__ volatile("mov $1, %%rdi\n"
                     "mov %0, %%rsi\n"
                     "mov $1, %%rdx\n"
                     "mov $1, %%rax\n"
                     "syscall\n" ::"r"(&buf[i])
                     : "rdi", "rsi", "rdx", "rax", "rcx", "r11");
  }
}

#define ECheckSum 0x00010C34
static inline unsigned char rotr8(unsigned char v, int s) {
  return (v >> s) | (v << (8 - s));
}

static inline unsigned long long rdtsc(void) {
  unsigned int lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)hi << 32) | lo;
}

int cCS(int seed) {
  volatile int x = seed;
  if ((x ^ x) != 0) {
    return 999;
  }

  for (int i = 0; i < 5; i++) {
    if (x % 2 == 0)
      x = x / 2;
    else
      x = 3 * x + 1;
  }

  return x;
}

__attribute__((naked)) void _payload(void) {
  __asm__ volatile(".intel_syntax noprefix\n"
                   ".global pl_start\n"
                   "pl_start:\n"
                   "push rbp\n"
                   "mov rbp, rsp\n"

                   "push rbx\n"
                   "mov rbx, 0xAA\n"
                   "xor rbx, 0xAA\n"
                   "add rbx, 0\n"
                   "pop rbx\n"

                   "mov rax, 1\n"
                   "mov rdi, 1\n"
                   "movabs rsi, offset rtHello\n"
                   "mov rdx, 14\n"
                   "syscall\n"

                   "mov rax, 60\n"
                   "xor rdi, rdi\n"
                   "syscall\n"

                   "push rcx\n"
                   "not rcx\n"
                   "not rcx\n"
                   "pop rcx\n"

                   ".global pl_end\n"
                   "pl_end:\n"
                   ".att_syntax prefix\n");
}

void isFucked(void) {
  unsigned int checksum = 0;
  unsigned char *ptr;

  unsigned long ccLen = (unsigned long)&ccEnd - (unsigned long)&ccStart;
  ptr = (unsigned char *)&ccStart;
  for (unsigned long i = 0; i < ccLen; i++) {
    checksum += ptr[i];
  }

  ptr = (unsigned char *)hello;
  for (int i = 0; i < 128; i++) {
    checksum += ptr[i];
  }

  // if (0) {
  if (checksum != ECheckSum) {
    print_hex(checksum);
    __asm__("ud2");
  }
}

void ccEntry(void) {
  __asm__ volatile(".global ccStart\n"
                   "ccStart:\n");

  int state = 0;
  unsigned char m = 0x5A;
  int i = 0;

  volatile unsigned char stChar;
  void (*volatile ssPtr)(void) = _sSC;
  const unsigned long long thDBG = 250;

  while (state != 99) {
    int st1 = cCS(i);
    switch (state) {
    case 0:
      isFucked();
      i = 0;
      state = 10;
      break;
    case 10:
      if (i < 14)
        state = 20; // next bite
                    // nom nom
                    // yum
      else
        state = 30; // finished
      break;
    case 20: {
      unsigned long long t1 = rdtsc();

      unsigned char *b = (unsigned char *)hello;
      int o = (i * 9) % 128;
      unsigned char r = *(b + o);

      unsigned char s1 = rotr8(r, 3);
      unsigned char s2 = (unsigned char)(s1 - (i * 0x13));

      unsigned long long t2 = rdtsc();
      if ((t2 - t1) > thDBG) {
        m += (0x11 ^ st1);
      }

      unsigned char s3 = (unsigned char)(s2 ^ m);
      unsigned char s4 = (unsigned char)(s3 + 0x07);

      stChar = s4;
      m ^= s4;

      __asm__ volatile("mov $1, %%rdi\n"
                       "mov %0, %%rsi\n"
                       "mov $1, %%rdx\n"
                       "mov $1, %%rax\n"
                       "call *%1\n"
                       :
                       : "r"(&stChar), "r"(ssPtr)
                       : "rdi", "rsi", "rdx", "rax", "rcx", "r11", "memory");
      stChar = 0;

      i++;
      state = 10 + (st1 & 0);
    } break;
    case 30:
      isFucked();
      state = 40;
      break;
    case 40:
      state = 99;
      cCS(i);
      break;
    default:
      __asm__("ud2");
      break;
    }
  }

  __asm__ volatile(".global ccEnd\n"
                   "ccEnd:\n");
}

__attribute__((naked)) void _start(void) {
  __asm__ volatile("xor %rbp, %rbp\n"
                   "pop %rdi\n"       // argc
                   "mov %rsp, %rsi\n" // argv
                   "and $-16, %rsp\n" // align stack
                   "mov $ccEntry, %rax\n"
                   "call *%rax\n"
                   "mov $60, %rax\n" // exit
                   "xor %rdi, %rdi\n"
                   "syscall\n");
}
