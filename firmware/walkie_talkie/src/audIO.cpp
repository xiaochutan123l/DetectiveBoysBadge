#include "audIO.h"

extern "C" {
#include "audIO_impl.h"
}

AudIO::AudIO() {
};

void AudIO::init() {
    i2s_init();
};

int AudIO::audIO_read(char* read_buf, int read_buf_len) {
    return read_audio(read_buf, read_buf_len);
};

int AudIO::audIO_write(char* write_buf, int write_buf_len){
    return write_audio(write_buf, write_buf_len);
};

void AudIO::read_enable() {
    _read_enable();
};

void AudIO::read_disable() {
    _read_disable();
};

AudIO::~AudIO() {
};