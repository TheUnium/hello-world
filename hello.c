unsigned char rtHello[16] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                             0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

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

#define ECheckSum 0x5f65

// asm labels
extern unsigned char pl_start[];
extern unsigned char pl_end[];

static inline unsigned char rotr8(unsigned char v, int s) {
  return (v >> s) | (v << (8 - s));
}

int cCS(int seed, int id) {
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

  if (id == 4)
    return 0;

  return x;
}

__attribute__((always_inline)) static inline void rec(void) {
  unsigned char m = 0x5A;
  for (volatile int i = 0; i < 14; i++) {
    unsigned char r = hello[(i * 9) % 128];
    unsigned char s1 = rotr8(r, 3);
    unsigned char s2 = s1 - (unsigned char)(i * 0x13);
    unsigned char s3 = s2 ^ m;

    rtHello[i] = s3 + 0x07;
    m ^= rtHello[i];
  }
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

  unsigned long pl_len = (unsigned long)&pl_end - (unsigned long)&pl_start;
  ptr = (unsigned char *)&pl_start;
  for (unsigned long i = 0; i < pl_len; i++) {
    checksum += ptr[i];
  }

  ptr = (unsigned char *)hello;
  for (int i = 0; i < 128; i++) {
    checksum += ptr[i];
  }

  ptr = (unsigned char *)rtHello;
  for (int i = 0; i < 16; i++) {
    checksum += ptr[i];
  }

  // if (0) {
  if (checksum != ECheckSum) {
    __asm__("ud2");
  }
}

void ccEntry(void) {
  int state = 0;
  unsigned char m = 0x5A;
  int i = 0;

  while (state != 99) {
    int st1 = cCS(i, 4);
    switch (state + st1) {
    case 0:
      i = 0;
      state = 1;
      break;
    case 1:
      if (i < 14)
        state = 2; // next bite
                   // nom nom
                   // yum
      else
        state = 3; // finished
      break;
    case 2: {
      unsigned char *b = (unsigned char *)hello;
      int o = (i * 9) % 128;
      unsigned char r = *(b + o);

      unsigned char s1 = rotr8(r, 3);
      unsigned char s2 = (unsigned char)(s1 - (i * 0x13));
      unsigned char s3 = (unsigned char)(s2 ^ m);
      unsigned char s4 = (unsigned char)(s3 + 0x07);

      rtHello[i] = s4;
      m ^= s4;

      i++;
      state = 1;
    } break;
    case 3:
      isFucked();
      state = 4;
      break;
    case 4:
      state = 99;
      __asm__ volatile(".intel_syntax noprefix\n"
                       "mov rax, offset _payload\n"
                       "push rax\n"
                       "ret\n"
                       ".att_syntax prefix\n");
      break;
    case 5:
      rec();
      cCS(i, 2);
      break;
    default:
      __asm__("ud2");
      break;
    }
  }
}

__attribute__((naked)) void _start(void) {
  __asm__ volatile(".intel_syntax noprefix\n"
                   "xor rbp, rbp\n"
                   "pop rdi\n"
                   "mov rsi, rsp\n"
                   "and rsp, -16\n"
                   "mov rax, offset ccEntry\n"
                   "call rax\n"
                   "mov rax, 60\n"
                   "xor rdi, rdi\n"
                   "syscall\n"
                   ".att_syntax prefix\n");
}
