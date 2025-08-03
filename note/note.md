```
pte: PPN + flags(10)
0x21FC885B
0000 0000 1000 0111 1111 00 10 00 10|00010 11011
             8    7    F     2     2|

 22000
-1F000
=03000

```
* PPN corresponds to physical address >> 12
* physical addresss lower 12 bits corresponds to virtual address lower 12 bits
* Last two pages are trapoline(RX--) and trapframe(R-W-)

**Examing permissions below**

```
00010 11011
not dirty, accessed, not global, user, executable, not writable, Readable, Valid

last page: 00 0100 1011
not dirty, accessed, not global, kernel, X-R, valid -> it's trampoline

second last page: 00 1100 0111
dirty( have been writen before ), accessed, not global, kernel, -WR, valid -> it's trapframe

```

