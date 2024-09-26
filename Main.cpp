#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include<cmath>
#include "Header1.h"
using namespace std;

int ECPRI_SEQID = 0x00;
int frame_id = 0x00;
int Subframe_id = 0x0;
int slot_id = 0x0;
int sym_id = 0x00;
int startprbu =0 ;

int main() {

    string configFileName = "second_milestone.txt"; // Configuration file name     
    OranPacket oranPacket;
            // Read and parse the configuration file
            if (!readConfigFile(configFileName, oranPacket)) {
                cerr << "Configuration loading failed." << endl;
                return 1;
            }
            // Print the loaded configuration
            cout << "Line Rate: " << oranPacket.lineRate << endl;
            cout << "Capture Size (ms): " << oranPacket.captureSizeMs << endl;
            cout << "Min Number of IFGs: " << oranPacket.minNumOfIFGs << endl;
            cout << "Dest Address: " << oranPacket.destAddress << endl;
            cout << "Source Address: " << oranPacket.sourceAddress << endl;
            cout << "Max Packet Size: " << oranPacket.maxPacketSize << endl;
            cout << "SCS: " << oranPacket.SCS << endl;
            cout << "Max NRB: " << oranPacket.MaxNrb << endl;
            cout << "NrbPerPacket: " << oranPacket.NrbPerPacket << endl;
            cout << "PayloadType: " << oranPacket.PayloadType << endl;
            cout << "Payload: " << oranPacket.Payload << endl;


////////////////////////////////////////////////////////////////////////////////////////////////////    
//Calculations on data based on config file
long long Tot_Data_Bytes = (( oranPacket.lineRate * 1e9 ) / 8) * (oranPacket.captureSizeMs * 1e-3); 
cout<<"Total number of data bytes is "<<Tot_Data_Bytes<<" bytes"<<endl; 

int NO_OF_FRAMES = oranPacket.captureSizeMs / 10;       //based on capture size
cout<<"No of frames sent is  "<<NO_OF_FRAMES<<" frames"<<endl; 

int no_of_packets_per_sym = ceil (static_cast<double>(oranPacket.MaxNrb) /oranPacket.NrbPerPacket);
cout<<"No of packets per symbol is "<<no_of_packets_per_sym<<" Pack/sym"<<endl;

int no_packets_per_slot = no_of_packets_per_sym * 14 ;
cout<<"No of packets per slot is "<<no_packets_per_slot<<" Pack/slot"<<endl;

int no_slots_subframe = oranPacket.SCS /15 ;
cout<<"No of slots per subframe is "<<no_slots_subframe<<" slots/subframe"<<endl;

long long no_of_packets_per_frame = (no_packets_per_slot * no_slots_subframe) * 10 ;
cout<<"No of packets per frame is "<<no_of_packets_per_frame<<" packets/frame"<<endl;

long long frame_size =  no_of_packets_per_frame * oranPacket.maxPacketSize;
cout<<"Frame size in bytes is "<< frame_size << " bytes"<<endl;

long long No_of_IFGS = Tot_Data_Bytes - (NO_OF_FRAMES*frame_size) - (oranPacket.minNumOfIFGs *no_of_packets_per_frame *NO_OF_FRAMES);
cout<<"No of IFGS in bytes is "<< No_of_IFGS << " bytes"<<endl;

////////////////////////////////////////////////////////////////////////////////////////////////////    
    //Converting hex to string 
    string preamble_str = HexToString(preamble); 
    string SFD_str = HexToString(SFD);  
    string ethertype_str = HexToString(ethertype); 
    string IFG_str = HexToString(IFG); 
    string ECPRI_1stBYTE_str = HexToString(ECPRI_1stBYTE); 
    string ECPRI_MSG_str = HexToString(ECPRI_MSG); 
    string ECPRI_RTC_str = HexToString(ECPRI_RTC);
    string ORAN_UP_1stBYTE_str = HexToString(ORAN_UP_1stBYTE);
    string NRB_PERPACK_str = HexToString(oranPacket.NrbPerPacket);


//////////////////////////////////////////////////////////////////////////////////////////////////// 
    int i= 0;
    int paylaod_size = oranPacket.maxPacketSize - 26 - 15 ;
    string payload_size_str = HexToString(paylaod_size) ;
    int currentSample = 0; // Initialize current sample index

    // Open the output file
    ofstream outFile("output.txt");
    if (!outFile) {
        cerr << "Error opening file for writing." << endl;
        return 1;
    }  

    while (i<NO_OF_FRAMES){         //loop over on of frames
        
        //frame id of each packet
        string frame_id_str = HexToString(frame_id);
        frame_id++;

        for (int k =0; k< 10 ; k++){        //loop over subframes

            //Subframe id of each packet
            string Subframe_id_str = intToHex(k);

            for (int z=0 ;z< no_slots_subframe ; z++){    //loop over slots
                //Subframe id of each packet
                string Slot_id_str = intToHex(z);

                for (int x=0 ; x< 14 ; x++){    //loop over symbols
                    //Subframe id of each packet
                    string Sym_id_str = HexToString(x);

                    for (int y = 0 ; y< no_of_packets_per_sym ; y++){       //loop over no of packets per sym

                        //////////////////////////////////////////////////////////////////////////////////////////////////// 
                        string Payload_new = ECPRI(oranPacket.Payload, paylaod_size, currentSample ); 
                        string ECPRI_SEQID_str = HexToString(ECPRI_SEQID); 
                        ECPRI_SEQID++;
                        if (ECPRI_SEQID ==255){             //if seqid reaches 255 start to count from zero again
                            ECPRI_SEQID=0;
                        }
                        string startprbu_str = HexToString(startprbu); 
                        string ECPRI_HEADER = ECPRI_1stBYTE_str + ECPRI_MSG_str + payload_size_str + ECPRI_RTC_str + ECPRI_RTC_str +ECPRI_1stBYTE_str+ ECPRI_SEQID_str;
                        string ORAN_HEADER1 = ORAN_UP_1stBYTE_str + frame_id_str + Subframe_id_str + Slot_id_str + Sym_id_str ;
                        string ORAN_HEADER2 = ORAN_UP_1stBYTE_str + startprbu_str + NRB_PERPACK_str  ;

                        ////////////////////////////////////////////////////////////////////////////////////////////////////    
                        string Packet_No_CRC = preamble_str + SFD_str + oranPacket.destAddress + oranPacket.sourceAddress +
                                            ethertype_str + ECPRI_HEADER + ORAN_HEADER1 + ORAN_HEADER2 + Payload_new;

                        //Calculating CRC field 
                        // Allocate memory dynamically based on input size
                        int length ;
                        unsigned char* byteArray = new unsigned char[oranPacket.maxPacketSize];

                        // Convert hex string to byte array
                        hexStringToByteArray(Packet_No_CRC, byteArray, length);

                        // Calculate CRC-32
                        unsigned int crc = calculateCRC32(byteArray, length);

                        // Convert CRC to a hex string and display
                        string CRC_str = crc32ToHexString(crc);
                        cout << "CRC: 0x" << CRC_str << endl<<endl;

                        // Free dynamically allocated memory
                        delete[] byteArray;

                        string Packet_CRC = Packet_No_CRC + CRC_str  ;
                        //////////////////////////////////////////////////////////////////////////////////////////////////

                        int charCount = 0;
                            for (int i = 0; i < Packet_CRC.length(); i++) {  // Loop on packet size
                                outFile << Packet_CRC[i];
                                charCount++;

                                // Check if we need to start a new line every 8 bytes
                                if (charCount % 8 == 0) {
                                    outFile << endl;
                                    charCount = 0;
                                }
                                
                            }
                            
                            // If there are leftover bytes (less than 8), add a newline to maintain alignment
                            if (charCount > 0) {
                                outFile << endl;
                            }
                             
                            // IFGs
                            int charCount1 = 0;
                            for (int k = 0; k < oranPacket.minNumOfIFGs ; k++) {  // Loop for IFGs after each packet
                                outFile << IFG_str[0] << IFG_str[1];
                                charCount1 += 2;

                                // Check if we need to start a new line every 8 bytes
                                if (charCount1 % 8 == 0) {
                                    outFile << endl;
                                    charCount1 = 0;
                                }
                            }
                            
                            // If there are leftover bytes (less than 8), add a newline to maintain alignment
                            if (charCount1 > 0) {
                                outFile << endl;
                            }

                            //add paddding IFGs if the min IFGs are not 4 byte alligned
                            if (oranPacket.minNumOfIFGs % 4 != 0) {
                                int add_ifg = oranPacket.minNumOfIFGs;
                                // Continue adding IFG_str until add_ifg is a multiple of 4
                                while (add_ifg % 4 != 0) {
                                    outFile << IFG_str[0] << IFG_str[1];
                                    add_ifg++;
                                }
                            }


                    }
                    startprbu = 0;


                }
                
            }

        }
        //Extra IFGS AFTER EACH PACKETS PER FRAME
        // IFGs
        int charCount2 = 0;
        for (int k = 0; k < No_of_IFGS; k++) {  // Loop for IFGs
            outFile << IFG_str[0] << IFG_str[1];
            charCount2 += 2;

            // Check if we need to start a new line every 8 bytes
            if (charCount2 % 8 == 0) {
                outFile << endl;
                charCount2 = 0;
            }
        }
        
        // If there are leftover bytes (less than 8), add a newline to maintain alignment
        if (charCount2 > 0) {
            outFile << endl;
        }
        i++;
    }
            
    // Close the file
    outFile.close();

    return 0; 
}