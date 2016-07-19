/*
 * File      : mmu.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-04-15     ArdaFu     Add code for IAR
 */

#include "mmu.h"

/*----- Keil -----------------------------------------------------------------*/
#ifdef __CC_ARM
void mmu_setttbase(rt_uint32_t i)
{
    register rt_uint32_t value;

    /* Invalidates all TLBs.Domain access is selected as
     * client by configuring domain access register,
     * in that case access controlled by permission value
     * set by page table entry
     */
    value = 0;
    __asm volatile{ mcr p15, 0, value, c8, c7, 0 }
    value = 0x55555555;
    __asm volatile { mcr p15, 0, value, c3, c0, 0 }
    __asm volatile { mcr p15, 0, i, c2, c0, 0 }
}

void mmu_set_domain(rt_uint32_t i)
{
    __asm volatile { mcr p15, 0, i, c3, c0,  0 }
}

void mmu_enable()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        orr value, value, #0x01
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_disable()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        bic value, value, #0x01
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_enable_icache()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        orr value, value, #0x1000
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_enable_dcache()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        orr value, value, #0x04
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_disable_icache()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        bic value, value, #0x1000
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_disable_dcache()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        bic value, value, #0x04
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_enable_alignfault()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        orr value, value, #0x02
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_disable_alignfault()
{
    register rt_uint32_t value;

    __asm volatile
    {
        mrc p15, 0, value, c1, c0, 0
        bic value, value, #0x02
        mcr p15, 0, value, c1, c0, 0
    }
}

void mmu_clean_invalidated_cache_index(int index)
{
    __asm volatile { mcr p15, 0, index, c7, c14, 2 }
}

void mmu_clean_invalidated_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while(ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c14, 1 }
        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_clean_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c10, 1 }
        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile { MCR p15, 0, ptr, c7, c6, 1 }
        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_tlb()
{
    register rt_uint32_t value;

    value = 0;
    __asm volatile { mcr p15, 0, value, c8, c7, 0 }
}

void mmu_invalidate_icache()
{
    register rt_uint32_t value;

    value = 0;

    __asm volatile { mcr p15, 0, value, c7, c5, 0 }
}


void mmu_invalidate_dcache_all()
{
    register rt_uint32_t value;

    value = 0;

    __asm volatile { mcr p15, 0, value, c7, c6, 0 }
}
/*----- GNU ------------------------------------------------------------------*/
#elif defined(__GNUC__) || defined(__ICCARM__)
void mmu_setttbase(register rt_uint32_t i)
{
    register rt_uint32_t value;

    /* Invalidates all TLBs.Domain access is selected as
     * client by configuring domain access register,
     * in that case access controlled by permission value
     * set by page table entry
     */
    value = 0;
    asm volatile ("mcr p15, 0, %0, c8, c7, 0"::"r"(value));

    value = 0x55555555;
    asm volatile ("mcr p15, 0, %0, c3, c0, 0"::"r"(value));

    asm volatile ("mcr p15, 0, %0, c2, c0, 0"::"r"(i));

}

void mmu_set_domain(register rt_uint32_t i)
{
    asm volatile ("mcr p15,0, %0, c3, c0,  0": :"r" (i));
}

void mmu_enable()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "orr r0, r0, #0x1 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );
}

void mmu_disable()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "bic r0, r0, #0x1 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_enable_icache()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "orr r0, r0, #(1<<12) \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );
}

void mmu_enable_dcache()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "orr r0, r0, #4 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_disable_icache()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "bic r0, r0, #(1<<12) \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_disable_dcache()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "bic r0, r0, #4 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_enable_alignfault()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "orr r0, r0, #2 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_disable_alignfault()
{
    asm volatile
    (
        "mrc p15, 0, r0, c1, c0, 0 \n"
        "bic r0, r0, #2 \n"
        "mcr p15, 0, r0, c1, c0, 0 \n"
        :::"r0"
    );

}

void mmu_clean_invalidated_cache_index(int index)
{
    asm volatile ("mcr p15, 0, %0, c7, c14, 2": :"r" (index));
}

