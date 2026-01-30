# hello world

## how to build

```sh
gcc -nostdlib -fno-stack-protector -no-pie -o hello hello.c
```

## how to get checksum if payload is changed

uncomment `if (0) {` and comment out `if (checksum != ECheckSum) {`,
then:

```sh
gcc -g -nostdlib -fno-stack-protector -no-pie -o hello hello.c
gdb ./hello
layout asm
b isFucked
r
# use ni till you reach the cmpl instruction at the end of the function
# it should have a jl and nop instruction right after it
# take the address of the nop inst after jl
b *0x401076 # this is an eg
c
p /x *(unsigned int*)($rbp - 4)
```

update `ECheckSum` with the checksum and then
uncomment `if (checksum != ECheckSum) {` and comment out `if (0) {`

## expected result

```sh
$ ./hello
Hello, World!
```
