#ifndef LIB_PS_CD_ID_H
#define LIB_PS_CD_ID_H

#include <stdio.h>
#include <stdbool.h>

extern char boot_file[11];

int is_ps_cd(FILE *bin);
int get_boot_file_name(FILE *bin);
int id_rev(FILE *bin, const unsigned int difference_offset, const unsigned char old_byte, const unsigned char new_byte);

#endif /* LIB_PS_CD_ID_H */