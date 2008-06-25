gcc -fpack-struct -std=c99 -c test.c -o test.o

ar -r -v test.a test.o ../out/userlib.o ../out/string.o
ld -o test.obj -Ttext 0x80000000 -e _ASMENTRY ../userlib/entry.o test.a

objcopy -R .note -R .comment -S -O binary test.obj ../out/test.exe


