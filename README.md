# hello world

## how to build

```sh
gcc -nostdlib -fno-stack-protector -no-pie -o hello hello.c
```

## how to get checksum if payload is changed

1) run the program, itll give the right checksum before exiting

<details>
  <summary>if you remove the function</summary>

  1) gen the encoded message by using ./gen.c. you can build it using
`gcc gen.c -o gen` after editing its `main()` function.

  2) then, replace the `hello[]` array in `hello.c` with the array ./gen
outputs.

  3) uncomment `if (0) {` and comment out `if (checksum != ECheckSum) {`,
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
</details>

## expected result

```sh
$ ./hello
Hello, World!
```
