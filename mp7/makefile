AS=nasm
GCC=i386-elf-gcc
LD=i386-elf-ld

GCC_OPTIONS = -m32 -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector -fleading-underscore -fno-asynchronous-unwind-tables

all: kernel.bin

clean:
	rm -f *.o *.bin

start.o: start.asm gdt_low.asm idt_low.asm irq_low.asm
	$(AS) -f elf -o start.o start.asm

utils.o: utils.C utils.H
	$(GCC) $(GCC_OPTIONS) -c -o utils.o utils.C

assert.o: assert.C assert.H
	$(GCC) $(GCC_OPTIONS) -c -o assert.o assert.C


# ==== VARIOUS LOW-LEVEL STUFF =====

gdt.o: gdt.C gdt.H
	$(GCC) $(GCC_OPTIONS) -c -o gdt.o gdt.C

machine.o: machine.C machine.H
	$(GCC) $(GCC_OPTIONS) -c -o machine.o machine.C

machine_low.o: machine_low.asm machine_low.H
	$(AS) -f elf -o machine_low.o machine_low.asm

# ==== EXCEPTIONS AND INTERRUPTS =====

idt.o: idt.C idt.H
	$(GCC) $(GCC_OPTIONS) -c -o idt.o idt.C

irq.o: irq.C irq.H
	$(GCC) $(GCC_OPTIONS) -c -o irq.o irq.C

exceptions.o: exceptions.C exceptions.H
	$(GCC) $(GCC_OPTIONS) -c -o exceptions.o exceptions.C

interrupts.o: interrupts.C interrupts.H
	$(GCC) $(GCC_OPTIONS) -c -o interrupts.o interrupts.C

# ==== DEVICES =====

console.o: console.C console.H
	$(GCC) $(GCC_OPTIONS) -c -o console.o console.C

simple_timer.o: simple_timer.C simple_timer.H
	$(GCC) $(GCC_OPTIONS) -c -o simple_timer.o simple_timer.C

simple_keyboard.o: simple_keyboard.C simple_keyboard.H
	$(GCC) $(GCC_OPTIONS) -c -o simple_keyboard.o simple_keyboard.C

simple_disk.o: simple_disk.C simple_disk.H
	$(GCC) $(GCC_OPTIONS) -c -o simple_disk.o simple_disk.C

# ==== FILE SYSTEM =====

file.o: file.C file.H
	$(GCC) $(GCC_OPTIONS) -c -o file.o file.C

file_system.o: file_system.C file_system.H simple_disk.H
	$(GCC) $(GCC_OPTIONS) -c -o file_system.o file_system.C

# ==== MEMORY =====

frame_pool.o: frame_pool.C frame_pool.H 
	$(GCC) $(GCC_OPTIONS) -c -o frame_pool.o frame_pool.C

mem_pool.o: mem_pool.C mem_pool.H 
	$(GCC) $(GCC_OPTIONS) -c -o mem_pool.o mem_pool.C

# ==== KERNEL MAIN FILE =====

kernel.o: kernel.C machine.H console.H gdt.H idt.H irq.H exceptions.H interrupts.H simple_timer.H frame_pool.H mem_pool.H simple_disk.H file.H file_system.H
	$(GCC) $(GCC_OPTIONS) -c -o kernel.o kernel.C

kernel.bin: start.o utils.o kernel.o \
   assert.o console.o gdt.o idt.o irq.o exceptions.o \
   interrupts.o simple_timer.o simple_keyboard.o frame_pool.o mem_pool.o \
   simple_disk.o file.o file_system.o \
    machine.o machine_low.o 
	$(LD) -melf_i386 -T linker.ld -o kernel.bin start.o utils.o kernel.o \
   assert.o console.o gdt.o idt.o irq.o exceptions.o interrupts.o \
   simple_timer.o simple_keyboard.o frame_pool.o mem_pool.o \
   simple_disk.o file.o file_system.o \
    machine.o machine_low.o
