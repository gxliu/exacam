#include <iostream>
#include <libusb.h>
#include <stdexcept>
#include <fstream>
#include <signal.h>
#include <string.h>
using namespace std;

bool should_stop = false;
long received = 0;

void transfer_ended(struct libusb_transfer *transfer) {
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    received += 1024;
    //cout << "received: " << transfer->actual_length << " bytes" << endl;
    ((std::ofstream*)transfer->user_data)->write((const char*)(transfer->buffer), 1024); // TODO: default data seems to be YUYV
  }
  else cout << "error" << endl;
}

void interrupt(int s) {
  cout << "Stopping..." << endl;
  should_stop = true;
}

int main(void) {
  signal(SIGINT, interrupt);
  signal(SIGTERM, interrupt);
  
  std::ofstream file("output.data");
  
  /* initialize */
  libusb_init(NULL);
  
  libusb_device_handle* dev_handle = NULL;
  libusb_transfer* transfer = NULL;
  unsigned char* iso_data = NULL;
  iso_data = new unsigned char[4096];
  memset(iso_data, 0, sizeof(char) * 4096);
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
    
    libusb_transfer* transfer = libusb_alloc_transfer(4);
    libusb_fill_iso_transfer(transfer, dev_handle, 0x86, iso_data, 4096, 4, transfer_ended, (void*)(&file), 100);
    libusb_set_iso_packet_lengths(transfer, 1024);
    
    cout << "stop? " << should_stop << endl;
    while (!should_stop) {
      libusb_submit_transfer(transfer);
      libusb_handle_events(NULL);
    }
  }
  catch(const std::runtime_error& e) {
    cout << "ERROR: " << e.what() << endl;
  }
  
  cout << "received: " << (received / 1024) << "Kb" << endl;
  
  /* deinitialize */
  if (iso_data) delete[] iso_data;
  
  if (transfer) libusb_free_transfer(transfer);
  
  if (dev_handle) libusb_close(dev_handle);
  libusb_exit(NULL);
  return 0;
}