void mmu_clean_invalidated_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while(ptr < buffer + size)
    {
        asm volatile ("mcr p15, 0, %0, c7, c14, 1": :"r" (ptr));

        ptr += CACHE_LINE_SIZE;
    }
}


void mmu_clean_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        asm volatile ("mcr p15, 0, %0, c7, c10, 1": :"r" (ptr));

        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_dcache(rt_uint32_t buffer, rt_uint32_t size)
{
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        asm volatile ("mcr p15, 0, %0, c7, c6, 1": :"r" (ptr));

        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_tlb()
{
    asm volatile ("mcr p15, 0, %0, c8, c7, 0": :"r" (0));

}

void mmu_invalidate_icache()
{
    asm volatile ("mcr p15, 0, %0, c7, c5, 0": :"r" (0));

}

void mmu_invalidate_dcache_all()
{
    asm volatile ("mcr p15, 0, %0, c7, c6, 0": :"r" (0));

}
#endif

/* level1 page table */
#if defined(__ICCARM__)
#pragma data_alignment=(16*1024)
static volatile rt_uint32_t _page_table[4*1024];
#else
static volatile rt_uint32_t _page_table[(1+PROCESS_MAX)*4096] \
    __attribute__((aligned(16*1024)));
static volatile rt_uint32_t _small_table[PROCESS_MAX*(4+MMU_L2_SIZE)*256] \
    __attribute__((aligned(1024)));
#endif

void mmu_setmtt(rt_uint32_t vaddrStart, rt_uint32_t vaddrEnd,
                rt_uint32_t paddrStart, rt_uint32_t attr)
{
    volatile rt_uint32_t *pTT;
    volatile int nSec;
    int i = 0;
    pTT=(rt_uint32_t *)_page_table+(vaddrStart>>20);
    nSec=(vaddrEnd>>20)-(vaddrStart>>20);
    for(i=0; i<=nSec; i++)
    {
        *pTT = attr |(((paddrStart>>20)+i)<<20);
        pTT++;
    }
}

void mmu_setmap(rt_uint32_t pid, rt_uint32_t base, rt_uint32_t map, rt_uint32_t size)
{
    volatile rt_uint32_t *pTT;
    volatile int nSec,i,j;

    nSec = RT_ALIGN(size,0x10000)/0x10000;
    pTT=(rt_uint32_t *)_page_table+(pid*4096)+(map>>20);
    for (i=0; i<nSec; i++)
    {
        *pTT = DESC_PET|DOMAIN0|(rt_uint32_t )&_small_table[pid*(4+MMU_L2_SIZE)*256+i*256];
        pTT++;
    }
    nSec = size/4096;
    pTT = &_small_table[pid*(4+MMU_L2_SIZE)*256];
    for(j=0; j<nSec; j++)
    {
        *pTT = PET_RW_CB|base;
        base += 4096;
        pTT++;
    }
    mmu_invalidate_tlb();
}

void rt_hw_mmu_init(struct mem_desc *mdesc, rt_uint32_t size)
{
    /* disable I/D cache */
    mmu_disable_dcache();
    mmu_disable_icache();
    mmu_disable();
    mmu_invalidate_tlb();

    /* set page table */
    for (; size > 0; size--)
    {
        mmu_setmtt(mdesc->vaddr_start, mdesc->vaddr_end,
                   mdesc->paddr_start, mdesc->attr);
        mdesc++;
    }

    /* set MMU table address */
    mmu_setttbase((rt_uint32_t)_page_table);
    /* copy base MMU table */
    for (size=1; size<=PROCESS_MAX; size++)
    {
        rt_uint32_t index = 0;
        for (; index<4096; index++)
            _page_table[size*4096+index] = _page_table[index];
    }

    /* enables MMU */
    mmu_enable();

    /* enable Instruction Cache */
    mmu_enable_icache();

    /* enable Data Cache */
    mmu_enable_dcache();

    mmu_invalidate_icache();
    mmu_invalidate_dcache_all();
}
