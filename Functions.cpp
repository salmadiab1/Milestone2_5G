#include "Header1.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
using namespace std;

extern int startprbu ;

/////////////////////////////////////////////////////////////////////////////////////////////////////
string HexToString(long long value) {
    stringstream ss;

    // Convert to hexadecimal and ensure we retain two digits if it's a single-digit hex value
    ss << hex << value;

    string hexStr = ss.str();

    // Check if the result is a single digit (i.e., has length 1) and add a leading zero if needed
    if (hexStr.length() % 2 != 0) {
        hexStr = "0" + hexStr;
    }

    return hexStr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to trim leading and trailing whitespace
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to remove the '0x' prefix from hexadecimal values
string removeHexPrefix(const string& hexValue) {
    if (hexValue.find("0x") == 0 || hexValue.find("0X") == 0) {
        return hexValue.substr(2);  // Remove the '0x' prefix
    }
    return hexValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to read and parse the configuration file
bool readConfigFile(const string& fileName, OranPacket& oranPacket) {
    ifstream configFile(fileName);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open config file" << endl;
        return false;
    }

    string line;
    while (getline(configFile, line)) {
        // Remove comments
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        // Remove leading and trailing whitespace
        line = trim(line);
        if (line.empty()) continue;  // Skip empty lines

        stringstream ss(line);
        string key;
        string value;

        getline(ss, key, '=');
        getline(ss, value);

        key = trim(key);
        value = trim(value);

        if (key == "Eth.LineRate") {
            oranPacket.lineRate = stoi(value);
        }
        else if (key == "Eth.CaptureSizeMs") {
            oranPacket.captureSizeMs = stoi(value);
        }
        else if (key == "Eth.MinNumOfIFGsPerPacket") {
            oranPacket.minNumOfIFGs = stoi(value);
        }
        else if (key == "Eth.DestAddress") {
            oranPacket.destAddress = removeHexPrefix(value);
        }
        else if (key == "Eth.SourceAddress") {
            oranPacket.sourceAddress = removeHexPrefix(value);
        }
        else if (key == "Eth.MaxPacketSize") {
            oranPacket.maxPacketSize = stoi(value);
        }
        else if (key == "Oran.SCS") {
                oranPacket.SCS = stoi(value);
        }
        else if (key == "Oran.MaxNrb") {
            oranPacket.MaxNrb = stoi(value);
        }
        else if (key == "Oran.NrbPerPacket") {
            oranPacket.NrbPerPacket = stoi(value);
        }
        else if (key == "Oran.PayloadType") {
            oranPacket.PayloadType = value;
        }
        else if (key == "Oran.Payload") {
            oranPacket.Payload = value;
        }
        
    }
    configFile.close();
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to calculate CRC-32 of a byte array
unsigned int calculateCRC32(const unsigned char* data, int length) {
    unsigned int crc = 0xFFFFFFFF; // Initial CRC value
    unsigned int polynomial = 0xEDB88320; // CRC-32 polynomial

    for (int i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF; // Final XOR value
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to convert a string of hex digits to a byte array
void hexStringToByteArray(const string& hexStr, unsigned char* byteArray, int& length) {
    length = hexStr.length() / 2; // 2 hex digits per byte
    for (int i = 0; i < length; ++i) {
        string byteString = hexStr.substr(i * 2, 2);
        byteArray[i] = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to return CRC as a hexadecimal string
string crc32ToHexString(unsigned int crc) {
    stringstream ss;
    ss << hex << setw(8) << setfill('0') << crc;
    return ss.str();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
string intToHex(int number) {
    stringstream ss;
    ss << hex << number; // Convert the integer to hexadecimal
    return ss.str();          // Return the hexadecimal string
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
string ECPRI(const string& IQFilename, int payload_size, int& currentSample ) {
    static vector<int8_t> I_samples; // Store I samples as bytes
    static vector<int8_t> Q_samples; // Store Q samples as bytes
    static bool isInitialized = false; // Flag to check if samples are loaded

    // Load samples from the file if not already loaded
    if (!isInitialized) {
        ifstream infile(IQFilename);
        if (!infile) {
            cerr << "Error opening file: " << IQFilename << endl;
            return "";
        }

        string line;
        while (getline(infile, line)) {
            istringstream iss(line);
            int I, Q;
            if (!(iss >> I >> Q)) {
                cerr << "Error reading line: " << line << endl;
                continue; // Skip invalid lines
            }

            // Store I and Q samples as separate bytes
            I_samples.emplace_back(static_cast<int8_t>(I));
            Q_samples.emplace_back(static_cast<int8_t>(Q));
        }

        // Close the file
        infile.close();
        isInitialized = true; // Mark samples as loaded
    }

    int totalSamples = I_samples.size();
    ostringstream payloadStream;

    // Calculate the number of samples we can fit in the payload
    int samplesToWrite = payload_size / 2; // Each sample consists of 2 bytes (I and Q)

    // Ensure we don't try to write more samples than we have
    if (samplesToWrite > totalSamples) {
        samplesToWrite = totalSamples;
    }

    // Loop to generate payload
    for (int i = 0; i < samplesToWrite; ++i) {
        // Ensure currentSample is within bounds
        if (currentSample >= totalSamples) {
            currentSample = 0; // Loop back to the beginning
        }

        // Use currentSample to get the I and Q samples
        payloadStream << hex << setw(2) << setfill('0') 
                      << static_cast<int>(I_samples[currentSample] & 0xFF)
                      << hex << setw(2) << setfill('0') 
                      << static_cast<int>(Q_samples[currentSample] & 0xFF);

        // Move to the next sample
        currentSample++;

        // Increment startprbu after processing 14 I and 14 Q samples (28 bytes)
        if (i % 28 == 27) { // Check if we processed 28 samples (14 I + 14 Q)
            startprbu += 30; // Increment by 30
        }
    }

    // Convert startprbu to a hex string of 4 digits
    stringstream startprbuStream;
    startprbuStream << hex << setw(4) << setfill('0') << (startprbu & 0xFFFF);
    string startprbuHex = startprbuStream.str(); // Store the hex string

    // Ensure the payload size is correct
    string payload = payloadStream.str();
    if (payload.size() > payload_size * 2) { // Each byte is represented by 2 hex characters
        payload = payload.substr(0, payload_size * 2); // Trim if it exceeds
    }


    return payload; // Return the payload as a hex string without new lines
}