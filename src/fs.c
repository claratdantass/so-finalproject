#include "fs.h"
#include "kutil.h"

int fs_init(struct fs_instance *fs, unsigned int base_addr)
{
    fs->base_addr = base_addr;
    fs->header = (struct fs_header *)base_addr;

    if (fs->header->magic != FS_MAGIC) {
        fs->header = (struct fs_header *)0;
        fs->entries = (struct fs_file_entry *)0;
        return -1;
    }

    fs->entries = (struct fs_file_entry *)(base_addr + sizeof(struct fs_header));
    return 0;
}

int fs_num_files(struct fs_instance *fs)
{
    if (!fs->header) {
        return 0;
    }
    return (int)fs->header->num_files;
}

int fs_find(struct fs_instance *fs, const char *name)
{
    unsigned int i;

    if (!fs->header) {
        return -1;
    }

    for (i = 0; i < fs->header->num_files; i++) {
        if (strcmp(fs->entries[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

unsigned int fs_file_data(struct fs_instance *fs, int index)
{
    return fs->base_addr + fs->entries[index].offset;
}

unsigned int fs_file_size(struct fs_instance *fs, int index)
{
    return fs->entries[index].size;
}

const char *fs_file_name(struct fs_instance *fs, int index)
{
    return fs->entries[index].name;
}

int fs_list(struct fs_instance *fs, char *buf, unsigned int buf_size)
{
    int num;
    unsigned int i, pos = 0;

    if (!fs->header)
        return 0;

    num = (int)fs->header->num_files;
    for (i = 0; i < (unsigned int)num && pos < buf_size - 1; i++) {
        const char *name = fs->entries[i].name;
        unsigned int nlen = strlen(name);
        if (pos + nlen + 1 >= buf_size)
            break;
        memcpy(buf + pos, name, nlen);
        pos += nlen;
        buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    return (int)pos;
}
