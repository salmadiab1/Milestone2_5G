#pragma once
#ifndef FUNCTIONS
#define FUNCTIONS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

//Global Variables 
const long long preamble = 0xFB555555555555;
const int SFD = 0xD5;
const int ethertype = 0xAEFE;
const int IFG = 0x07;
const int ECPRI_1stBYTE = 0x00; 
const int ECPRI_MSG = 0x00;
const int ECPRI_RTC = 0x0000;
const int ORAN_UP_1stBYTE = 0x00;


///////////////////////////////////////////////////////////////////////////////////////////////
class EtherPacket{
public:
    int lineRate;              // gigabit/sec 
    int captureSizeMs ;
    int minNumOfIFGs ;
    string destAddress ;
    string sourceAddress ;
    int maxPacketSize ;
    int burstSize ;
    int burstPeriodicity_us ;   //microsec
    
};

///////////////////////////////////////////////////////////////////////////////////////////////

class OranPacket : public EtherPacket {
public:
    int SCS;              
    int MaxNrb ;          
    int NrbPerPacket ;     
    string PayloadType ; 
    string Payload;
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes
bool readConfigFile(const string& fileName, OranPacket& oranPacket);
string trim(const string& str);
string removeHexPrefix(const string& hexValue);
string HexToString(long long value);
string crc32ToHexString(unsigned int crc);
string intToHex(int number) ;
void hexStringToByteArray(const string& hexStr, unsigned char* byteArray, int& length);
unsigned int calculateCRC32(const unsigned char* data, int length);
string ECPRI(const string& IQFilename, int payload_size, int& currentSample );

#endif 