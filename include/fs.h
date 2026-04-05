#ifndef INCLUDE_FS_H
#define INCLUDE_FS_H

#define FS_MAGIC     0x534F4653U
#define FS_MAX_NAME  64

struct fs_header {
    unsigned int magic;
    unsigned int num_files;
};

struct fs_file_entry {
    char name[FS_MAX_NAME];
    unsigned int offset;
    unsigned int size;
};

struct fs_instance {
    unsigned int base_addr;
    struct fs_header *header;
    struct fs_file_entry *entries;
};

int  fs_init(struct fs_instance *fs, unsigned int base_addr);
int  fs_num_files(struct fs_instance *fs);
int  fs_find(struct fs_instance *fs, const char *name);
unsigned int fs_file_data(struct fs_instance *fs, int index);
unsigned int fs_file_size(struct fs_instance *fs, int index);
const char  *fs_file_name(struct fs_instance *fs, int index);

#endif /* INCLUDE_FS_H */
