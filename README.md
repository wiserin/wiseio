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
- 🗂️ **Structured Binary Files**: Chunk-based binary file parsing and compilation via `ByteFile`
- 🔢 **Numeric Views**: Endianness-aware integer serialization with `NumView`
- 💾 **Lazy Storage**: Memory-efficient data storage with optional disk caching via `Storage`
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
  - [ByteFile](#bytefile)
  - [Chunks](#chunks)
  - [Storage](#storage)
  - [NumView](#numview)
- [Usage Examples](#usage-examples)
- [Recommended Patterns](#recommended-patterns)
  - [File Wrapper Pattern](#file-wrapper-pattern)
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
    GIT_REPOSITORY https://github.com/wiserin/wiseio.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(WiseIO)

target_link_libraries(your_target PRIVATE WiseIO)
```

### Manual Installation

```bash
git clone https://github.com/wiserin/wiseio.git
cd wiseio
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

### Reading a Structured Binary File

```cpp
#include <wise-io/byte/bytefile.hpp>
#include <wise-io/byte/chunks.hpp>
#include <wise-io/byte/views.hpp>

int main() {
    wiseio::ByteFile<std::string> file("data.bin");

    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), "version");
    file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), "payload");

    file.InitChunksFromFile();

    wiseio::BaseChunk& version_chunk = file.GetAndLoadChunk("version");
    wiseio::NumView view(version_chunk.GetStorage().GetData());
    std::cout << "Version: " << view.GetNum<uint32_t>() << std::endl;

    return 0;
}
```

## API Reference

### Stream

The `Stream` class provides low-level file operations with automatic resource management.

#### Creating a Stream

```cpp
Stream CreateStream(const char* path, OpenMode mode, bool is_temp = false);
Stream CreateStream(const std::filesystem::path& path, OpenMode mode, bool is_temp = false);
```

**Open Modes:**
- `OpenMode::kRead` - Read-only mode
- `OpenMode::kWrite` - Write mode (creates file if doesn't exist)
- `OpenMode::kAppend` - Append mode
- `OpenMode::kReadAndWrite` - Read and write mode

**`is_temp` flag:** When `true`, the file is unlinked from the filesystem immediately after opening. The file descriptor remains valid for the lifetime of the `Stream` object, but the file will be automatically deleted when the stream is closed or destroyed. Useful for scratch files.

```cpp
// Temporary file — deleted when stream is destroyed
auto tmp = wiseio::CreateStream("/tmp/scratch.bin", wiseio::OpenMode::kWrite, true);
tmp.CWrite(std::string("temporary data"));
// file is automatically removed when tmp goes out of scope
```

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
size_t GetCursor() const;             // Get current cursor position
size_t GetFileSize() const;           // Get file size in bytes
bool IsEOF() const;                   // Check if reached end of file
bool IsOpen() const;                  // Check if the file descriptor is open
void SetDelete() const;               // Unlink the file from the filesystem
void Rename(std::string&& new_name);  // Rename the file (within the same directory)
void Close();                         // Manually close file
```

**Example:**
```cpp
size_t file_size = stream.GetFileSize();
std::cout << "File size: " << file_size << " bytes" << std::endl;

size_t pos = stream.GetCursor();
std::cout << "Current position: " << pos << std::endl;

stream.SetCursor(0);  // Reset to beginning
if (stream.IsEOF()) {
    std::cout << "Reached end of file" << std::endl;
}

if (stream.IsOpen()) {
    stream.Close();
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

### ByteFile

`ByteFile<T>` provides a high-level abstraction for structured binary files composed of typed chunks. It manages layout, indexing, lazy loading, and atomic recompilation of binary files.

The template parameter `T` must be hashable (usable as an `std::unordered_map` key). Common choices are `std::string` or a user-defined `enum class`.

```cpp
#include <wise-io/byte/bytefile.hpp>
#include <wise-io/byte/chunks.hpp>
```

#### Creating a ByteFile

```cpp
template <Hashable T = std::string>
class ByteFile {
public:
    ByteFile(const char* file_name);

    void AddChunk(std::unique_ptr<BaseChunk> chunk, T&& name);
    void AddChunk(std::unique_ptr<BaseChunk> chunk, const T& name);

    BaseChunk& GetChunk(const T& name);
    BaseChunk& GetAndLoadChunk(const T& name);

    void InitChunksFromFile();
    void Compile();
};
```

#### Methods

**`AddChunk(chunk, name)`** — Registers a chunk with a unique name. Chunks must be added in the order they appear in the file. Throws `std::logic_error` if the name is already taken.

**`GetChunk(name)`** — Returns a reference to the chunk by name without loading its data. Throws `std::logic_error` if the name is not found.

**`GetAndLoadChunk(name)`** — Returns a reference to the chunk and loads its data from disk into the chunk's `Storage`. Use this when you need to read the chunk's actual bytes.

**`InitChunksFromFile()`** — Scans the file sequentially, recording the offset and size of each chunk without loading its data. Must be called before `GetAndLoadChunk`.

**`Compile()`** — Rewrites the file by iterating through all chunks. Chunks whose `Storage` has been modified are serialized from memory; unchanged chunks are re-read from the original file. The original file is replaced atomically.

#### Example

```cpp
wiseio::ByteFile<std::string> file("records.bin");

file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), "count");
file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), "data");

file.InitChunksFromFile();

// Read the count field
wiseio::BaseChunk& count_chunk = file.GetAndLoadChunk("count");
wiseio::NumView count_view(count_chunk.GetStorage().GetData());
uint32_t count = count_view.GetNum<uint32_t>();
std::cout << "Count: " << count << std::endl;

// Modify the count and save
count_view.SetNum<uint32_t>(count + 1);
count_chunk.GetStorage().Commit();

file.Compile();
```

---

### Chunks

Chunks are the building blocks of a `ByteFile`. Each chunk represents a contiguous region within the binary file and knows how to initialize itself (record offset/size) and load its data.

#### BaseChunk Interface

```cpp
class BaseChunk {
public:
    virtual void Init(wiseio::Stream& stream) = 0;     // Record offset; advance stream cursor
    virtual void Load(wiseio::Stream& stream) = 0;     // Load data into Storage
    virtual std::vector<uint8_t> GetCompiledChunk() = 0; // Serialize to bytes
    virtual bool IsInitialized() = 0;

    virtual uint64_t GetOffset() = 0;   // Byte offset in the file
    virtual uint64_t GetSize() = 0;     // Size in bytes
    virtual Storage& GetStorage() = 0; // Access the chunk's data storage
};
```

#### NumChunk

Represents a fixed-size integer field (1, 2, 4, or 8 bytes).

```cpp
// Factory function
std::unique_ptr<BaseChunk> MakeNumChunk(NumSize size);
```

`NumSize` values:

| Enum | Bytes |
|------|-------|
| `NumSize::kUint8_t` | 1 |
| `NumSize::kUint16_t` | 2 |
| `NumSize::kUint32_t` | 4 |
| `NumSize::kUint64_t` | 8 |

**Example:**
```cpp
auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
// Represents a 4-byte integer field
```

#### ByteChunk

Represents a length-prefixed variable-size byte array. The prefix is a fixed-size integer (specified by `NumSize`) that holds the length of the following data.

```cpp
// Factory function
std::unique_ptr<BaseChunk> MakeByteChunk(
    NumSize len_num_size,
    Endianness num_endianess = Endianness::kLittleEndian
);
```

File layout for a `ByteChunk` with `NumSize::kUint32_t`:
```
[ 4 bytes: length ] [ length bytes: data ]
```

**Example:**
```cpp
// Little-endian 32-bit length prefix followed by payload
auto chunk = wiseio::MakeByteChunk(
    wiseio::NumSize::kUint32_t,
    wiseio::Endianness::kLittleEndian
);
```

#### ValidateChunk

Reads a fixed-size region and validates it against an expected byte sequence. Throws `std::logic_error` during `Init` if the bytes do not match. Useful for magic number / file signature checks.

```cpp
// Factory function
std::unique_ptr<BaseChunk> MakeValidateChunk(
    uint64_t size,
    std::vector<uint8_t>&& target_value
);
```

**Example:**
```cpp
// Verify a 4-byte magic number at the start of the file
auto magic = wiseio::MakeValidateChunk(4, {0x89, 0x50, 0x4E, 0x47});

wiseio::ByteFile<std::string> file("image.png");
file.AddChunk(std::move(magic), "magic");
file.InitChunksFromFile();  // throws if magic bytes don't match
```

#### Endianness

```cpp
enum class Endianness : uint8_t {
    kLittleEndian = 0,
    kBigEndian
};
```

---

### Storage

`Storage` is a lazy data container used by chunks to hold their raw bytes. It tracks whether the data has been modified and supports optional disk caching to free heap memory for large datasets.

```cpp
#include <wise-io/byte/storage.hpp>
```

#### Methods

```cpp
class Storage {
public:
    std::vector<uint8_t>& GetData();   // Access (and mark dirty) the data buffer
    bool IsChanged();                  // True if data has been modified or committed
    void Commit();                     // Flush data to a cache file and free heap memory

    static void SetCacheDir(std::string&& path);  // Set directory for cache files
};
```

**`GetData()`** — Returns a mutable reference to the underlying `std::vector<uint8_t>`. Calling this method marks the storage as dirty (`StorageState::kDirty`), meaning it will be written out during `Compile()`. If the storage was previously committed to disk, the data is transparently reloaded before being returned.

**`IsChanged()`** — Returns `true` if the storage is dirty or has been committed (i.e., differs from its initial clean state). `ByteFileEngine` uses this to decide which chunks need to be re-serialized during `Compile()`.

**`Commit()`** — Writes the current data to a temporary cache file and frees the heap buffer. The data remains accessible via `GetData()`, which will reload it from the cache file transparently. Useful when working with many large chunks that would otherwise exhaust memory.

**`SetCacheDir(path)`** — Sets the directory where `Commit()` stores its temporary files. Throws `std::runtime_error` if the path is not an existing directory.

#### Example

```cpp
// Configure cache directory (call once at startup)
wiseio::Storage::SetCacheDir("/tmp/wiseio_cache");

// ... load and work with chunks ...

wiseio::Storage& storage = file.GetAndLoadChunk("large_payload").GetStorage();

// Modify data
std::vector<uint8_t>& data = storage.GetData();
data[0] = 0xFF;

// Free heap memory while preserving the change
storage.Commit();

// data is still accessible — reloaded transparently from cache
std::vector<uint8_t>& reloaded = storage.GetData();
```

---

### NumView

`NumView` is a typed, endianness-aware view over a `std::vector<uint8_t>`. It provides templated `GetNum<T>()` and `SetNum<T>(value)` methods for reading and writing integers of any integral type.

```cpp
#include <wise-io/byte/views.hpp>
```

#### Constructor

```cpp
NumView(std::vector<uint8_t>& data, Endianness endianess = Endianness::kLittleEndian);
```

The view holds a reference to the provided vector. The vector must remain valid for the lifetime of the view, and its size must match `sizeof(T)` at the time `GetNum` or `SetNum` is called.

#### Methods

```cpp
template<typename T>
T GetNum();  // Deserialize integer from buffer (T must be integral)

template<typename T>
void SetNum(T num);  // Serialize integer into buffer (resizes buffer to sizeof(T))
```

#### Example

```cpp
std::vector<uint8_t> raw_bytes(4);

wiseio::NumView view(raw_bytes, wiseio::Endianness::kLittleEndian);

// Write a value
view.SetNum<uint32_t>(0xDEADBEEF);
// raw_bytes is now {0xEF, 0xBE, 0xAD, 0xDE}

// Read it back
uint32_t value = view.GetNum<uint32_t>();  // 0xDEADBEEF

// Works with any integral type
view.SetNum<uint16_t>(1000);
uint16_t small = view.GetNum<uint16_t>();  // 1000
```

#### Utility Functions

`NumView` is backed by two free functions available via `<wise-io/utils.hpp>`:

```cpp
template<Integral T>
T FromVector(const std::vector<uint8_t>& data, Endianness source_endian);

template<Integral T>
std::vector<uint8_t> ToVector(T num, Endianness target_endian);
```

These can be used directly when a full `NumView` is not needed.

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

### Example 6: Structured Binary File with ByteFile

```cpp
#include <wise-io/byte/bytefile.hpp>
#include <wise-io/byte/chunks.hpp>
#include <wise-io/byte/views.hpp>

// File layout:
//   [4 bytes] magic: 0xDEADBEEF
//   [4 bytes] version: uint32_t
//   [4 + N bytes] payload: uint32_t length prefix + N bytes data

void write_structured(const char* path) {
    auto stream = wiseio::CreateStream(path, wiseio::OpenMode::kWrite);

    // Write magic
    std::vector<uint8_t> magic = {0xEF, 0xBE, 0xAD, 0xDE};
    stream.CWrite(magic);

    // Write version = 1
    wiseio::NumView version_view(magic);  // reuse buffer
    version_view.SetNum<uint32_t>(1);
    stream.CWrite(magic);

    // Write payload with length prefix
    std::vector<uint8_t> payload = {'H', 'e', 'l', 'l', 'o'};
    uint32_t len = payload.size();
    std::vector<uint8_t> len_buf = wiseio::ToVector<uint32_t>(len, wiseio::Endianness::kLittleEndian);
    stream.CWrite(len_buf);
    stream.CWrite(payload);
}

void read_structured(const char* path) {
    wiseio::ByteFile<std::string> file(path);

    file.AddChunk(wiseio::MakeValidateChunk(4, {0xEF, 0xBE, 0xAD, 0xDE}), "magic");
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), "version");
    file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), "payload");

    file.InitChunksFromFile();  // throws if magic doesn't match

    wiseio::BaseChunk& ver = file.GetAndLoadChunk("version");
    wiseio::NumView view(ver.GetStorage().GetData());
    std::cout << "Version: " << view.GetNum<uint32_t>() << std::endl;

    wiseio::BaseChunk& pay = file.GetAndLoadChunk("payload");
    std::vector<uint8_t>& data = pay.GetStorage().GetData();
    std::cout << "Payload: " << std::string(data.begin(), data.end()) << std::endl;
}
```

### Example 7: Modifying a Structured File

```cpp
#include <wise-io/byte/bytefile.hpp>
#include <wise-io/byte/chunks.hpp>
#include <wise-io/byte/views.hpp>

void increment_version(const char* path) {
    wiseio::ByteFile<std::string> file(path);

    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), "version");
    file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), "data");

    file.InitChunksFromFile();

    // Load and modify the version field
    wiseio::BaseChunk& ver = file.GetAndLoadChunk("version");
    wiseio::Storage& storage = ver.GetStorage();
    wiseio::NumView view(storage.GetData());

    uint32_t current = view.GetNum<uint32_t>();
    view.SetNum<uint32_t>(current + 1);

    // Mark the storage as committed so Compile() picks it up
    storage.Commit();

    // Atomically rewrite the file
    file.Compile();
}
```

---

## Recommended Patterns

### File Wrapper Pattern

For production code, the recommended way to use `ByteFile` is to wrap it in a domain-specific class. This approach provides several benefits: the file layout is defined in one place, chunk names are type-safe enum values instead of strings, and the public interface exposes only meaningful operations rather than raw chunk access.

The pattern uses an `enum class` as the `ByteFile` key type, which is hashable by default and allows the compiler to catch typos at compile time.

```cpp
#include <wise-io/byte/bytefile.hpp>
#include <wise-io/byte/chunks.hpp>
#include <wise-io/byte/views.hpp>
#include <wise-io/byte/storage.hpp>

class MyFile {
    enum class Modules {
        kFirst = 0,
        kSecond,
        kThird
    };

    wiseio::ByteFile<Modules> file_;

public:
    explicit MyFile(const char* name)
        : file_(wiseio::ByteFile<Modules>(name))
    {
        // Define the file layout: order of AddChunk calls must match the on-disk layout
        file_.AddChunk(
            wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t),
            Modules::kFirst);
        file_.AddChunk(
            wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t),
            Modules::kSecond);
        file_.AddChunk(
            wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t),
            Modules::kThird);
    }

    // Scan the file and record offsets (must be called before any GetAndLoadChunk)
    void InitFromFile() {
        file_.InitChunksFromFile();
    }

    // Atomically rewrite the file with any pending changes
    void Compile() {
        file_.Compile();
    }

    // Load the first numeric field and return its storage
    wiseio::Storage& LoadFirstChunk() {
        return file_.GetAndLoadChunk(Modules::kFirst).GetStorage();
    }

    // Load the second numeric field and return its storage
    wiseio::Storage& LoadSecondChunk() {
        return file_.GetAndLoadChunk(Modules::kSecond).GetStorage();
    }

    // Access the variable-length payload storage without loading it
    // (useful for writing new data before Compile)
    wiseio::Storage& GetPayloadStorage() {
        return file_.GetChunk(Modules::kThird).GetStorage();
    }
};
```

#### Full Usage Example

```cpp
#include "my_file.hpp"  // the wrapper above

int main() {
    MyFile file("test3.bin");
    file.InitFromFile();

    // Read the first uint32_t field
    wiseio::Storage& st = file.LoadFirstChunk();
    std::vector<uint8_t>& buff = st.GetData();
    wiseio::NumView view(buff);

    uint32_t value = view.GetNum<uint32_t>();
    std::cout << "First field: " << value << std::endl;

    // Modify it
    view.SetNum<uint32_t>(250);

    // Commit frees heap memory while preserving the change for Compile()
    st.Commit();

    // Rewrite the file with the updated value
    file.Compile();

    return 0;
}
```

#### Why This Pattern Works Well

Using an `enum class` as the key type instead of `std::string` means the compiler rejects invalid chunk names at compile time. The wrapper class hides the `ByteFile` layout details and exposes a stable API — if the file format changes, only the wrapper needs updating. The `Commit()` call after modification freeing the heap buffer to keep memory usage predictable.

---

## Building

### Requirements

- CMake 3.12 or higher
- C++23 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- POSIX-compliant system (Linux, macOS, BSD)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/wiserin/wiseio.git
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

5. **Use `Storage::Commit()` for large chunk data**: When a `ByteFile` has many large chunks loaded simultaneously, call `Commit()` on chunks you are done modifying to free heap memory. The data will be reloaded transparently if accessed again.
   ```cpp
   wiseio::Storage::SetCacheDir("/tmp/wiseio_cache");
   
   // Load, modify, then commit to free memory
   wiseio::Storage& st = file.GetAndLoadChunk("large_data").GetStorage();
   // ... modify st.GetData() ...
   st.Commit();  // data flushed to cache; heap buffer freed
   ```

6. **Use `GetChunk` instead of `GetAndLoadChunk` when you only want to write**: If you intend to overwrite a chunk's contents entirely (not read the existing value), use `GetChunk` to avoid an unnecessary disk read.
   ```cpp
   wiseio::Storage& st = file.GetChunk("output_field").GetStorage();
   st.GetData() = new_bytes;  // write without loading old value
   ```

---

## Error Handling

All methods that can fail return appropriate error indicators:

- **Reading methods**: Return `-1` on error, `0` on EOF, bytes read otherwise
- **Writing methods**: Return `false` on error, `true` on success
- **Constructor**: Throws `std::runtime_error` if file cannot be opened
- **Buffer methods**: Throw `std::out_of_range` for invalid positions
- **Chunk Init**: `ValidateChunk` throws `std::logic_error` if bytes don't match expected value
- **ByteFile**: `AddChunk`, `GetChunk`, `GetAndLoadChunk` throw `std::logic_error` for duplicate or missing names
- **Storage**: `SetCacheDir` throws `std::runtime_error` for invalid directory; `Commit` throws if the cache stream cannot be created

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
} catch (const std::logic_error& e) {
    std::cerr << "Structured file error: " << e.what() << std::endl;
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