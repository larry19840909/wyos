nasm boot/WYOSBOOT.asm -f bin -o out/WYOSBOOT.bin
nasm boot/setup.asm -f bin -o out/setup.bin
nasm boot/tmp.asm -f bin -o out/tmp.bin

nasm -f coff kernel/head.asm -o out/head.o
nasm -f coff video/print.asm -o out/print.o
nasm -f coff kernel/exp.asm -o out/exp.o
nasm -f coff userlib/entry.asm -o userlib/entry.o

gcc -fpack-struct -std=c99 -c kernel/kernel.c -o out/kernel.o
gcc -fpack-struct -std=c99 -c kernel/cpu.c -o out/cpu.o
gcc -fpack-struct -std=c99 -c kernel/mutex.c -o out/mutex.o
gcc -fpack-struct -std=c99 -c video/video.c -o out/video.o
gcc -fpack-struct -std=c99 -c lib/math.c -o out/math.o
gcc -fpack-struct -std=c99 -c lib/string.c -o out/string.o
gcc -fpack-struct -std=c99 -c mm/memory.c -o out/mm.o
gcc -fpack-struct -std=c99 -c io/io.c -o out/io.o
gcc -fpack-struct -std=c99 -c syscall/syscall.c -o out/syscall.o
gcc -fpack-struct -std=c99 -c userlib/userlib.c -o out/userlib.o
gcc -fpack-struct -std=c99 -c process/process.c -o out/proc.o
gcc -fpack-struct -std=c99 -c process/thread.c -o out/thread.o
gcc -fpack-struct -std=c99 -c process/block.c -o out/block.o
gcc -fpack-struct -std=c99 -c message/message.c -o out/msg.o
gcc -fpack-struct -std=c99 -c io/dma.c -o out/dma.o
gcc -fpack-struct -std=c99 -c driver/driver.c -o out/driver.o
gcc -fpack-struct -std=c99 -c driver/disk/floppy.c -o out/floppy.o
gcc -fpack-struct -std=c99 -c fs/fat12.c -o out/fat12.o

cd out

ar -r -v extern.a print.o video.o string.o  mm.o mutex.o math.o syscall.o proc.o block.o msg.o exp.o

ar -r -v driver.a cpu.o  io.o driver.o floppy.o dma.o thread.o fat12.o


cd..

ld -M -x -o out/kernel.ld -Ttext 0x100000 -e _WYOSEntry out/head.o out/kernel.o out/extern.a out/driver.a>out/system.map
objcopy -R .note -R .comment -S -O binary out/kernel.ld out/kernel.bin

cd out
fdmake -v -i -o WYOSFLP.IMG -b WYOSBOOT.BIN -a kernel.bin -a setup.bin -a tmp.bin -a test.EXE
cd..
copy .\out\WYOSFLP.IMG WYOSFLP.IMG

