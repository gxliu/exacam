#include <iostream>
#include <libusb.h>
#include <stdexcept>
using namespace std;

int main(void) {
  /* initialize */
  libusb_init(NULL);
  
  libusb_device_handle* dev_handle = NULL;
  try {
    /* open device */
    dev_handle = libusb_open_device_with_vid_pid(NULL, 0xfffe, 0x0100);
    if (!dev_handle) throw std::runtime_error("Could not find device");
    
    /* read firmware version */
    char data[65];
    cout << "firmware version: " << endl;
    int ret = libusb_control_transfer(dev_handle, 0xC0, 0x94, 0, 0, (unsigned char*)data, 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    else { cout << "received " << ret << " bytes: "; data[ret] = '\0'; cout << data << endl; }
    
    cout << "firmware git revision: " << endl;
    ret = libusb_control_transfer(dev_handle, 0xC0, 0x95, 0, 0, (unsigned char*)data, 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    else { cout << "received " << ret << " bytes: "; data[ret] = '\0'; cout << data << endl; }
    
  }
  catch(const std::runtime_error& e) {
    cout << "ERROR: " << e.what() << endl;
  }
  
  /* deinitialize */
  if (dev_handle) libusb_close(dev_handle);
  libusb_exit(NULL);
  return 0;
}
