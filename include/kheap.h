#ifndef INCLUDE_KHEAP_H
#define INCLUDE_KHEAP_H

void kheap_init(void);
void *kmalloc(unsigned int bytes);
void kfree(void *ptr);

#endif /* INCLUDE_KHEAP_H */
