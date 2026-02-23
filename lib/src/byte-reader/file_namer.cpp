#include <string>  // Copyright 2025 wiserin

#include <wise-io/utils.hpp>


using str = std::string;

namespace wiseio {

str FileNamer::GetName() {
    str name = "__" + std::to_string(current) + ".bin";
    ++current;
    return name;
}

} // namespace wiseio
