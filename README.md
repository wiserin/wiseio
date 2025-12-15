# WiseIO

A modern, high-performance C++ library for file I/O operations with smart buffering capabilities. WiseIO provides a clean API for reading, writing, and manipulating files with support for both binary and text data.

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/tests-passing-brightgreen.svg)](tests/)

## Features

- 🚀 **High Performance**: Uses low-level POSIX `pread`/`pwrite` for efficient I/O
- 📦 **Smart Buffering**: Built-in buffer classes for optimal memory management
- 🔄 **Flexible Reading**: Cursor-based and offset-based reading modes
- 📝 **Text Processing**: Line-by-line reading with comment filtering support
- 🔒 **Type Safety**: Modern C++23 with strong type checking
- 🧪 **Well Tested**: Comprehensive test suite with 90%+ coverage
- 🎯 **Zero Dependencies**: Only requires standard library and POSIX (except for logging)

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
  - [Stream](#stream)
  - [BytesIOBuffer](#bytesiobuffer)
  - [StringIOBuffer](#stringiobuffer)
- [Usage Examples](#usage-examples)
- [Building](#building)
- [Testing](#testing)
- [Contributing](#contributing)
- [License](#license)

## Installation

### Using CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    WiseIO
    GIT_REPOSITORY https://github.com/yourusername/wiseio.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(WiseIO)

target_link_libraries(your_target PRIVATE WiseIO)
```

### Manual Installation

```bash
git clone https://github.com/yourusername/wiseio.git
cd file-io
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

## Quick Start

### Reading a File

```cpp
#include <wise-io/stream.hpp>
#include <iostream>
#include <vector>

int main() {
    // Create a read stream
    auto stream = wiseio::CreateStream("input.txt", wiseio::OpenMode::kRead);
    
    // Read entire file
    std::vector<uint8_t> data;
    stream.ReadAll(data);
    
    std::cout << "Read " << data.size() << " bytes" << std::endl;
    return 0;
}
```

### Writing a File

```cpp
#include <wise-io/stream.hpp>
#include <vector>

int main() {
    auto stream = wiseio::CreateStream("output.txt", wiseio::OpenMode::kWrite);
    
    std::string message = "Hello, WiseIO!";
    stream.CWrite(message);
    
    return 0;
}
```

### Processing Text Line by Line

```cpp
#include <wise-io/stream.hpp>
#include <wise-io/buffer.hpp>
#include <iostream>

int main() {
    auto stream = wiseio::CreateStream("config.txt", wiseio::OpenMode::kRead);
    
    wiseio::StringIOBuffer buffer;
    buffer.SetIgnoreComments(true);  // Skip lines starting with #
    
    stream.ReadAll(buffer);
    buffer.SetCursor(0);
    
    while (buffer.IsLines()) {
        std::string line = buffer.GetLine();
        std::cout << line << std::endl;
    }
    
    return 0;
}
```

## API Reference

### Stream

The `Stream` class provides low-level file operations with automatic resource management.

#### Creating a Stream

```cpp
Stream CreateStream(const char* path, OpenMode mode);
```

**Open Modes:**
- `OpenMode::kRead` - Read-only mode
- `OpenMode::kWrite` - Write mode (creates file if doesn't exist)
- `OpenMode::kAppend` - Append mode
- `OpenMode::kReadAndWrite` - Read and write mode

#### Reading Methods

##### CRead - Cursor-based Reading
Reads data from current cursor position and advances the cursor.

```cpp
ssize_t CRead(std::vector<uint8_t>& buffer);
ssize_t CRead(IOBuffer& buffer);
ssize_t CRead(std::string& buffer);
```

**Example:**
```cpp
auto stream = wiseio::CreateStream("file.bin", wiseio::OpenMode::kRead);

std::vector<uint8_t> data(1024);  // Prepare buffer
ssize_t bytes_read = stream.CRead(data);  // Read up to 1024 bytes

std::cout << "Read " << bytes_read << " bytes" << std::endl;
```

##### CustomRead - Offset-based Reading
Reads data from a specific offset without affecting the cursor.

```cpp
ssize_t CustomRead(std::vector<uint8_t>& buffer, size_t offset);
ssize_t CustomRead(IOBuffer& buffer, size_t offset);
ssize_t CustomRead(std::string& buffer, size_t offset);
```

**Example:**
```cpp
std::vector<uint8_t> chunk(100);
stream.CustomRead(chunk, 500);  // Read 100 bytes starting from position 500
```

##### ReadAll - Read Entire File
Reads the entire file into a buffer.

```cpp
ssize_t ReadAll(std::vector<uint8_t>& buffer);
ssize_t ReadAll(IOBuffer& buffer);
ssize_t ReadAll(std::string& buffer);
```

**Example:**
```cpp
std::vector<uint8_t> entire_file;
stream.ReadAll(entire_file);  // Automatically resizes buffer
```

#### Writing Methods

##### CWrite - Cursor-based Writing
Writes data at current cursor position and advances the cursor.

```cpp
bool CWrite(const std::vector<uint8_t>& buffer);
bool CWrite(const IOBuffer& buffer);
bool CWrite(const std::string& buffer);
```

**Example:**
```cpp
auto stream = wiseio::CreateStream("output.bin", wiseio::OpenMode::kWrite);

std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
stream.CWrite(data);

std::string text = "Hello World\n";
stream.CWrite(text);
```

##### AWrite - Append Writing
Appends data to the end of file (requires `OpenMode::kAppend`).

```cpp
bool AWrite(const std::vector<uint8_t>& buffer);
bool AWrite(const IOBuffer& buffer);
bool AWrite(const std::string& buffer);
```

**Example:**
```cpp
auto log = wiseio::CreateStream("app.log", wiseio::OpenMode::kAppend);

log.AWrite("New log entry\n");
log.AWrite("Another entry\n");
```

##### CustomWrite - Offset-based Writing
Writes data at a specific offset.

```cpp
bool CustomWrite(const std::vector<uint8_t>& buffer, size_t offset);
bool CustomWrite(const IOBuffer& buffer, size_t offset);
bool CustomWrite(const std::string& buffer, size_t offset);
```

**Example:**
```cpp
// Overwrite specific region
std::vector<uint8_t> patch = {0xFF, 0xAA, 0xBB};
stream.CustomWrite(patch, 100);  // Write at position 100
```

#### Utility Methods

```cpp
void SetCursor(size_t position);      // Set cursor position
size_t GetFileSize() const;           // Get file size in bytes
bool IsEOF() const;                   // Check if reached end of file
void Close();                         // Manually close file
```

**Example:**
```cpp
size_t file_size = stream.GetFileSize();
std::cout << "File size: " << file_size << " bytes" << std::endl;

stream.SetCursor(0);  // Reset to beginning
if (stream.IsEOF()) {
    std::cout << "Reached end of file" << std::endl;
}
```

---

### BytesIOBuffer

Binary data buffer with cursor-based operations.

#### Methods

```cpp
class BytesIOBuffer {
public:
    // Buffer management
    void ResizeBuffer(size_t size);
    size_t GetBufferSize() const;
    uint8_t* GetDataPtr();
    const uint8_t* GetDataPtr() const;
    
    // Data operations
    void AddDataToBuffer(const std::vector<uint8_t>& data);
    std::vector<uint8_t> ReadFromBuffer(size_t size);
    
    // Cursor operations
    void SetCursor(size_t position);
    bool IsData() const;  // Check if data available at cursor
    
    // Cleanup
    void Clear();
};
```

#### Usage Example

```cpp
wiseio::BytesIOBuffer buffer;

// Add data
std::vector<uint8_t> chunk1 = {0x01, 0x02, 0x03};
std::vector<uint8_t> chunk2 = {0x04, 0x05, 0x06};
buffer.AddDataToBuffer(chunk1);
buffer.AddDataToBuffer(chunk2);

// Read data
buffer.SetCursor(0);
while (buffer.IsData()) {
    auto data = buffer.ReadFromBuffer(3);
    // Process data...
}

// Clear when done
buffer.Clear();
```

#### Protocol Handling Example

```cpp
// Write binary protocol: [length:4][data:length]
wiseio::BytesIOBuffer buffer;

std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o'};
uint32_t length = message.size();

// Add length header
buffer.AddDataToBuffer({
    static_cast<uint8_t>(length & 0xFF),
    static_cast<uint8_t>((length >> 8) & 0xFF),
    static_cast<uint8_t>((length >> 16) & 0xFF),
    static_cast<uint8_t>((length >> 24) & 0xFF)
});

// Add message
buffer.AddDataToBuffer(message);

// Write to stream
auto stream = wiseio::CreateStream("protocol.bin", wiseio::OpenMode::kWrite);
stream.CWrite(buffer);
```

---

### StringIOBuffer

Text buffer with line-based operations and comment filtering.

#### Methods

```cpp
class StringIOBuffer {
public:
    // Buffer management
    void ResizeBuffer(size_t size);
    size_t GetBufferSize() const;
    uint8_t* GetDataPtr();
    const uint8_t* GetDataPtr() const;
    
    // Text operations
    void AddDataToBuffer(const std::string& data);
    std::string GetLine();  // Read next line
    bool IsLines() const;   // Check if more lines available
    
    // Configuration
    void SetCursor(size_t position);
    void SetIgnoreComments(bool state);  // Ignore lines starting with #
    void SetIgnoreBlank(bool state);     // Ignore empty lines
    void SetEncoding(Encoding encoding); // UTF-8 or UTF-16
    
    // Utility
    size_t GetLen() const;  // Get length in characters
    void Clear();
};
```

#### Encoding

```cpp
enum class Encoding {
    kUTF_8 = 1,   // 1 byte per character (ASCII)
    kUTF_16 = 2   // 2 bytes per character
};
```

#### Usage Examples

##### Basic Line Reading

```cpp
wiseio::StringIOBuffer buffer;
buffer.AddDataToBuffer("Line 1\nLine 2\nLine 3\n");

buffer.SetCursor(0);
while (buffer.IsLines()) {
    std::string line = buffer.GetLine();
    std::cout << line << std::endl;
}
```

##### Configuration File Parsing

```cpp
auto stream = wiseio::CreateStream("config.ini", wiseio::OpenMode::kRead);

wiseio::StringIOBuffer buffer;
buffer.SetIgnoreComments(true);  // Skip # comments
buffer.SetIgnoreBlank(true);     // Skip empty lines

stream.ReadAll(buffer);
buffer.SetCursor(0);

while (buffer.IsLines()) {
    std::string line = buffer.GetLine();
    
    // Parse "key=value" format
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        std::cout << "Config: " << key << " = " << value << std::endl;
    }
}
```

##### Comment Filtering

The comment filter recognizes:
- Lines starting with `#` (after whitespace)
- Inline comments: `value # comment`

```cpp
buffer.SetIgnoreComments(true);
buffer.AddDataToBuffer(
    "# This is a comment\n"
    "data=value1\n"
    "# Another comment\n"
    "data2=value2 # inline comment\n"
);

buffer.SetCursor(0);
std::string line1 = buffer.GetLine();  // "data=value1"
std::string line2 = buffer.GetLine();  // "data2=value2 "
```

---

## Usage Examples

### Example 1: File Copy

```cpp
#include <wise-io/stream.hpp>

void copy_file(const char* source, const char* dest) {
    auto reader = wiseio::CreateStream(source, wiseio::OpenMode::kRead);
    auto writer = wiseio::CreateStream(dest, wiseio::OpenMode::kWrite);
    
    const size_t CHUNK_SIZE = 4096;
    
    while (!reader.IsEOF()) {
        std::vector<uint8_t> chunk(CHUNK_SIZE);
        ssize_t bytes_read = reader.CRead(chunk);
        
        if (bytes_read > 0) {
            writer.CWrite(chunk);
        }
    }
}
```

### Example 2: CSV Processing

```cpp
#include <wise-io/stream.hpp>
#include <wise-io/buffer.hpp>
#include <sstream>
#include <vector>

struct Person {
    std::string name;
    int age;
    std::string city;
};

std::vector<Person> parse_csv(const char* filename) {
    auto stream = wiseio::CreateStream(filename, wiseio::OpenMode::kRead);
    
    wiseio::StringIOBuffer buffer;
    buffer.SetIgnoreComments(true);
    buffer.SetIgnoreBlank(true);
    
    stream.ReadAll(buffer);
    buffer.SetCursor(0);
    
    std::vector<Person> people;
    
    // Skip header
    if (buffer.IsLines()) {
        buffer.GetLine();
    }
    
    while (buffer.IsLines()) {
        std::string line = buffer.GetLine();
        std::istringstream ss(line);
        
        Person person;
        std::string age_str;
        
        std::getline(ss, person.name, ',');
        std::getline(ss, age_str, ',');
        std::getline(ss, person.city, ',');
        
        person.age = std::stoi(age_str);
        people.push_back(person);
    }
    
    return people;
}
```

### Example 3: Log File Appending

```cpp
#include <wise-io/stream.hpp>
#include <chrono>
#include <iomanip>

class Logger {
    wiseio::Stream log_stream_;
    
public:
    Logger(const char* filename) 
        : log_stream_(wiseio::CreateStream(filename, wiseio::OpenMode::kAppend)) {}
    
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") 
            << "] " << message << "\n";
        
        log_stream_.AWrite(oss.str());
    }
};

int main() {
    Logger logger("app.log");
    
    logger.log("Application started");
    logger.log("Processing data...");
    logger.log("Application finished");
    
    return 0;
}
```

### Example 4: Binary Protocol

```cpp
#include <wise-io/stream.hpp>
#include <wise-io/buffer.hpp>

// Write binary message with length prefix
void write_message(const char* filename, const std::vector<uint8_t>& message) {
    auto stream = wiseio::CreateStream(filename, wiseio::OpenMode::kWrite);
    
    wiseio::BytesIOBuffer buffer;
    
    // Write length (4 bytes, little-endian)
    uint32_t length = message.size();
    buffer.AddDataToBuffer({
        static_cast<uint8_t>(length & 0xFF),
        static_cast<uint8_t>((length >> 8) & 0xFF),
        static_cast<uint8_t>((length >> 16) & 0xFF),
        static_cast<uint8_t>((length >> 24) & 0xFF)
    });
    
    // Write message
    buffer.AddDataToBuffer(message);
    
    stream.CWrite(buffer);
}

// Read binary message with length prefix
std::vector<uint8_t> read_message(const char* filename) {
    auto stream = wiseio::CreateStream(filename, wiseio::OpenMode::kRead);
    
    // Read length
    wiseio::BytesIOBuffer len_buffer;
    len_buffer.ResizeBuffer(4);
    stream.CRead(len_buffer);
    
    const uint8_t* len_ptr = len_buffer.GetDataPtr();
    uint32_t length = len_ptr[0] | (len_ptr[1] << 8) | 
                      (len_ptr[2] << 16) | (len_ptr[3] << 24);
    
    // Read message
    wiseio::BytesIOBuffer msg_buffer;
    msg_buffer.ResizeBuffer(length);
    stream.CRead(msg_buffer);
    
    msg_buffer.SetCursor(0);
    return msg_buffer.ReadFromBuffer(length);
}
```

### Example 5: Random Access File Editing

```cpp
#include <wise-io/stream.hpp>

void patch_file(const char* filename, size_t offset, 
                const std::vector<uint8_t>& new_data) {
    auto stream = wiseio::CreateStream(filename, wiseio::OpenMode::kReadAndWrite);
    
    // Read original data at offset
    std::vector<uint8_t> original(new_data.size());
    stream.CustomRead(original, offset);
    
    std::cout << "Original data: ";
    for (auto byte : original) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;
    
    // Write new data
    stream.CustomWrite(new_data, offset);
    
    std::cout << "Data patched at offset " << offset << std::endl;
}
```

---

## Building

### Requirements

- CMake 3.12 or higher
- C++23 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- POSIX-compliant system (Linux, macOS, BSD)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/file-io.git
cd WiseIO

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Install (optional)
sudo cmake --install .
```

### Build Options

```bash
# Debug build with sanitizers
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined" \
      ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build with tests
cmake -DBUILD_TESTS=ON ..
```

---

## Testing

WiseIO includes a comprehensive test suite with 90%+ code coverage.

### Running Tests

```bash
# Build with tests
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
cmake --build .

# Run all tests
ctest --output-on-failure

# Or run directly
./tests/wiseio_tests

# Run specific tests
./tests/wiseio_tests --gtest_filter=StreamReadTest*

# Run with verbose output
./tests/wiseio_tests --gtest_print_time=1
```

### Using the Test Script

```bash
# Run all tests
./run_tests.sh

# Clean build and run tests
./run_tests.sh --clean

# Run with coverage report
./run_tests.sh --coverage

# Run specific test suite
./run_tests.sh --filter "BytesBufferTest*"

# Verbose output
./run_tests.sh --verbose
```

### Coverage Report

```bash
# Generate coverage with lcov
./run_tests.sh --coverage

# View HTML report
firefox build/coverage_html/index.html
```

---

## Performance Considerations

### Best Practices

1. **Use ReadAll for small files**: More efficient than multiple reads
   ```cpp
   std::vector<uint8_t> data;
   stream.ReadAll(data);  // Single system call
   ```

2. **Choose appropriate buffer sizes**: Larger buffers reduce system calls
   ```cpp
   const size_t OPTIMAL_SIZE = 8192;  // 8KB typical optimal
   std::vector<uint8_t> buffer(OPTIMAL_SIZE);
   ```

3. **Use CustomRead/CustomWrite for random access**: Doesn't affect cursor state
   ```cpp
   // Multiple random reads without cursor management
   stream.CustomRead(chunk1, offset1);
   stream.CustomRead(chunk2, offset2);
   ```

4. **Reuse buffers**: Avoid repeated allocations
   ```cpp
   std::vector<uint8_t> buffer(4096);
   while (!stream.IsEOF()) {
       buffer.resize(4096);  // Reset size
       stream.CRead(buffer);
       process(buffer);
   }
   ```

---

## Error Handling

All methods that can fail return appropriate error indicators:

- **Reading methods**: Return `-1` on error, `0` on EOF, bytes read otherwise
- **Writing methods**: Return `false` on error, `true` on success
- **Constructor**: Throws `std::runtime_error` if file cannot be opened
- **Buffer methods**: Throw `std::out_of_range` for invalid positions

### Example Error Handling

```cpp
try {
    auto stream = wiseio::CreateStream("file.txt", wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> data(1024);
    ssize_t bytes = stream.CRead(data);
    
    if (bytes < 0) {
        std::cerr << "Read error occurred" << std::endl;
        return 1;
    } else if (bytes == 0) {
        std::cout << "End of file reached" << std::endl;
    } else {
        std::cout << "Read " << bytes << " bytes" << std::endl;
    }
    
} catch (const std::runtime_error& e) {
    std::cerr << "Failed to open file: " << e.what() << std::endl;
    return 1;
} catch (const std::out_of_range& e) {
    std::cerr << "Buffer error: " << e.what() << std::endl;
    return 1;
}
```

---

## Thread Safety

WiseIO is **not thread-safe** by design for performance reasons. If you need to access streams from multiple threads:

1. **Use external synchronization**:
   ```cpp
   std::mutex stream_mutex;
   
   void thread_safe_write(wiseio::Stream& stream, const std::string& data) {
       std::lock_guard<std::mutex> lock(stream_mutex);
       stream.CWrite(data);
   }
   ```

2. **Use separate streams per thread**:
   ```cpp
   // Each thread opens its own stream
   void worker(int thread_id) {
       auto stream = wiseio::CreateStream(
           ("output_" + std::to_string(thread_id) + ".txt").c_str(),
           wiseio::OpenMode::kWrite
       );
       stream.CWrite("Thread data");
   }
   ```

---

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- Uses [WiseLogging](https://github.com/wiserin/CPP-logging) for internal logging
- Inspired by Python's file I/O and buffer interfaces
- Built with modern C++ best practices

---

## Contact

**Author**: wiserin  
**Issues**: [https://github.com/wiserin/file-io/issues](https://github.com/wiserin/wiseio/issues)
