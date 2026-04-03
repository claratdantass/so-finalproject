#include "kheap.h"
#include "pfa.h"

typedef long Align;

union header {
    struct {
        union header *next;
        unsigned int units;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

static void kfree_internal(void *ptr);

static Header *morecore(unsigned int units)
{
    unsigned int bytes = units * (unsigned int)sizeof(Header);
    unsigned int pages = (bytes + PAGE_SIZE - 1U) / PAGE_SIZE;
    unsigned int addr = pfa_alloc_contiguous(pages);
    Header *new_block;

    if (addr == 0U) {
        return (Header *)0;
    }

    new_block = (Header *)addr;
    new_block->s.units = (pages * PAGE_SIZE) / (unsigned int)sizeof(Header);
    kfree_internal((void *)(new_block + 1));
    return freep;
}

void kheap_init(void)
{
    base.s.next = &base;
    base.s.units = 0U;
    freep = &base;
}

void *kmalloc(unsigned int bytes)
{
    Header *curr;
    Header *prev;
    unsigned int units;

    if (bytes == 0U) {
        return (void *)0;
    }

    units = (bytes + (unsigned int)sizeof(Header) - 1U) /
            (unsigned int)sizeof(Header) + 1U;

    if (freep == (Header *)0) {
        kheap_init();
    }

    prev = freep;
    curr = prev->s.next;

    for (;;) {
        if (curr->s.units >= units) {
            if (curr->s.units == units) {
                prev->s.next = curr->s.next;
            } else {
                curr->s.units -= units;
                curr += curr->s.units;
                curr->s.units = units;
            }
            freep = prev;
            return (void *)(curr + 1);
        }

        if (curr == freep) {
            if (morecore(units) == (Header *)0) {
                return (void *)0;
            }
            prev = freep;
            curr = prev->s.next;
            continue;
        }

        prev = curr;
        curr = curr->s.next;
    }
}

static void kfree_internal(void *ptr)
{
    Header *block;
    Header *curr;

    if (ptr == (void *)0) {
        return;
    }

    block = ((Header *)ptr) - 1;

    if (freep == (Header *)0) {
        kheap_init();
    }

    curr = freep;
    for (; !(block > curr && block < curr->s.next); curr = curr->s.next) {
        if (curr >= curr->s.next && (block > curr || block < curr->s.next)) {
            break;
        }
    }

    if (block + block->s.units == curr->s.next) {
        block->s.units += curr->s.next->s.units;
        block->s.next = curr->s.next->s.next;
    } else {
        block->s.next = curr->s.next;
    }

    if (curr + curr->s.units == block) {
        curr->s.units += block->s.units;
        curr->s.next = block->s.next;
    } else {
        curr->s.next = block;
    }

    freep = curr;
}

void kfree(void *ptr)
{
    kfree_internal(ptr);
}
