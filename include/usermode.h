#ifndef INCLUDE_USERMODE_H
#define INCLUDE_USERMODE_H

void enter_usermode(unsigned int eip, unsigned int esp_user,
                    unsigned int page_dir_phys);

#endif /* INCLUDE_USERMODE_H */
