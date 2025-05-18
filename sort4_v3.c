/*OA
Copyright <2023> <Louay Alsakka>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Algorithm:
I did quick search to see if such idea existed, but could't find anything that lead me to belivave it to be known or used before, if it is indeed a new method, I would like to call it bit sort, as it use bits to sort.
The basic idea here, to sort a uint32_~t large array, we start from first bit(MSB) to last bit(LSB), using the following recurisive steps:
Assume it is sorted up to bit n, it can be sorted up to bit n+1 using the following stpes:
- scan the list for machting segments (which have the first n bits are the same) this will generate start seg index and end segment index.
- Sort each sector(segment) found to the n+1 by sorting bit n+1: 
    -by using the sector boudaries(start and end index),intialize two pointers one point to the start and one point to the end.
    -keep skiping elements that are in place (bit n+1 is 0(1) for the start pointer, and bit n+1 is 1(0) for the end pointer for  sorting ascending/descending. start pointer increase while end pointer decrease.
    -one both pointers reach element in the wrong place, swap the content of start/end pointers.
    -repeat until start pointer exceed or equal end pointer.
-Repeat until reaching the end of the list.



Code size:
This is uint32_t sorting algo that I designed that take about 1200 bytes of code when compile for arm. it can be easly ported to small embedded without any indepdend need on any external libraries. it can be easly adapted to run in verilog for FPGA/ASIC implementations.

Use Cases:
The small code footfrint make this algo an excellent choice for embedded applications, beside the code size and the data that need to be sorted, there isn't much exta temporary data used.
it can easly be modified to sort uint64_t, int, uint16, strings and so on.
To sort strings, it either can work on one byte ascii which means for each characther need to be sorted it represent 8 bits, or a faster method by using subset of charechters and code it into 5 bits (32 charachters) or 6 bits (64 charachters). with the 6 bits per charachter the sort time will be less. it can also easly adaped to sort sign interfers. I might add this options laters.

To compile:
This can easly integrated with any existing code/make but for simple testing on M1 MAC I used:
gcc -O3 sort.c

Performence:
This Algo is O(N), it scan the list N times X Number of Bits(32/64/etc) X 2 
it take on M1 MAC 0.1 sec to sort 1M entires (32 bits unsigned) which seems very comparable to what it take numpy sort.
On M1 Apple M1 Max it use ~0.1 sec per 1M entries.
SORTED!!!! 0.108435 sec
From python if I load the same data and use sort
l1=np.loadtxt("/tmp/unsorted.txt")
len(l1)
1000000
cProfile.run(l1.sort()')
 ncalls  tottime  percall  cumtime  percall filename:lineno(function)
        1    0.000    0.000    0.091    0.091 <string>:1(<module>)
        1    0.000    0.000    0.091    0.091 {built-in method builtins.exec}
        1    0.000    0.000    0.000    0.000 {method 'disable' of '_lsprof.Profiler' objects}
        1    0.091    0.091    0.091    0.091 {method 'sort' of 'numpy.ndarray' objects}


Please note this sort algo can easly be adapted to run on multi thread and significantly reduce the proces time, findendsec will run in main thread and then each call to sortsec can be called in different thread, as each sortsec will work on different part of the list so there is no colision. this only work effectivly after passing the first few bits (bit 0 have one sec, bit 1 have two and so on, so from bit 3 can leverage up to 8 parallel CPU then each bit after can leveage more. which will lead to significant time reduction in a multi thread invirement with minimal code overhead. I might add an example on how to do so.

Verification:
I use Python to verify the results are well sorted, 
import numpy as np
l=np.loadtxt("/tmp/sorted.txt")
ld=np.gradient(l)
for 1M random numbers 
np.min(ld)
1.5
>>> np.max(ld)
19160.0
for 10M random numbers:
np.min(ld)
0.0
>>> np.max(ld)
2194.0
so the gradient is always positive or 0, meaning the random  entries are always increasing. if one element is smaller than the previous one it will generate negative gradient.
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct belowmeanstruct{
    uint32_t startidx;
    uint32_t endidx;
}belowmeanstruct_t;
int dbg=0;

void printrec(belowmeanstruct_t *p){
    printf("{%d %d}\n",p->startidx,p->endidx);
}

int  belowmeanin2(uint32_t *l , uint32_t llen,belowmeanstruct_t inopt,belowmeanstruct_t *out)
{
    belowmeanstruct_t*outopt=out;
    uint32_t caidx=inopt.startidx;
    uint32_t cbidx=inopt.endidx;
    uint32_t midx=caidx+((cbidx-caidx)>>1);
    int ret=0;
    if(l[caidx]>l[midx])
    {
        uint32_t tmp=l[caidx];
        l[caidx]=l[midx];
        l[midx]=tmp;                
    }

    if(l[cbidx]<l[midx])
    {
        uint32_t tmp=l[cbidx];
        l[cbidx]=l[midx];
        l[midx]=tmp;                
    }
    
    if(l[caidx]>l[midx])
    {
        uint32_t tmp=l[caidx];
        l[caidx]=l[midx];
        l[midx]=tmp;                
    }
    if(cbidx<=caidx+2)
        return(ret);
    uint32_t omean=l[midx];
    
    uint32_t acnt=0;
    uint32_t bcnt=0;

    while(caidx<cbidx)
    {
        while((l[caidx]<omean)&&(caidx<cbidx))
        {
            acnt++;
            caidx++;
        }
        while((l[cbidx]>=omean)&&(caidx<cbidx))
        {
            bcnt++;
            cbidx--;
        }
        if(caidx>=cbidx)
            break;
        uint32_t t=l[caidx];
        l[caidx]=l[cbidx];
        l[cbidx]=t;
        
    }
    if(acnt)
    {
        if((l[caidx]>=omean)||(caidx>=cbidx))
        {
            caidx--;
        }
        else
        {
            acnt++;

        }
    }
    if(bcnt)
    {
        if((l[cbidx]<omean)||(caidx>=cbidx))
        {
            cbidx++;
        }
        else
        {
            bcnt++;        
        }
    }
    if((acnt)&&(caidx>=inopt.startidx+1))
    {
        (outopt)->startidx=inopt.startidx;
        (outopt)->endidx=caidx;
        if(l[caidx]>=omean)
            (outopt)->endidx--;
        
        //printrec(outopt);
        ret++;

        (outopt)++;
    }
    if((bcnt)&&(inopt.endidx>=cbidx+1))
    {
        (outopt)->startidx=cbidx;
        (outopt)->endidx=inopt.endidx;
        if(l[cbidx]<=omean)
            (outopt)->startidx++;

        //(outopt)->mean=(uint32_t)((bsum+bcnt/2)/bcnt);
        //printrec(outopt);
        ret++;

        (outopt)++;
    }
    return(ret);
}

void belowmean(uint32_t *l , uint32_t llen)    
{
    belowmeanstruct_t *inoutp[2];
    inoutp[0]=(belowmeanstruct_t *)malloc(sizeof(belowmeanstruct_t)*(llen>>1));
    inoutp[1]=(belowmeanstruct_t *)malloc(sizeof(belowmeanstruct_t)*(llen>>1));
    uint32_t idx=0;    
    inoutp[0][0].startidx=0;
    inoutp[0][0].endidx=llen-1;    
    ///////////////
    
    int ff=0;
    int nff=ff^1;
    int icnt=1;
    int ocnt=0;
    belowmeanstruct_t * outp;
    do{
        for(int i=0;i<icnt;i++)
        {
            ocnt+=belowmeanin2(l,llen,inoutp[ff][i],inoutp[nff]+ocnt);
        }
        if(ocnt==0)
            break;
        ff^=1;
        nff=ff^1;
        icnt=ocnt;
        ocnt=0;
    }while(1);
    
}
//this code is just an example to test the  sort algo and profile it's performence..
int main()
{
    //
    int max=100000000;
    int maxbit=32;
    uint32_t *l=malloc(max*4);
    //randomize () ;
    srandom(time(NULL));
    for(int i=0;i<max;i++)
    {
        
        //srand(time(NULL));
        
        long int a=random();
        //long int a=random()&0xFF;
        //long int a=srand(time(NULL))&0xF;
        l[i]=a;
    }
    //saving the unsorted data into a file to compare it with other sort algos.
    FILE *f1=fopen("/tmp/unsorted.txt","w");
    for(int i=0;i<max;i++)
    {
        fprintf(f1,"%d\n",l[i]);
    }
    fclose(f1);
    printf("\nStarted!!!!\n");
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    belowmean(l,max);
    //sort(l,max,maxbit);
    //radix_sort(l,max);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nSORTED!!!! %f sec\n",cpu_time_used);
    FILE *f=fopen("/tmp/sorted.txt","w");
    for(int i=0;i<max;i++)
    {        
        fprintf(f,"%d\n",l[i]);
    }

    fclose(f);        
}

