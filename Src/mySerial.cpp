extern "C" {
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
}
#include <iostream>
using namespace std;

#include "mySerial.h"

mySerial::mySerial()
{
   this->deviceName = "";
   this->baud = 0;
   handle=-1;
}

mySerial::mySerial(string deviceName, int baud)
{
   handle=-1;
   Open(deviceName,baud);
}

void mySerial::setDeviceName (string deviceName)
{
   this->deviceName=deviceName;
}

void mySerial::setBaud (int baud)
{
   this->baud = baud;
}

mySerial::~mySerial()
{
  if(handle >=0)
      Close();
}

void mySerial::Close(void)
{
   if(handle >=0)
      close(handle);
   handle = -1;
}

bool mySerial::Open()
{
   if (this->deviceName.size() == 0 || this->baud == 0)
      return false;

   struct termios tio;
   struct termios2 tio2;
   //this->deviceName=deviceName;
   //this->baud=baud;
   handle  = open(this->deviceName.c_str(),O_RDWR | O_NOCTTY /* | O_NONBLOCK */);

   if(handle <0)
      return false;
   tio.c_cflag =  CS8 | CLOCAL | CREAD;
   tio.c_iflag = IGNPAR | ICRNL;
   tio.c_oflag = 0;
   tio.c_lflag = ICANON;//0;//
   tio.c_cc[VMIN]=0;
   tio.c_cc[VTIME]=1;     // time out every .1 sec
   ioctl(handle,TCSETS,&tio);

   ioctl(handle,TCGETS2,&tio2);
   tio2.c_cflag &= ~CBAUD;
   tio2.c_cflag |= BOTHER;
   tio2.c_ispeed = baud;
   tio2.c_ospeed = baud;
   ioctl(handle,TCSETS2,&tio2);

//   flush buffer
   ioctl(handle,TCFLSH,TCIOFLUSH);

   return true;
}

bool mySerial::Open(string deviceName , int baud)
{
    struct termios tio;
    struct termios2 tio2;
    this->deviceName=deviceName;
    this->baud=baud;
    handle  = open(this->deviceName.c_str(),O_RDWR | O_NOCTTY /* | O_NONBLOCK */);

    if(handle <0)
       return false;
    tio.c_cflag =  CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR | ICRNL;
    tio.c_oflag = 0;
    tio.c_lflag = ICANON;//0;//
    tio.c_cc[VMIN]=0;
    tio.c_cc[VTIME]=1;     // time out every .1 sec
    ioctl(handle,TCSETS,&tio);

    ioctl(handle,TCGETS2,&tio2);
    tio2.c_cflag &= ~CBAUD;
    tio2.c_cflag |= BOTHER;
    tio2.c_ispeed = baud;
    tio2.c_ospeed = baud;
    ioctl(handle,TCSETS2,&tio2);

//   flush buffer
    ioctl(handle,TCFLSH,TCIOFLUSH);

    return true;
}

bool mySerial::IsOpen(void)
{
   return( handle >=0);
}

bool mySerial::Send (unsigned char  * data, int len)
{
   if (!this->IsOpen()) return false;
   int lenSEND = 0;
   while (lenSEND < len)
   {
      int rlen = write(handle, data + lenSEND, 1);
      lenSEND++;
   }
   return (lenSEND == len);
}

int  mySerial::Receive( unsigned char  * data, int len)
{
   if(!IsOpen()) return -1;

   // this is a blocking receives
   int lenRCV=0;
   while(lenRCV < len)
     {
       int rlen = read(handle,&data[lenRCV],len - lenRCV);
       lenRCV+=rlen;
     }
   return  lenRCV;
}

bool mySerial::NumberByteRcv(int &bytelen)
{
   if(!IsOpen()) return false;
   ioctl(handle, FIONREAD, &bytelen);
   return true;
}

