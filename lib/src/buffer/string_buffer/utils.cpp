#include <cstddef>  // Copyright 2025 wiserin
#include <cctype>
#include <vector>

#include "wise-io/buffer.hpp"


namespace wiseio {


bool StringIOBuffer::Validate(std::vector<char>& line) const {
    if (ignore_blank_) {
        if (IsBlank(line)) {
            return false;
        }
    }
    if (ignore_comments_) {
        if (CommentChecker(line)) {
            return false;
        }
    }
    return true;
}



bool StringIOBuffer::IsBlank(const std::vector<char>& line) const {
    for (unsigned char el : line) {
        if (!std::isspace(el)) {
            return false;
        } 
    }
    return true;
}


void StringIOBuffer::DeleteComment(std::vector<char>& line) const {
    std::vector<char> temp;

    for (char el : line) {
        if (el == '\0') {
            break;
        } else {
            temp.push_back(el);
        }
    }

    line = std::move(temp);
}


bool StringIOBuffer::CommentChecker(std::vector<char>& line) const {
    bool is_prev_space = true;
    bool is_comment = false;
    bool is_symbol = false;

    for (int i = 0; i < line.size(); ++i) {
        if (std::isspace(line[i])) {
            is_prev_space = true;
        } else if (line[i] == '#' && is_prev_space) {
            line[i] = '\0';
            is_comment = true;
            break;
        } else {
            is_symbol = true;
            is_prev_space = false;
        }
    }

    if (is_comment && is_symbol) {
        DeleteComment(line);
        return false;
    } else if (is_comment) {
        return true;
    } else {
        return false;
    }
}


} // namespase wiseio