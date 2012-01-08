void init_cpu(void);
void cpu_nmi(unsigned int);
void cpu_cycle(void);
unsigned int cpu_getcycles(void);
void cpu_set_writeb(void (*wbfp)(unsigned int, unsigned char));
void cpu_set_readb(unsigned char (*rbfp)(unsigned int));
