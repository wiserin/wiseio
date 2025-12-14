#include <cstdint>  // Copyright 2025 wiserin
#include <iostream>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <vector>

#include "logging/logger.hpp"
#include "logging/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/buffer.hpp"


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

    // wiseio::Stream file = wiseio::CreateStream("test2.txt", wiseio::OpenMode::kReadAndWrite);

    // auto stream = wiseio::CreateStream("out.txt", wiseio::OpenMode::kWrite);
    //     wiseio::StringIOBuffer buffer;
        
    //     buffer.AddDataToBuffer(
    //         "# Configuration file\n"
    //         "setting1=value1\n"
    //         "# This is a comment\n"
    //         "setting2=value2\n"
    //         "setting3=value3 # inline comment\n"
    //     );
        
    //     stream.CWrite(buffer);


    auto stream = wiseio::CreateStream("out.txt", wiseio::OpenMode::kRead);
        wiseio::StringIOBuffer buffer;
        
        buffer.SetIgnoreComments(true);
        
        size_t file_size = stream.GetFileSize();
        buffer.ResizeBuffer(file_size);
        stream.CRead(buffer);
        
        buffer.SetCursor(0);

        std::cout << buffer.GetBufferSize() << std::endl;
        
        std::vector<std::string> lines;
        while (buffer.IsLines()) {
            std::string line = buffer.GetLine();
            if (!line.empty()) {
                // lines.push_back(line);
                std::cout << line << std::endl;
            }
        }


}