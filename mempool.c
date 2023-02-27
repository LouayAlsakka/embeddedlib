/*
Copyright <2023> <Louay Alsakka>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Algorithm:
this very fast malloc have predicable and always the same  allocation time, it basicly use multiple link lists with different sizes, it can be configured for example to use 32K block and break it to 8/16/32/64/128/256/512/1024 blocks, where mem element size double and number of elements in a block half.
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


Code size:

Use Cases:
This is a very small footprint heap that have predicable allocation performence. it is best suited for embedded applications where regular heap unpredictability  can't be tolerated (always it is a bad idea to use regular heap in embedded applications except at init time) this power of two block sizes allow to fulfill different mem requirments with minimal waste, if for example 200 bytes requested, 256 bytes return, and if no more in that block type, then 512 and if no more 512, 1024 is returned. if 1024 is full also, then it return Null.
Heap also allow using it to debug allocations/free issues. profiling the memory constantly to make sure no leakage, double free  can also be detected. Free 0 is harmless and ignored

To compile:
This can easly integrated with any existing code/make but for simple testing on M1 MAC I used:

gcc   mempool.c main.c

Performence:
this is a fast link list travel that allocate the smallest buffer available that fit the size
Verification:
main.c have some verification
*/

#include "mempool.h"

//static

memb_t *mpp;
uint8_t memplm;//mempoollogmask
volatile int16_t fmpp[MAXPOOL];
volatile uint16_t nfmb[MAXPOOL];
volatile uint16_t nfmbmin[MAXPOOL];
int mempool_debug;

void mp_init(ADDR_T memaddr)
{
    mempool_debug=0;
    mpp=(memb_t *)memaddr;
    for(int i=0;i<MAXPOOL;i++)
    {
        nfmb[i]=1<<(BSB-i-MINBSIB);
        nfmbmin[i]=nfmb[i];
        fmpp[i]=i<<(BSB-MINBSIB);
        memb_t *memb=(memb_t *)memaddr;
        int16_t *n=0;
        for(int bi=0;bi<nfmb[i];bi++)
        {
            n=(int16_t*)(memaddr+(i<<BSB)+(bi<<(i+MINBSIB)));//+MINBSIB since we start with  2 words not one
            *n=(i<<(BSB-MINBSIB))+((bi+1)<<(i));//since we put address in words
            //myprintf("[%x : %x]\n",n,*n);
        }
        *n=-1;
    }
    memplm=0x00;
}

//size can be 1 to 1024
int16_t  memb_alloc(uint16_t s){

    if(s<(1<<MINBSIB))
        s=1<<MINBSIB;
    if(s>(1<<(MINBSIB+MAXPOOL-1)))
        return -1;
    int8_t t=0;
    //this to check if the instance size fit the size we want.
    //0x100 for example should handle size 0 to 0x100, so 0x100-1=0xFF when and it with s-1 it shouldn't change the value of s-1 
    while((((1<<(t+MINBSIB))-1)&(s-1))!=(s-1))
        t++;
    //this is not needed since it is cover in the check before if(s>(1<<(MINBSIB+MAXPOOL-1)))
    if(t>MAXPOOL-1)
        return -1;

    LOCKON;
    while(fmpp[t]<0)
    {
        if((++t)>(MAXPOOL-2))
            break;
        
    }
    if(t>MAXPOOL-1)
        return -1;
    
    int retv=fmpp[t];
    
    if(fmpp[t]>=0)
    {
        memb_t *mb=mpp+fmpp[t];
        fmpp[t]=mb->n;
        ASSERT(nfmb[t]);
        nfmb[t]--;
        if(nfmb[t] < nfmbmin[t])
            nfmbmin[t]=nfmb[t];
#if DEBUG        
        if((1<<t)&memplm)
            newcle(MEMPOOL,1, retv,nfmb[t],nfmbmin[t],__get_LR());
#endif        
    }    
    LOCKOFF;
//    if(mempool_debug)
//        myprintf("Alloc %x size %d\n",retv,s);

    //myprintf("memb_alloc: %x\n",retv);

    return retv;
}


uint8_t *  mem_alloc(uint16_t s){
    int16_t b=memb_alloc(s);
    if(b<0)return 0;
    //myprintf("mem_alloc: %x\n",b);
    return ((uint8_t *)idx2memb(b));
        
}

void  memb_free(int16_t mbidx){    
    uint16_t t=(mbidx>>(BSB-MINBSIB))&(MAXPOOL-1);
    int maxnofb=0;
//    if(mempool_debug)
    //       myprintf("Free %x \n",mbidx);

    //myprintf("memb_free: %x\n",mbidx);
    LOCKON;
#if DEBUG    
    if((1<<t)&memplm)
        newcle(MEMPOOL,2, mbidx,nfmb[t],nfmbmin[t],__get_LR());
#endif    
    nfmb[t]++;
    maxnofb=1<<(BSB-t-MINBSIB);
    if(nfmb[t]>maxnofb)
    {
#if 0
        
        int idx=200;
        ADDR_T *addr=0x2000634c;
        //*addr++=0xcafebaba;
        *addr++=mbidx;
        *addr++=(BSB-t-MINBSIB);
        //*addr++=maxnofb;
        *addr++=    (ADDR_T)__get_LR();
        *addr++=nfmb[t];
#endif        
        ASSERT(0);

    }

    
    int oldfmpp=fmpp[t];
    fmpp[t]=mbidx; 
    memb_t *mb=mpp+mbidx;
    mb->n=oldfmpp;
#if 1    
//    if(t==2)
    
#endif    
    LOCKOFF;
    return;
}

void  mem_free(uint8_t *m){
    //Free 0 is harmless
    if(m==0)
        return;
    int b=memb2idx((memb_t *)m);
    //myprintf("mem_FREE: %x @ %x\n",b,m);
    if(b<0)
    {
        myprintf("ERROR can't free %p\n",m);
        ASSERT(0);
        return;
    }
    memb_free(b);
}

memb_t * idx2memb(int idx){
    memb_t *mb=mpp+idx;
    return mb;
}

int memb2idx(memb_t *mb){
    if(mb==0)return -1;
    int idx=(int)(mb-mpp);
    //myprintf("\memb idx=%d mb=%08x\n",idx,(ADDR_T)mb);
    return idx;
}


int memprofile()
{
    for(int i=0;i<MAXPOOL;i++)
    {
        int maxnofb=1<<(BSB-i-MINBSIB);
        myprintf("Type %d  Current usage: %d blocks out of %d, %d %% Max Usage: %d blocks: %d %% CP=%d\n",i,(maxnofb-nfmb[i]),maxnofb,(maxnofb-nfmb[i])*100/maxnofb,(maxnofb-nfmbmin[i]), (maxnofb-nfmbmin[i])*100/maxnofb,fmpp[i]);
    }
    
    return 0;
}
