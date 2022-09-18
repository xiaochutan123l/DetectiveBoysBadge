#include "audIO_macro.h"

extern void i2s_init(void);
extern size_t read_audio(char *i2s_read_buff, int i2s_read_len);
extern size_t write_audio(char *i2s_write_buff, int i2s_write_len);
extern void _read_enable();
extern void _read_disable();
