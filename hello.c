__attribute__((section(".data"))) unsigned char hello[] = {
    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20,
    0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0a};

#define ECheckSum 0x12fb

// asm labels
extern char pl_start;
extern char pl_end;

__attribute__((naked)) void _payload(void) {
  __asm__ volatile(".intel_syntax noprefix\n"
                   "pl_start:\n"
                   "xor rbx, rbx\n"
                   "xor rcx, rcx\n"
                   "xor rdx, rdx\n"

                   "mov rax, 1\n"
                   "mov rdi, 1\n"
                   "movabs rsi, offset hello\n"
                   "mov rdx, 14\n"

                   "syscall\n"

                   "mov rax, 60\n"
                   "xor rdi, rdi\n"
                   "syscall\n"
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
  for (int i = 0; i < 14; i++) {
    checksum += ptr[i];
  }

  // if (0) {
  if (checksum != ECheckSum) {
    __asm__("ud2");
  }
}

void _start(void) {
  isFucked();
  _payload();
}
