#include <iostream>
#include <string.h>
#include "String.hpp"

xml::String::String() {
    str = NULL;
    len = 0;
}

xml::String::String(const char *s, int l) {
    str = s;
    len = l;
}

xml::String::String(const String &s) {
    str = s.str;
    len = s.len;
}

xml::String::operator std::string() const {
    return std::string(str, len);
}

void xml::String::append(int i) {
    len += i;
}

int xml::String::size() {
    return len;
}

bool xml::String::operator==(const String &s) const {
    return strncmp(str, s.str, len) == 0;
}

bool xml::String::operator<(const String &s) const {
    return strncmp(str, s.str, len) < 0;
}

bool operator==(const std::string &s1, const xml::String &s2) {
    return (std::string)s2 == s1;
}

bool operator==(const xml::String &s1, const std::string &s2) {
    return (std::string)s1 == s2;
}

bool operator!=(const std::string &s1, const xml::String &s2) {
    return (std::string)s2 != s1;
}

bool operator!=(const xml::String &s1, const std::string &s2) {
    return (std::string)s1 != s2;
}

bool operator==(const char *c, const xml::String &s) {
    return strncmp(s.str, c, s.len) == 0;
}

bool operator==(const xml::String &s, const char *c) {
    return strncmp(s.str, c, s.len) == 0;
}

bool operator!=(const char *c, const xml::String &s) {
    return strncmp(s.str, c, s.len) != 0;
}

bool operator!=(const xml::String &s, const char *c) {
    return strncmp(s.str, c, s.len) != 0;
}

std::ostream& operator<<(std::ostream &out, const xml::String &s) {
    out << std::string(s);
    return out;
}