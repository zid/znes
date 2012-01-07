unsigned char *rom_getbytes(void);
unsigned int rom_get_mirror(void);
unsigned char *rom_get_chr(unsigned int);
unsigned int rom_chr_is_readonly(void);
void init_rom(unsigned char *);
void rom_load_bank(unsigned int, unsigned int, unsigned int);
unsigned int rom_get_mapper(void);
unsigned char *rom_get_bank(signed int, unsigned int);
