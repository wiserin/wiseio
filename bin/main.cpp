#include <iostream>  // Copyright 2025 wiserin
#include <sys/types.h>
#include <vector>

#include "logging/logger.hpp"
#include "logging/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"


const std::vector<uint8_t> kMagicBytes {0x48, 0x41, 0x4D, 0x4D, 0x49, 0x4E, 0x47, 0x01};

int main() {
    logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);

    wiseio::Stream file = wiseio::CreateStream("test2.txt", wiseio::OpenMode::kReadAndWrite);

    // std::vector<uint8_t> init_data = kMagicBytes;
    // init_data.resize(init_data.size() + 8, 0);

    std::vector<uint8_t> load_buffer;
    load_buffer.reserve(4095);
    file.CRead(load_buffer);
    file.CWrite(load_buffer);
    std::cout << "Size: " << load_buffer.size() << std::endl;

    // std::vector<uint8_t> buffer {'x', 'a', 'a', 'a'};
    // file.CWrite(init_data);
}

    // wiseio::Stream out_file = wiseio::CreateStream("out.txt", wiseio::OpenMode::kWrite);

    // std::vector<uint8_t> buffer;

    // buffer.reserve(4096);
    // buffer.reserve(1);
    // std::cerr << "Ok 1" << std::endl;

    // int state = file.Read(buffer);

    // for(uint8_t el: buffer) {
    //     std::cerr << el;
    // }

    // std::cerr << "Ok 2" << std::endl;

    // wiseio::IOBuffer out_buff;

    // std::cerr << "Ok 3" << std::endl;
    // out_buff.Add(buffer);

    // std::cerr << "Ok 4" << std::endl;

    // out_file.CustomWrite(buffer, 1);
    
    // out_file.Write(buffer);
    // out_file.Write(buffer);


    // std::string text(buffer.begin(), buffer.end());
    // std::cout << text << std::endl;