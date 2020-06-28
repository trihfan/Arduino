#include <IRLibAll.h>
#include <IRLib_HashRaw.h>

//#define DEBUG 1
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261
#define SONY_OUTPUT_MIN_DELAY 300

#define TCL_KEY_POWER 0X6A68351E
#define TCL_KEY_VOL_UP 0x22D912BB
#define TCL_KEY_VOL_DOWN 0X776C6E7A

#define SONY_KEY_POWER 21516
#define SONY_KEY_VOL_UP 9228
#define SONY_KEY_VOL_DOWN 25612

IRrecv receiver(11);
IRsendRaw sender;
IRsendSony senderSony;
long lastTime = 0;
uint16_t rawData[64];

int compare (unsigned int oldval,  unsigned int newval)
{
  if      (newval < oldval * .8)  return 0 ;
  else if (oldval < newval * .8)  return 2 ;
  else                            return 1 ;
}

long decode()
{
  long  hash = FNV_BASIS_32;
  for (int i = 1; (i + 2) < recvGlobal.decodeLength; i++) 
  {
    int value =  compare(recvGlobal.decodeBuffer[i], recvGlobal.decodeBuffer[i + 2]);
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  return hash;
}

void convert(long hash)
{  
  resend();
  
  digitalWrite(5, HIGH);
  switch (hash)
  {
    case TCL_KEY_POWER:
      senderSony.send(SONY_KEY_POWER, 15);
      break;
    case TCL_KEY_VOL_UP:
      senderSony.send(SONY_KEY_VOL_UP, 15);
      break;
    case TCL_KEY_VOL_DOWN:
      senderSony.send(SONY_KEY_VOL_DOWN, 15);
      break;
  }
  digitalWrite(5, LOW);

  if (hash == TCL_KEY_VOL_UP or hash == TCL_KEY_VOL_DOWN)
  {
    resend();
  }
}

void resend()
{
  digitalWrite(6, HIGH); 
  memcpy(rawData, recvGlobal.recvBuffer, sizeof(uint16_t) * recvGlobal.recvLength);
  rawData[recvGlobal.recvLength] = 1000;
  sender.send(recvGlobal.recvBuffer, recvGlobal.recvLength, 38);
  digitalWrite(6, LOW);
  delay(10);
}

void setup() 
{
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  pinMode(2, OUTPUT);
  receiver.enableIRIn();
}

void loop()
{
  // Receive
  if (receiver.getResults())
  {
    // Get IR code
    long hash = decode();

    #ifdef DEBUG
    //Serial.println(hash, HEX);
    #endif

    switch (hash)
    {
    case TCL_KEY_POWER:
    case TCL_KEY_VOL_UP:
    case TCL_KEY_VOL_DOWN:
      // If time interval is ok
      if (millis() - lastTime > SONY_OUTPUT_MIN_DELAY)
      {
        convert(hash);
        lastTime = millis();
      }
      break;
      
    default:
      resend();
      break;
    }
    delay(20);
  }
  receiver.enableIRIn();
}
