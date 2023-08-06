#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <openssl/sha.h>

struct SocketInfo {
    std::string timestamp;
    std::string protocol;
    std::string localAddr;
    std::string remoteAddr;
    std::string status;
    std::string processName;
    int processPID;
    int processPPID;
    std::string sha256;
    std::string user;
};

std::string getCurrentTimeUTC() {
    time_t now = time(nullptr);
    struct tm gmtm;
    gmtime_r(&now, &gmtm);
    char time_buf[50];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &gmtm);
    return std::string(time_buf);
}

std::string hashFileSHA256(const std::string& filename) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    char buffer[4096];
    int bytesRead = 0;

    int file = open(filename.c_str(), O_RDONLY);
    if (file < 0) return "";

    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        SHA256_Update(&sha256, buffer, bytesRead);
    }
    SHA256_Final(hash, &sha256);

    close(file);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << ((int)hash[i] & 0xff);
    }
    return ss.str();
}

// ... (More functions to gather info. This example does not gather all info for brevity.)

int main() {
    std::vector<SocketInfo> sockets;

    // TODO: Populate the sockets vector with info from /proc

    // Example of populating a single record:
    SocketInfo si;
    si.timestamp = getCurrentTimeUTC();
    si.protocol = "tcp";
    si.localAddr = "127.0.0.1:8080";
    si.remoteAddr = "0.0.0.0:*";
    si.status = "LISTEN";
    si.processName = "/usr/bin/app";
    si.processPID = 1234;
    si.processPPID = 123;
    si.sha256 = hashFileSHA256(si.processName);
    si.user = "root";

    sockets.push_back(si);

    // Print
    for (const auto& s : sockets) {
        std::cout << s.timestamp << "," << s.protocol << "," << s.localAddr << "," 
                  << s.remoteAddr << "," << s.status << "," << s.processName << "," 
                  << s.processPID << "," << s.processPPID << "," << s.sha256 << "," 
                  << s.user << std::endl;
    }

    return 0;
}
