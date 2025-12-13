#include <cstdint>
#include <iostream>  // Copyright 2025 wiserin
#include <ostream>
#include <string>
#include <sys/types.h>
#include <vector>

#include "logging/logger.hpp"
#include "logging/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"


std::ostream& operator<<(std::ostream& stream, std::vector<uint8_t> data) {
    for (uint8_t el : data) {
        std::cout << (char) el << ' ';
    }
    std::cout << '\n';
    return stream;
}



const std::vector<uint8_t> kMagicBytes {0x48, 0x41, 0x4D, 0x4D, 0x49, 0x4E, 0x47, 0x01};

int main() {
    logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);

    wiseio::Stream file = wiseio::CreateStream("test2.txt", wiseio::OpenMode::kReadAndWrite);

    std::string str;

    // std::cout << str << std::endl;
    // str.resize(100);

    // file.CRead(str);

    // std::cout << str.size() << std::endl;
    // std::cout << str << std::endl;

    // file.CustomWrite(str, 12);

    std::cout << file.GetFileSize() << std::endl;

    // // std::vector<uint8_t> init_data = kMagicBytes;
    // // init_data.resize(init_data.size() + 8, 0);

    // std::vector<uint8_t> load_buffer;
    // load_buffer.resize(3);
    // file.CustomRead(load_buffer, 4);

    // std::cout << "Size: " << std::to_string(load_buffer.size()) << std::endl;

    // for (uint8_t el : load_buffer) {
    //     std::cout << (int) el << ' ';
    // }
    // std::cout << '\n';

    // std::cout << load_buffer << std::endl;

}