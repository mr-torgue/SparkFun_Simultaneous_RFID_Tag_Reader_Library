/*
Date: 25-04-2024
Author: Folmer Heikamp
Board: Arduino Uno R4 Wifi
Requires:  UHF m6e nano shield

used for testing
*/


//Used for transmitting to the device
//If you run into compilation errors regarding this include, see the README
#include <SoftwareSerial.h>
#include <MemoryFree.h>
#include <pgmStrToRAM.h>

SoftwareSerial softSerial(2, 3); //RX, TX

//#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module
#include <mysparkfunlibrary.h>

RFID nano; //Create instance
uint16_t total = 0;
uint16_t success = 0;

void setup()
{
  Serial.begin(115200);

  while (!Serial);
  Serial.println();
  Serial.println("Initializing...");
  Serial.println(F("Press a key to start testing"));
}

// Runs test filter with the same result
void testData(byte *data, uint8_t &len, byte *expectedData, uint8_t expectedLen)
{
  testFilter(0, data, len, 0, expectedData, expectedLen);
}

// Tests if the filter returned is correct
void testFilter(uint8_t result, byte *data, uint8_t &len, uint8_t expectedResult, byte *expectedData, uint8_t expectedLen) 
{
  // test result
  if(result != expectedResult)
  {
    Serial.println(F("ERROR!!! Result mismatch"));
    Serial.print(F("Real result: "));
    Serial.println(result, HEX);
    Serial.print(F("Expected result: "));
    Serial.println(expectedResult, HEX);
  }
  else
  {
    Serial.print(F("Result matches: "));
    Serial.println(result, HEX);

    if(len != expectedLen)
    {
      Serial.println(F("ERROR!!! Length mismatch"));
      Serial.print(F("Real length: "));
      Serial.println(len);
      Serial.print(F("Expected length: "));
      Serial.println(expectedLen);
    }
    else 
    {
      Serial.print(F("Length matches: "));
      Serial.println(len);
      byte x;
      for (x = 0 ; x < len ; x++)
      {
        if(data[x] != expectedData[x]) 
        {
          break;
        }
      }
      if(x != len)
      {
        Serial.print(F("ERROR!!! Data mismatch at byte "));
        Serial.println(x);
        Serial.print(F("Real data: "));
        printBytes(data, x + 1);
        Serial.print(F("Expected result: "));
        printBytes(expectedData, x + 1);
      }
      else
      {
        Serial.print(F("Data matches: "));
        printBytes(data, len);
        success++;
      }
    }
  }
  // increase total and reset length
  total++;
  len = 0;
}


