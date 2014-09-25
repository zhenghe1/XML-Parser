#ifndef STRING_HPP
#define STRING_HPP

#include <string>

namespace xml { class String; }

bool operator==(const std::string &, const xml::String &);
bool operator==(const xml::String &, const std::string &);
bool operator!=(const std::string &, const xml::String &);
bool operator!=(const xml::String &, const std::string &);
bool operator==(const char *, const xml::String &);
bool operator==(const xml::String &, const char *);
bool operator!=(const char *, const xml::String &);
bool operator!=(const xml::String &, const char *);

namespace xml {
    class String {
        public:
            String();
            String(const char *, int);
            String(const String &);
            operator std::string() const;
            bool operator==(const String &) const;
            bool operator<(const String &) const;
            void append(int);
            int size();
            friend bool (::operator==)(const std::string &, const xml::String &);
            friend bool (::operator==)(const xml::String &, const std::string &);
            friend bool (::operator!=)(const std::string &, const xml::String &);
            friend bool (::operator!=)(const xml::String &, const std::string &);
            friend bool (::operator==)(const char *, const xml::String &);
            friend bool (::operator==)(const xml::String &, const char *);
            friend bool (::operator!=)(const char *, const xml::String &);
            friend bool (::operator!=)(const xml::String &, const char *);

        private:
            const char *str;
            int len;
    };
}

std::ostream &operator<<(std::ostream &, const xml::String &);

#endif