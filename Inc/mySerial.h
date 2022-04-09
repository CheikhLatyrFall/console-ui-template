#ifndef SERIAL
#define SERIAL

#include <string>

class mySerial
{
public:
  int handle;
  std::string deviceName;
  int baud;

  mySerial();
  mySerial(std::string deviceName, int baud);
  ~mySerial();

  void setDeviceName (std::string deviceName);
  void setBaud (int baud);
  bool Send(unsigned char *data, int len);
  bool Send(unsigned char value);
  bool Send(std::string value);
  int Receive(unsigned char *data, int len);
  bool IsOpen(void);
  void Close(void);
  bool Open(std::string deviceName, int baud);
  bool Open();
  bool NumberByteRcv(int &bytelen);
};

#endif