// this function tests as much code as possible:
// 1. compares the generation of requests with known values
// 2. parses known responses
// Testing with the RFID reader itself is more difficult since we don't know what tags it returns
void loop()
{
  Serial.println(F("Press key to start testing."));
  while (!Serial.available());
  int option = Serial.parseInt();

  // test filters
  byte data[255];
  ReadConfig config;
  TagFilter filter;
  uint8_t i = 0, result;

  // EPC TESTS
  byte epc[] = {0x98, 0x12, 0xAB};
  filter = nano.initEPCReadFilter(epc, sizeof(epc));

  // filter 1 - EPC standard
  byte expectedData1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB, 0x04, 0x00, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x04, expectedData1, sizeof(expectedData1));
  // filter 1b - no metadata and no multitag
  filter.isMultiselect = false;
  byte expectedData1b[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x04, expectedData1b, sizeof(expectedData1b));
  filter.isMultiselect = true;

  // filter 2 - EPC with invert
  filter.isInverse = true;
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x0c, expectedData1, sizeof(expectedData1));
  filter.isInverse = false;

  // filter 3 - EPC with action 5 and target 3
  filter.target = 3;
  filter.action = 5;
  byte expectedData3[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB, 0x03, 0x05, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x04, expectedData3, sizeof(expectedData3));

  byte longepc[] = {0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 
                    0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 
                    0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 
                    0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 
                    0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44}; // 320 bits
  filter = nano.initEPCReadFilter(longepc, sizeof(longepc));
  // filter 4 - EPC with extended length
  byte expectedData4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x01, 0x40, 
                          0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                          0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                          0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                          0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44,
                          0x11, 0x22, 0x33, 0x44, 0x11, 0x22, 0x33, 0x44, 
                          0x04, 0x00, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x24, expectedData4, sizeof(expectedData4));

  // filter 5 - EPC with extended length and inverse
  filter.isInverse = true;
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x2C, expectedData4, sizeof(expectedData4));
  filter.isInverse = false;

  // USER FILTERS
  byte userdata[] = {0x11, 0x22, 0x33, 0x44};
  filter = nano.initUserDataReadFilter(userdata, sizeof(userdata));
  //filter 6 - Userdata from start 0x00 and action is 1
  Serial.println(F("Filter 6: USER start at 0x00"));
  i = 0;
  filter.action = 0x01;
  byte expectedData6[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x11, 0x22, 0x33, 0x44, 0x04, 0x01, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x03, expectedData6, sizeof(expectedData6));

  //filter 7 - Userdata from start 0x00 and action is 1 with start address at 0x22 and inverse
  Serial.println(F("Filter 7: USER start at 0x22 with inverse"));
  i = 0;
  filter.isInverse = true;
  filter.start = 0x22;
  byte expectedData7[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x20, 0x11, 0x22, 0x33, 0x44, 0x04, 0x01, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x0B, expectedData7, sizeof(expectedData7));
  filter.isInverse = false;

  //filter 8 - Userdata with extended filter
  byte longuserdata[] = {0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                        0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                        0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                        0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB}; // 48 * 8 = 384 bits
  filter = nano.initUserDataReadFilter(longuserdata, sizeof(longuserdata));
  Serial.println(F("Filter 8: large user filter"));
  byte expectedData8[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
                          0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                          0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                          0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB,
                          0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 0xFE, 0x11, 0x22, 0x33, 0x44, 0xAB, 
                          0x04, 0x00, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x23, expectedData8, sizeof(expectedData8));

  // EPC LENGTH
  uint16_t length = 112;
  filter = nano.initEPCLengthReadFilter(length);
  //filter 9 - length 96
  Serial.println(F("Filter 9: EPC length"));
  byte expectedData9[] = {0x00, 0x70, 0x04, 0x00, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x06, expectedData9, sizeof(expectedData9));

  //filter 10 - length 96 with inverse
  Serial.println(F("Filter 10: EPC length with inverse"));
  filter.isInverse = true;
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x06, expectedData9, sizeof(expectedData9));
  filter.isInverse = false;

  //TID FILTERS
  //filter 11 - TID 0x8812AB start from 0x10 with action 3
  byte tid[] = {0x88, 0x12, 0xAB};
  filter = nano.initTIDReadFilter(tid, sizeof(tid));
  Serial.println(F("Filter 11: TID"));
  byte expectedData11[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18, 0x88, 0x12, 0xAB, 0x04, 0x03, 0x00};
  filter.action = 0x03;
  filter.start = 0x10;
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x02, expectedData11, sizeof(expectedData11));

  //filter 12 - TID with inverse  
  Serial.println(F("Filter 12: TID with invert"));
  filter.isInverse = true;
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x0A, expectedData11, sizeof(expectedData11));
  filter.isInverse = false;

  // Password FILTERS
  //filter 13 - password
  uint32_t password = 0x12345678;
  filter = nano.initPasswordFilter(password);
  Serial.println(F("Filter 13: password"));
  byte expectedData13[] = {0x12, 0x34, 0x56, 0x78, 0x04, 0x00, 0x00};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x05, expectedData13, sizeof(expectedData13));

  // filter 14 - test inverse and multiselect and metadata
  Serial.println(F("Filter 13: password without multiselect"));
  filter.isMultiselect = false;
  filter.isInverse = true;
  byte expectedData14[] = {0x12, 0x34, 0x56, 0x78};
  result = nano.constructFilterMsg(data, i, filter);
  testFilter(result, data, i, 0x05, expectedData14, sizeof(expectedData14));

  // TEST READ FUNCTIONS
  // TEST 0x22

  // Read 1 (0x22) - without filter and simple config
  config = nano.initStandardReadMultipleTagsOnceConfig();
  filter = nano.initEmptyFilter();
  Serial.println(F("Read 1"));
  byte expected[] = {0x88, 0x10, 0x00, 0x13, 0x07, 0xD0, 0x00, 0x57};
  nano.constructReadTagIdMultipleMsg(data, i, config, filter);
  testData(data, i, expected, sizeof(expected));

  // Read 2 (0x22) - with EPC filter
  filter = nano.initEPCReadFilter(epc, sizeof(epc));
  Serial.println(F("Read 2"));
  byte expected2[] = {0x88, 0x14, 0x00, 0x13, 0x07, 0xD0, 0x00, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB, 0x04, 0x00, 0x00};
  nano.constructReadTagIdMultipleMsg(data, i, config, filter);
  testData(data, i, expected2, sizeof(expected2));

  // Read 3 (0x22) - with EPC filter and different multiselect, searchflag, metadata and continuous enabled
  Serial.println(F("Read 3"));
  config.isContinuous = true;
  config.offtime = 0x1234;
  config.multiSelect = 0x85;
  config.searchFlag = 0x3333;
  config.metadataFlag = 0x1122;
  byte expected3[] = {0x85, 0x14, 0x33, 0x33, 0x07, 0xD0, 0x12, 0x34, 0x11, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB, 0x04, 0x00, 0x00};
  nano.constructReadTagIdMultipleMsg(data, i, config, filter);
  testData(data, i, expected3, sizeof(expected3));

  // Continuous Read 1 - standard
  config = nano.initStandardContinuousReadConfig();
  Serial.println(F("Continuous Read 1"));
  byte expectedCon1[] = {0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x05, 0x1B, 0x22, 0x88, 0x14, 0x05, 0x1B, 0x03, 0xE8, 0x00, 0xFA, 0x00, 0x57, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x18, 0x98, 0x12, 0xAB, 0x04, 0x00, 0x00};
  nano.constructMultiProtocolTagOpMsg(data, i, config, filter);
  testData(data, i, expectedCon1, sizeof(expectedCon1));

  // Read Data 1 - no filter
  filter = nano.initEmptyFilter();
  config = nano.initStandardReadTagDataOnce();
  Serial.println(F("Read Data 1: no filter"));
  byte expectedReadData1[] = {0x03, 0xE8, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00};
  nano.constructReadTagDataMsg(data, i, 0x00, 0x00, config, filter, 0x03E8);
  testData(data, i, expectedReadData1, sizeof(expectedReadData1));

  // Read Data 2 - filter 9812AB32FF00020605E401F6
  Serial.println(F("Read Data 2: with filter"));
  byte epc2[] = {0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  filter = nano.initEPCSingleReadFilter(epc2, sizeof(epc2));
  byte expectedReadData2[] = {0x03, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  nano.constructReadTagDataMsg(data, i, 0x00, 0x00, config, filter, 0x03E8);
  testData(data, i, expectedReadData2, sizeof(expectedReadData2));

  // Write EPC Tests
  // Write EPC 1 - no filter
  Serial.println(F("Write EPC 1: no filter"));
  filter = nano.initEmptyFilter();
  byte newEPC[] = {0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  byte expectedWriteEPC1[] = {0x07, 0xD0, 0x00, 0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  nano.constructWriteTagIdMsg(data, i, newEPC, sizeof(newEPC), filter);
  testData(data, i, expectedWriteEPC1, sizeof(expectedWriteEPC1));

  // Write EPC 2 - filter 
  byte oldEPC[] = {0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF4};
  byte newEPC2[] = {0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  filter = nano.initEPCWriteFilter(oldEPC, sizeof(oldEPC));
  byte expectedWriteEPC2[] = {0x03, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x60, 0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF4, 0x98, 0x12, 0xAB, 0x32, 0xFF, 0x00, 0x02, 0x06, 0x05, 0xE4, 0x01, 0xF6};
  nano.constructWriteTagIdMsg(data, i, newEPC2, sizeof(newEPC2), filter, 0x03E8);
  testData(data, i, expectedWriteEPC2, sizeof(expectedWriteEPC2));


  // Write Data 1 - no filter
  filter = nano.initEmptyFilter();
  byte data1[] = {0x00, 0xEE};
  byte expectedWriteData1[] = {0x03, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xEE};
  nano.constructWriteTagDataMsg(data, i, 0x03, 0x00, data1, sizeof(data1), filter, 0x03E8);
  testData(data, i, expectedWriteData1, sizeof(expectedWriteData1));

  // Write Data 2 - with filter
  byte oldEPC3[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  byte data2[] = {0xCC, 0xFF};
  filter = nano.initEPCWriteFilter(oldEPC3, sizeof(oldEPC3));
  byte expectedWriteData2[] = {0x03, 0xE8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x30, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xCC, 0xFF};
  nano.constructWriteTagDataMsg(data, i, 0x03, 0x00, data2, sizeof(data2), filter, 0x03E8);
  testData(data, i, expectedWriteData2, sizeof(expectedWriteData2));

  // print end of testing
  Serial.println(""); 
  Serial.print(F("END OF REQUEST TESTING: "));
  Serial.print(success);
  Serial.print(F("/"));
  Serial.println(total); 
  Serial.println("");

  Serial.println("START TESTING RESPONSE PARSER (check manually...)"); 
  uint16_t EPCLength = 16, embeddedDataLength;
  uint8_t EPC[EPCLength], tagTypeLength;
  uint16_t myDataLength = 16;
  uint8_t myData[myDataLength];
  // automatic testing is complicated, so test it manually
  /*
  Read once with tags, default metadata:
  1   E200001B6909019816706BBF    12-5-2024 15:59:32  -48 20
  2   E200001B6909019816806DEB    12-5-2024 15:59:32  -32 20
  3   E200001B6909019816906BBB    12-5-2024 15:59:32  -42 20
  4   EE00001B6909019817006996    12-5-2024 15:59:32  -56 20

  0xFF, 0x6C, 0x29, // header, length, opcode
  0x00, 0x00,       // response
  0x00, 0x57,       // metadata
  0x00, 0x04,       // count
  0x14,             // read count
  0xD0,             // rssi
  0x11,             // antenna id
  0x00, 0x00, 0x00, 0x09,  // timestamp
  0x05,             // protocol
  0x00, 0x80,       // bit length EPC
  0x30, 0x00,       // PC
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, // EPC
  0xBE, 0xEB,       // CRC
  0x14,             // [repeat from read count]
  0xE0, 
  0x11, 
  0x00, 0x00, 0x00, 0x0D, 
  0x05, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 
  0xED, 0x6E, 
  0x14, 
  0xD6, 
  0x11, 
  0x00, 0x00, 0x00, 0x12, 
  0x05, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x90, 0x6B, 0xBB, 
  0x5E, 0x5E, 
  0x14, 
  0xC8, 
  0x11, 
  0x00, 0x00, 0x00, 0x18, 
  0x05, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 
  0xD5, 0x02, 
  0x57, 0x4A // message checksum
  */
  uint8_t data29_1[] = {0xFF, 0x6C, 0x29, 0x00, 0x00, 0x00, 0x57, 0x00, 0x04, 0x14, 0xD0, 0x11, 0x00, 0x00, 0x00, 0x09, 0x05, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, 0xBE, 0xEB, 0x14, 0xE0, 0x11, 0x00, 0x00, 0x00, 0x0D, 0x05, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 0xED, 0x6E, 0x14, 0xD6, 0x11, 0x00, 0x00, 0x00, 0x12, 0x05, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x90, 0x6B, 0xBB, 0x5E, 0x5E, 0x14, 0xC8, 0x11, 0x00, 0x00, 0x00, 0x18, 0x05, 0x00, 0x80, 0x30, 0x00, 0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 0xD5, 0x02, 0x57, 0x4A};

  Serial.println(F("Read buffer 1: 4 results standard metadata")); 
  Response r(data29_1, sizeof(data29_1));
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;


  /*
  Read once with tags, default metadata + embedded data(TID)
  1   E200001B6909019816706BBF    E2 00 34 12 01 39 1C 00 00 7E D5 8A 0E 11 01 36 00 0D 5F FB FF FF DC 50 12-5-2024 16:01:15  -53 34
  2   EE00001B6909019817006996    E2 00 34 12 01 38 1C 00 00 7E D3 61 11 11 01 35 30 0D 5F FB FF FF DC 50 12-5-2024 16:01:15  -36 34
  3   E200001B6909019817106766    E2 00 34 12 01 35 1C 00 00 7E D1 31 12 11 01 32 00 0D 5F FB FF FF DC 50 12-5-2024 16:01:15  -45 15

  0xFF, 0xA0, 0x29,    // header, length, opcode
  0x00, 0x00,          // status
  0x00, 0xD7,          // metadata
  0x00, 0x03,          // number of tags
  0x0E,                // read count       
  0xDC,                // rssi
  0x11,                // antenna
  0x00, 0x00, 0x00, 0x12, // timestamp
  0x05,                // protocol
  0x00, 0xC0,          // embedded data length in bits
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x38, 0x1C, 0x00, 0x00, 0x7E, 0xD3, 0x61, 0x11, 0x11, 0x01, 0x35, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, // embedded data
  0x00, 0x80,         // size in bits of epc
  0x30, 0x00,         // PC
  0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 
  0xD5, 0x02, // EPC CRC
  0x0F,               // [go again]
  0xD3, 
  0x11, 
  0x00, 0x00, 0x00, 0x20, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x35, 0x1C, 0x00, 0x00, 0x7E, 0xD1, 0x31, 0x12, 0x11, 0x01, 0x32, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x10, 0x67, 0x66, 
  0x4C, 0x0D, 
  0x0E, 
  0xCB, 
  0x11, 
  0x00, 0x00, 0x00, 0x31, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x39, 0x1C, 0x00, 0x00, 0x7E, 0xD5, 0x8A, 0x0E, 0x11, 0x01, 0x36, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, 
  0xBE, 0xEB, 
  0xD7, 0x4E
  */
  Serial.println(F("Read buffer 2: 3 results standard metadata + embedded data")); 
  uint8_t data29_2[] = {0xFF, 0xA0, 0x29, 0x00, 0x00, 0x00, 0xD7, 0x00, 0x03, 0x0E, 0xDC, 0x11, 0x00, 0x00, 0x00, 0x12, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x38, 0x1C, 0x00, 0x00, 0x7E, 0xD3, 0x61, 0x11, 0x11, 0x01, 0x35, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x80, 0x30, 0x00, 0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 0xD5, 0x02, 0x0F, 0xD3, 0x11, 0x00, 0x00, 0x00, 0x20, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x35, 0x1C, 0x00, 0x00, 0x7E, 0xD1, 0x31, 0x12, 0x11, 0x01, 0x32, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x10, 0x67, 0x66, 0x4C, 0x0D, 0x0E, 0xCB, 0x11, 0x00, 0x00, 0x00, 0x31, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x39, 0x1C, 0x00, 0x00, 0x7E, 0xD5, 0x8A, 0x0E, 0x11, 0x01, 0x36, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, 0xBE, 0xEB, 0xD7, 0x4E};
  r.parse(data29_2, sizeof(data29_2));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;

  /*
  Read once with tags, default metadata + GPIO, frequency, phase, protocol + embedded data(TID)
  1   E200001B6909019816706BBF    E2 00 34 12 01 39 1C 00 00 7E D5 8A 0E 11 01 36 00 0D 5F FB FF FF DC 50 12-5-2024 16:05:51  -44 45  GEN2    921250  174 IN: 1-L 2-L 3-L 4-L
  OUT: 1-L 2-L 3-L 4-L
  2   E200001B6909019816806DEB    E2 00 34 12 01 37 1C 00 00 7E D7 B6 0F 11 01 34 30 0D 5F FB FF FF DC 50 12-5-2024 16:05:51  -39 32  GEN2    921250  174 IN: 1-L 2-L 3-L 4-L
  OUT: 1-L 2-L 3-L 4-L        
  3   EE00001B6909019817006996    E2 00 34 12 01 38 1C 00 00 7E D3 61 11 11 01 34 00 0D 5F FB FF FF DC 50 12-5-2024 16:05:51  -48 46  GEN2    921250  174 IN: 1-L 2-L 3-L 4-L
  OUT: 1-L 2-L 3-L 4-L
  4   E200001B6909019817106766    E2 00 34 12 01 35 1C 00 00 7E D1 31 12 11 01 32 30 0D 5F FB FF FF DC 50 12-5-2024 16:05:51  -36 27  GEN2    921250  174 IN: 1-L 2-L 3-L 4-L
  OUT: 1-L 2-L 3-L 4-L

  0xFF, 0xEC, 0x29,       // header, length, opcode
  0x00, 0x00,             // status
  0x01, 0xFF,             // metadata
  0x00, 0x04,             // number of tags
  0x0C,                   // count
  0xDC,                   // rssi
  0x11,                   // antenna id
  0x0E, 0x0E, 0xA2,       // frequency
  0x00, 0x00, 0x00, 0x12, // timestamp
  0x00, 0xAE,             // phase
  0x05,                   // protocol
  0x00, 0xC0,             // embedded data length in bits
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x35, 0x1C, 0x00, 0x00, 0x7E, 0xD1, 0x31, 0x12, 0x11, 0x01, 0x32, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50,                   // embedded data
  0x00,                   // GPIO
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x10, 0x67, 0x66, 
  0x4C, 0x0D, 
  0x0C, 
  0xD9, 
  0x11, 
  0x0E, 0x0E, 0xA2, 
  0x00, 0x00, 0x00, 0x1F, 
  0x00, 0xAE, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x34, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x00, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 
  0xED, 0x6E, 
  0x0C, 
  0xD0, 
  0x11, 
  0x0E, 0x0E, 0xA2, 
  0x00, 0x00, 0x00, 0x2C, 
  0x00, 0xAE, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x38, 0x1C, 0x00, 0x00, 0x7E, 0xD3, 0x61, 0x11, 0x11, 0x01, 0x34, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x00, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 
  0xD5, 0x02, 
  0x0B, 
  0xD4, 
  0x11, 
  0x0E, 0x0E, 0xA2, 
  0x00, 0x00, 0x00, 0x3B, 
  0x00, 0xAE, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x39, 0x1C, 0x00, 0x00, 0x7E, 0xD5, 0x8A, 0x0E, 0x11, 0x01, 0x36, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x00, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, 
  0xBE, 0xEB, 
  0x14, 0x89 // checksum
  */

  uint8_t data29_3[] = {0xFF, 0xEC, 0x29, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x04, 0x0C, 0xDC, 0x11, 0x0E, 0x0E, 0xA2, 0x00, 0x00, 0x00, 0x12, 0x00, 0xAE, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x35, 0x1C, 0x00, 0x00, 0x7E, 0xD1, 0x31, 0x12, 0x11, 0x01, 0x32, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x10, 0x67, 0x66, 0x4C, 0x0D, 0x0C, 0xD9, 0x11, 0x0E, 0x0E, 0xA2, 0x00, 0x00, 0x00, 0x1F, 0x00, 0xAE, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x34, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 0xED, 0x6E, 0x0C, 0xD0, 0x11, 0x0E, 0x0E, 0xA2, 0x00, 0x00, 0x00, 0x2C, 0x00, 0xAE, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x38, 0x1C, 0x00, 0x00, 0x7E, 0xD3, 0x61, 0x11, 0x11, 0x01, 0x34, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x00, 0x80, 0x30, 0x00, 0xEE, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x17, 0x00, 0x69, 0x96, 0xD5, 0x02, 0x0B, 0xD4, 0x11, 0x0E, 0x0E, 0xA2, 0x00, 0x00, 0x00, 0x3B, 0x00, 0xAE, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x39, 0x1C, 0x00, 0x00, 0x7E, 0xD5, 0x8A, 0x0E, 0x11, 0x01, 0x36, 0x00, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x70, 0x6B, 0xBF, 0xBE, 0xEB, 0x14, 0x89};
  Serial.println(F("Read buffer 3")); 
  r.parse(data29_3, sizeof(data29_3));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;

  /*
  Nothing found
  0xFF, 0x0B, 0x22,   // header, length, opcode
  0x00, 0x00,         // status
  0x88,               // multiselect flag
  0x00,               // optionbyte
  0x05,               // protocol
  0x1F, 0x02, 0x82, 0x00, 0x82, 0x00, 0x01, 0x2B, 
  0xDA, 0x19          // checksum
  */

  uint8_t data22_1[] = {0xFF, 0x0B, 0x22, 0x00, 0x00, 0x88, 0x00, 0x05, 0x1F, 0x02, 0x82, 0x00, 0x82, 0x00, 0x01, 0x2B, 0xDA, 0x19};
  Serial.println(F("Read multiple 1:"));
  r.parse(data22_1, sizeof(data22_1));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  /*
  Nothing found but slightly different
  0xFF, 0x0B, 0x22, 
  0x00, 0x00, 
  0x88, 
  0x00, 
  0x05, 
  0x1F, 0x02, 0x82, 0x00, 0x82, 0x00, 0x01, 0x2C, 0xDA, 0x19
  */

  uint8_t data22_2[] = {0xFF, 0x0B, 0x22, 0x00, 0x00, 0x88, 0x00, 0x05, 0x1F, 0x02, 0x82, 0x00, 0x82, 0x00, 0x01, 0x2C, 0xDA, 0x19};
  Serial.println(F("Read multiple 2:"));
  r.parse(data22_2, sizeof(data22_2));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;

  /*
  Also nothing found but opion byte seems to be on...
  Wrong status, should not set anything besides status
  0xFF, 0x0B, 0x22, 
  0x04, 0x00,         // status code not correct 
  0x88, 
  0x10,
  0x05, 0x1F, 0x00, 0x10, 0x01, 0x00, 0x00, 0x03, 0xF8, 0x91, 0x87
  */
  uint8_t data22_3[] = {0xFF, 0x0B, 0x22, 0x04, 0x00, 0x88, 0x10, 0x05, 0x1F, 0x00, 0x10, 0x01, 0x00, 0x00, 0x03, 0xF8, 0x91, 0x87};
  Serial.println(F("Read multiple 3:"));
  r.parse(data22_3, sizeof(data22_3));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;

  /*
  With embedded data and GPIO
  0xFF, 0x64, 0x22,   // header, length, opcode
  0x00, 0x00,         // status
  0x88,               // multiselect flag
  0x10,               // option byte
  0x05,               // protocol 
  0x1F,               // ??           
  0x01, 0xD7,         // metadata (gpio & embedded)
  0x01,               // Number of tags
  0x01,               // counnt
  0xC8,               // rssi
  0x11,               // antenna id
  0x00, 0x00, 0x00, 0xA3, // timestamp
  0x05,               // protocol
  0x02, 0x00,          // embedded data
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 
  0x00, 0x80,         // epc length bits
  0x30, 0x00,         // PC
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB,               // EPC
  0xED, 0x6E,         // checksum EPC
  0xC2, 0x35          // checksum
  */
  uint8_t data22_4[] = {0xFF, 0x64, 0x22, 0x00, 0x00, 0x88, 0x10, 0x05, 0x1F, 0x01, 0xD7, 0x01, 0x01, 0xC8, 0x11, 0x00, 0x00, 0x00, 0xA3, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 0xED, 0x6E, 0xC2, 0x35};
  Serial.println(F("Read multiple 4:"));
  r.parse(data22_4, sizeof(data22_4));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());  // print how much RAM is available in bytes.

  /*
  Standard, without embedded data.
  0xFF, 0x21, 0x22, 0x00, 0x00, 0x88, 0x10, 0x05, 0x1B, 0x00, 0x57, 0x01, 0x01, 0xCA, 0x11, 0x00, 0x00, 0x00, 0x53, 0x05, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x90, 0x6B, 0xBB, 0x5E, 0x5E, 0x20, 0x71
  */

  uint8_t data22_5[] = {0xFF, 0x21, 0x22, 0x00, 0x00, 0x88, 0x10, 0x05, 0x1B, 0x00, 0x57, 0x01, 0x01, 0xCA, 0x11, 0x00, 0x00, 0x00, 0x53, 0x05, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x90, 0x6B, 0xBB, 0x5E, 0x5E, 0x20, 0x71};
  Serial.println(F("Read multiple 5:"));
  r.parse(data22_5, sizeof(data22_5));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());  // print how much RAM is available in bytes.

  /*
  with filter and embedded data and filter (gpio as well)
  0xFF, 0x3C, 0x22, 
  0x00, 0x00, 
  0x88, 
  0x14, 
  0x05, 
  0x1F,       
  0x01, 0xD7, 
  0x01, 
  0x01, 
  0xE0, 
  0x11, 
  0x00, 0x00, 0x00, 0x15, 
  0x05, 
  0x00, 0xC0, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x33, 0x70, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 
  0x00, 0x80, 
  0x30, 0x00, 
  0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 
  0xED, 0x6E, 
  0xA4, 0x5A
  */
  uint8_t data22_6[] = {0xFF, 0x3C, 0x22, 0x00, 0x00, 0x88, 0x14, 0x05, 0x1F, 0x01, 0xD7, 0x01, 0x01, 0xE0, 0x11, 0x00, 0x00, 0x00, 0x15, 0x05, 0x00, 0xC0, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x33, 0x70, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x00, 0x00, 0x80, 0x30, 0x00, 0xE2, 0x00, 0x00, 0x1B, 0x69, 0x09, 0x01, 0x98, 0x16, 0x80, 0x6D, 0xEB, 0xED, 0x6E, 0xA4, 0x5A};
  Serial.println(F("Read multiple 6:"));
  r.parse(data22_6, sizeof(data22_6));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());  // print how much RAM is available in bytes.

  /*
  inspect tag E200001B6909019816806DEB    12-5-2024 17:19:36  -32 37
  Spread out over multip reads (per bank)

  0xFF, 0x43, 0x28,   // header, length, opcode
  0x00, 0x00,         // status code 
  0x11,               // option byte (?)
  0x00, 0x00,         // data size (0 means all)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // data
  0x9D, 0x55          // crc

  0xFF, 0x1B, 0x28, // header, length, opcode
  0x00, 0x00,       // status code 
  0x11, 
  0x00, 
  0x00, 
  0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x34, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 
  0x2C, 0x13
  */
  uint8_t data28_1[] = {0xFF, 0x43, 0x28, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D, 0x55};
  Serial.println(F("Read data 1"));
  r.parse(data28_1, sizeof(data28_1));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());  // print how much RAM is available in bytes.

  uint8_t data28_2[] = {0xFF, 0x1B, 0x28, 0x00, 0x00, 0x11, 0x00, 0x00, 0xE2, 0x00, 0x34, 0x12, 0x01, 0x37, 0x1C, 0x00, 0x00, 0x7E, 0xD7, 0xB6, 0x0F, 0x11, 0x01, 0x34, 0x30, 0x0D, 0x5F, 0xFB, 0xFF, 0xFF, 0xDC, 0x50, 0x2C, 0x13};
  Serial.println(F("Read data2:"));
  r.parse(data28_2, sizeof(data28_2));
  Serial.print(F("Nr of Tags: "));
  Serial.println(r.nrTags);
  Serial.print(F("Msg Length "));
  Serial.println(r.msgLength);
  Serial.print(F("Status "));
  Serial.println(r.status);
  for(uint8_t i = 0; i < r.nrTags; i++)
  {
    r.getMetadata(i, myData, myDataLength, embeddedDataLength, tagTypeLength);
    r.getData(i, EPC, EPCLength, 4);
    Serial.print(F("EPC: "));
    printBytes(EPC, EPCLength);
    r.printMetadata(i);
    Serial.println("");
  }
  EPCLength = 16;
  myDataLength = 64;
  Serial.println(F("Free RAM = ")); //F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());  // print how much RAM is available in bytes.
  /*
  Error message
  FF  00  28  // header, length, opcode
  04  23  // status code
  25  89  // crc
  */
  uint8_t data28_3[] = {0xFF, 0x00, 0x28, 0x04, 0x23, 0x25, 0x89};



  // empty buffer
  while(Serial.available() > 0) Serial.read();
}