#include "multiboot.h"
#include "pfa.h"

#define MAX_FRAMES (1024U * 1024U) /* 4 GiB / 4 KiB */
#define BITMAP_WORD_BITS 32U
#define BITMAP_WORDS (MAX_FRAMES / BITMAP_WORD_BITS)

static unsigned int frame_bitmap[BITMAP_WORDS];
static unsigned int total_frame_count;
static unsigned int used_frame_count;

static unsigned int align_up(unsigned int value, unsigned int align)
{
    return (value + (align - 1U)) & ~(align - 1U);
}

static unsigned int align_down(unsigned int value, unsigned int align)
{
    return value & ~(align - 1U);
}

static void mark_used(unsigned int frame_idx)
{
    unsigned int word = frame_idx / BITMAP_WORD_BITS;
    unsigned int bit = frame_idx % BITMAP_WORD_BITS;
    unsigned int mask = 1U << bit;

    if ((frame_bitmap[word] & mask) == 0U) {
        frame_bitmap[word] |= mask;
        used_frame_count++;
    }
}

static void mark_free(unsigned int frame_idx)
{
    unsigned int word = frame_idx / BITMAP_WORD_BITS;
    unsigned int bit = frame_idx % BITMAP_WORD_BITS;
    unsigned int mask = 1U << bit;

    if ((frame_bitmap[word] & mask) != 0U) {
        frame_bitmap[word] &= ~mask;
        used_frame_count--;
    }
}

static int is_used(unsigned int frame_idx)
{
    unsigned int word = frame_idx / BITMAP_WORD_BITS;
    unsigned int bit = frame_idx % BITMAP_WORD_BITS;
    return (frame_bitmap[word] >> bit) & 1U;
}

static void mark_region_free(unsigned int start_addr, unsigned int end_addr)
{
    unsigned int frame_idx;
    unsigned int start = align_up(start_addr, PAGE_SIZE);
    unsigned int end = align_down(end_addr, PAGE_SIZE);

    if (end <= start) {
        return;
    }

    for (frame_idx = start / PAGE_SIZE; frame_idx < end / PAGE_SIZE; frame_idx++) {
        if (frame_idx < MAX_FRAMES) {
            mark_free(frame_idx);
        }
    }
}

static void mark_region_used(unsigned int start_addr, unsigned int end_addr)
{
    unsigned int frame_idx;
    unsigned int start = align_down(start_addr, PAGE_SIZE);
    unsigned int end = align_up(end_addr, PAGE_SIZE);

    if (end <= start) {
        return;
    }

    for (frame_idx = start / PAGE_SIZE; frame_idx < end / PAGE_SIZE; frame_idx++) {
        if (frame_idx < MAX_FRAMES) {
            mark_used(frame_idx);
        }
    }
}

void pfa_init(unsigned int multiboot_info_addr,
              unsigned int kernel_physical_start,
              unsigned int kernel_physical_end)
{
    unsigned int i;
    struct multiboot_info *mbi = (struct multiboot_info *)multiboot_info_addr;

    total_frame_count = MAX_FRAMES;
    used_frame_count = MAX_FRAMES;

    for (i = 0; i < BITMAP_WORDS; i++) {
        frame_bitmap[i] = 0xFFFFFFFFU;
    }

    if ((mbi->flags & MULTIBOOT_INFO_MMAP) != 0U) {
        unsigned int mmap_end = mbi->mmap_addr + mbi->mmap_length;
        unsigned int mmap_curr = mbi->mmap_addr;

        while (mmap_curr < mmap_end) {
            struct multiboot_mmap_entry *entry =
                (struct multiboot_mmap_entry *)mmap_curr;

            if (entry->type == 1U) {
                unsigned long long addr64 = entry->addr;
                unsigned long long len64 = entry->len;
                unsigned long long end64 = addr64 + len64;

                if (addr64 < 0x100000000ULL) {
                    unsigned int start = (unsigned int)addr64;
                    unsigned int end = (end64 > 0x100000000ULL)
                                           ? 0xFFFFFFFFU
                                           : (unsigned int)end64;
                    mark_region_free(start, end);
                }
            }

            mmap_curr += entry->size + sizeof(entry->size);
        }
    } else if ((mbi->flags & MULTIBOOT_INFO_MEMORY) != 0U) {
        unsigned int mem_top = 0x00100000U + (mbi->mem_upper * 1024U);
        mark_region_free(0x00100000U, mem_top);
    }

    /* Do not ever hand out frame zero. */
    mark_region_used(0U, PAGE_SIZE);

    /* The kernel image is owned by the kernel, not free memory. */
    mark_region_used(kernel_physical_start, kernel_physical_end);

    /* Reserve the Multiboot info structure itself. */
    mark_region_used(multiboot_info_addr,
                     multiboot_info_addr + sizeof(struct multiboot_info));

    /* Reserve the GRUB-provided memory map table. */
    if ((mbi->flags & MULTIBOOT_INFO_MMAP) != 0U) {
        mark_region_used(mbi->mmap_addr, mbi->mmap_addr + mbi->mmap_length);
    }
}

unsigned int pfa_alloc(void)
{
    unsigned int frame_idx;
    for (frame_idx = 0; frame_idx < MAX_FRAMES; frame_idx++) {
        if (!is_used(frame_idx)) {
            mark_used(frame_idx);
            return frame_idx * PAGE_SIZE;
        }
    }
    return 0U;
}

void pfa_free(unsigned int frame_addr)
{
    unsigned int frame_idx = frame_addr / PAGE_SIZE;
    if (frame_idx < MAX_FRAMES) {
        mark_free(frame_idx);
    }
}

unsigned int pfa_alloc_contiguous(unsigned int frame_count)
{
    unsigned int run = 0U;
    unsigned int run_start = 0U;
    unsigned int frame_idx;

    if (frame_count == 0U) {
        return 0U;
    }

    for (frame_idx = 0; frame_idx < MAX_FRAMES; frame_idx++) {
        if (!is_used(frame_idx)) {
            if (run == 0U) {
                run_start = frame_idx;
            }
            run++;
            if (run == frame_count) {
                unsigned int i;
                for (i = 0; i < frame_count; i++) {
                    mark_used(run_start + i);
                }
                return run_start * PAGE_SIZE;
            }
        } else {
            run = 0U;
        }
    }

    return 0U;
}

void pfa_free_contiguous(unsigned int frame_addr, unsigned int frame_count)
{
    unsigned int base = frame_addr / PAGE_SIZE;
    unsigned int i;

    for (i = 0; i < frame_count; i++) {
        if ((base + i) < MAX_FRAMES) {
            mark_free(base + i);
        }
    }
}

unsigned int pfa_total_frames(void)
{
    return total_frame_count;
}

unsigned int pfa_used_frames(void)
{
    return used_frame_count;
}

unsigned int pfa_free_frames(void)
{
    return total_frame_count - used_frame_count;
}
