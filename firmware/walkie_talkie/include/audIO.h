#pragma once

// #TODO: need to accept parameters by initialisation. 
// #TODO: add a Flash Manager to enable flash handling. 
class AudIO {
public:
    AudIO();
    void init();
    int audIO_read(char* read_buf, int read_buf_len);
    int audIO_write(char* write_buf, int write_buf_len);
    void read_enable();
    void read_disable();
    ~AudIO();
};