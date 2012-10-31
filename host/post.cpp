#include <fstream>
#include <iostream>
using namespace std;

int main(void) {
  ifstream in("output.data");
  long begin = in.tellg();
  in.seekg (0, ios::end);
  long end = in.tellg();
  long size = end - begin;
  cout << "file is " << size << " bytes" << endl;

  char* in_data = new char[size];
  in.seekg(0);
  in.read(in_data, size);
  char* out_data = new char[size * 3];
  for (int i = 0; i < size; i++) {
    /*char y = (in_data[i] >> 4) * 16;
    out_data[i * 3 + 0] = y;
    out_data[i * 3 + 1] = y;
    out_data[i * 3 + 2] = y;    */
    /*out_data[i * 3 + 0] = (in_data[i] & 0x0C) >> 2;
    out_data[i * 3 + 1] = in_data[i] & 0x03;
    out_data[i * 3 + 2] = (in_data[i] & 0xF0) >> 4;*/
  }
  
  ofstream out("output2.data");
  out.write(out_data, size * 3);
  
  delete[] in_data;
  delete[] out_data;
}
