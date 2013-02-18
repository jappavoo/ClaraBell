#ifndef __ARDUINO_SERIAL_H__
#define __ARDUINO_SERIAL_H__

#ifdef __cplusplus
extern "C" {
#endif
  int serialport_init(const char* serialport, int baud);
  int serialport_writebyte(int fd, uint8_t b);
  int serialport_write(int fd, const char* str);
  int serialport_read_until(int fd, char* buf, char until);
  void serialport_read_loop(int fd);
  void serialport_write_loop(int fd);

#ifdef __cplusplus
}
#endif

#endif
