/*
Copyright <2023> <Louay Alsakka>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Algorithm:


Code size:

Use Cases:

To compile:
This can easly integrated with any existing code/make but for simple testing on M1 MAC I used:

gcc   mempool.c main.c

Performence:
this is a fast link list travel that allocate the smallest buffer available that fit the size
Verification:
main.c have some verification
*/



#ifndef __MEMPOOL_H
#define __MEMPOOL_H
#include "platform_specific.h"

//8 pools with each pool double the size and half the number
#define MAXPOOL 8
//Block  Bit size in bytes 4096 bytes=12 bits 1<<12
#define BSB 12
//Minimum block size in bits 3 means 8 byte block is the minimum block
#define MINBSIB 3


//for example base on this configurations
//32K allocated to the heap will have the following pools:
//pool 1: 2 words/8 bytes cb/cbdata 512
//pool 2 4 words/ 16 bytes  256
//pool 3: 8 words 32bytes   128
//pool 4: 16 words 64 bytes     64
//pool 5: 32 words 128 bytes    32
//pool 6 : 64 words 256 btes    16
//pool 7: 128 words 512 bytes    8 
//pool 8: 256 words 1024 bytes    4


typedef struct{
    int16_t n;
    uint8_t data[(1<<MINBSIB)-2];
}memb_t;


extern  memb_t * mpp;
//extern  int16_t *fmpp;



void mp_init(ADDR_T memaddr);
int16_t  memb_alloc(uint16_t t);
uint8_t *  mem_alloc(uint16_t s);

//int16  memb_alloc();
void  memb_free(int16_t mbidx);
void  mem_free(uint8_t *m);
memb_t * idx2memb(int idx);
int memb2idx(memb_t *mb);
int memprofile();
#endif
