#include <iostream>
#include <libusb.h>
#include <stdexcept>
#include <fstream>
#include <signal.h>
#include <string.h>
#include <vector>
using namespace std;

bool should_stop = false;
long received = 0;
std::ofstream file;

void transfer_ended(struct libusb_transfer *transfer) {
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    int num_packets = transfer->num_iso_packets;
    //cout << "packets: " << num_packets << endl;
    
    int completed = 0;
    for (int i = 0; i < num_packets; i++) {
      libusb_iso_packet_descriptor* pack = &transfer->iso_packet_desc[i];
      if (pack->status == LIBUSB_TRANSFER_COMPLETED) {
        cout << "length: " << pack->actual_length << endl;
        
        //cout << "received: " << transfer->length << " bytes" << endl;
        if (pack->actual_length != 0) {
          received += pack->actual_length;
          const unsigned char* buf = libusb_get_iso_packet_buffer_simple(transfer, i);
          file.write((const char*)buf, pack->actual_length); // TODO: default data seems to be YUYV
        }
        completed++;
      }
      else if (pack->status == LIBUSB_TRANSFER_OVERFLOW) cout << "overflow" << endl;
      else if (pack->status == LIBUSB_TRANSFER_STALL) cout << "stall" << endl;
      else if (pack->status == LIBUSB_TRANSFER_TIMED_OUT) cout << "timeout " << endl;
      else if (pack->status == LIBUSB_TRANSFER_ERROR) cout << "fail" << endl;
    }
    if (completed == 0) cout << "no transfers completed" << endl;
    else cout << "completed " << completed << endl;
  }
  else cout << "error" << endl;
  
  libusb_submit_transfer(transfer);
}

class Request {
  public:
    Request(libusb_device_handle* _dev_handle, int _packet_length, int _packets, int _buf_multiplier) : transfer(NULL) {
      dev_handle = _dev_handle;
      packet_length = _packet_length;
      packets = _packets;
      buf_multiplier = _buf_multiplier;
      buf_size = packet_length * packets * buf_multiplier;
      data.resize(buf_size, 0);      
    }
    
    void init(void)  {
      transfer = libusb_alloc_transfer(packets);
      libusb_fill_iso_transfer(transfer, dev_handle, 0x82, &data[0], buf_size, packets, transfer_ended, NULL, 100);
      libusb_set_iso_packet_lengths(transfer, packet_length);    
    }    
    
    void deinit(void) {
      libusb_cancel_transfer(transfer);
      libusb_free_transfer(transfer);
    }
        
    vector<unsigned char> data;
    int packet_length, packets, buf_multiplier, buf_size;
    libusb_transfer* transfer;  
    libusb_device_handle* dev_handle;
};



void interrupt(int s) {
  cout << "Stopping..." << endl;
  should_stop = true;
}

int main(void) {
  signal(SIGINT, interrupt);
  signal(SIGTERM, interrupt);
  
  file.open("output.data");
  
  /* initialize */
  libusb_init(NULL);
  libusb_set_debug(NULL, 3);
  
  size_t packet_length = 1024;
  int packets = 160;
  int buf_multiplier = 5;
  int request_num = 5;
  
  libusb_device_handle* dev_handle = NULL;
  vector<Request> requests;
  
  try {
    /* open device */
    dev_handle = libusb_open_device_with_vid_pid(NULL, 0xfffe, 0x0100);
    if (!dev_handle) throw std::runtime_error("Could not find device");
    
    libusb_claim_interface(dev_handle, 0);
    
    /* read firmware version */
    vector<unsigned char> data(65);
    cout << "firmware version: " << endl;
    int ret = libusb_control_transfer(dev_handle, 0xC0, 0x94, 0, 0, &data[0], 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    else { cout << "received " << ret << " bytes: "; data[ret] = '\0'; cout << (const char*)&data[0] << endl; }
    
    cout << "firmware git revision: " << endl;
    ret = libusb_control_transfer(dev_handle, 0xC0, 0x95, 0, 0, &data[0], 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    else { cout << "received " << ret << " bytes: "; data[ret] = '\0'; cout << (const char*)&data[0] << endl; }
    
    requests.resize(request_num, Request(dev_handle, packet_length, packets, buf_multiplier));
    for (int i = 0; i < request_num; i++) requests[i].init();
    
    ret = libusb_control_transfer(dev_handle, 0xC0, 0x96, 0, 0, &data[0], 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    
    // i2c test
    /*ret = libusb_control_transfer(dev_handle, 0xC0, 0x97, 0, 0, &data[0], 64, 100);
    if (ret == LIBUSB_ERROR_TIMEOUT) { cout << "transfer timed out" << endl; }
    else if (ret == 1) { cout << "returned: " << (int)data[0] << endl; }
    else { data[ret] = '\0'; cout << "error: " << (const char*)&data[0] << endl; }*/
    
    for (int i = 0; i < request_num; i++) libusb_submit_transfer(requests[i].transfer);      
    while (!should_stop) {
      
      libusb_handle_events(NULL);
    }
  }
  catch(const std::runtime_error& e) {
    cout << "ERROR: " << e.what() << endl;
  }
  
  cout << "received: " << (received / 1024) << "Kb" << endl;

  for (int i = 0; i < request_num; i++) requests[i].deinit();
  
  if (dev_handle) {
    libusb_release_interface(dev_handle, 0);  
    libusb_close(dev_handle);
  }
  libusb_exit(NULL);
  return 0;
}
