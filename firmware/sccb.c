#include <sccb.h>
#include <i2c.h>

char sccb_read(char addr) {
  __xdata char dummy = addr;
  i2c_write(0x21, &dummy, 1); // send address within write command
  i2c_read(0x21, &dummy, 1); // read value
  return dummy;
}

void sccb_write(char addr, char reg) {
  __xdata char dummy[2] = { addr, reg };
  i2c_write(0x21, dummy, 2);
}

void sccb_modify(char addr, char set, char unset) {
  char tmp = sccb_read(addr);
  sccb_write(addr, (tmp | set) & ~unset);
}
