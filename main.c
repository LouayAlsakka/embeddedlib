/*
Copyright <2023> <Louay Alsakka>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This code is to test mempool.c
*/


#include "stdio.h"
#include "stdlib.h"
#include "mempool.h"

int main()
{
    char *data=malloc(1024*32);
    printf("%p\n", data);

    mp_init((ADDR_T)data);
    memprofile();
    uint8_t* d[1000];
    int idx=0;
    for(uint16_t size=4;size<1000;size+=4)
    {
        d[idx++]=mem_alloc(size);
        if(d[idx-1]==0)
        {
            printf("\nFailed Allocating %d %d %p\n",idx-1,size,d[idx-1]);
            idx--;
        }
        else
            printf("\nAllocating %d %d %p\n",idx-1,size,d[idx-1]);
        memprofile();  
    }

    for(--idx;idx>=0;idx--)
    {
        printf("\nFreeing %d   %p\n",idx,d[idx]);
        mem_free(d[idx]);
        
        memprofile();  
    }
}
