#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../debug.h"

#define DEV_MEM "/dev/mem"

static void usage(char *pgm)
{
    
}

int main(int argc, char **argv)
{
    unsigned long phyaddr, phybase, off;
    unsigned long val;
    unsigned char *virtbase, *virtaddr;
    int pagesize, memfd, ret = 1, op = 0;
    if(argc < 3) {
        err("required arguments not present\n");
        usage(argv[0]);
        goto err1;
    }
    op = argv[1][1];
    if(op == 'w' && argc != 4) {
        err("invalid number of arguments for write");
        usage(argv[0]);
        goto err1;
    }
    if(op == 'w')
        val = strtoul(argv[3], NULL, 0);

    pagesize = getpagesize();
    phyaddr = strtoul(argv[2], NULL, 0);
    off = phyaddr & (pagesize - 1);
    phybase = phyaddr ^ off;
    dbg("phyaddr = %lx, phybase = %lx, off = %lx, op = %c", phyaddr, phybase, off, op);
    memfd = open(DEV_MEM, O_RDWR);
    if(memfd < 0) {
        perr("opening " DEV_MEM);
        goto err1;
    }
    virtbase = mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, phybase);
    if(virtbase == MAP_FAILED) {
        perr("mmap");
        goto err2;
    }

    virtaddr = virtbase + off;
    dbg("virtbase = %p, virtaddr = %p", virtbase, virtaddr);
    if(op == 'r') {
        val = *((unsigned long *)virtaddr);
        info("[value @ %#lx] = [%#lx]", phyaddr, val);
    } else if (op == 'w') {
        *((unsigned long *)virtaddr) = val;
        if(msync(virtbase, pagesize, MS_SYNC)) {
            perr("msync");
        }
    }
    ret = 0;
err3:
    if(munmap(virtbase, pagesize)) {
        perr("munmap");
    }
err2:
    close(memfd);
err1:
    return ret;
}
