#include <stdio.h>

static inline unsigned char rotl8(unsigned char v, int s) {
  return (unsigned char)((v << s) | (v >> (8 - s)));
}

void gen(const char *input) {
  unsigned char storage[128];
  for (int i = 0; i < 128; i++)
    storage[i] = (unsigned char)(i * 0x41 ^ 0xAA);

  unsigned char m = 0x5A;
  for (int i = 0; i < 14; i++) {
    unsigned char target = (unsigned char)input[i];
    unsigned char s3 = (unsigned char)(target - 0x07);
    unsigned char s2 = (unsigned char)(s3 ^ m);
    unsigned char s1 = (unsigned char)(s2 + (i * 0x13));
    unsigned char raw = rotl8(s1, 3);
    storage[(i * 9) % 128] = raw;
    m ^= target;
  }

  printf(
      "__attribute__((section(\".data\"))) unsigned char hello[128] = {\n    ");
  for (int i = 0; i < 128; i++) {
    printf("0x%02x%s", storage[i], (i == 127) ? "" : ", ");
    if ((i + 1) % 12 == 0)
      printf("\n    ");
  }
  printf("\n};\n");
}

int main() {
  gen("Hello, World!\n");
  return 0;
}
