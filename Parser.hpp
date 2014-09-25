#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <map>
#include <stack>
#include "Element.hpp"

namespace xml {
    class Parser {
        public:
            Parser();
            const Element* parse(const char *, size_t);
            void addE(Element *);
            void addT(Text *);
            void getParseStr(const char *);

        private:
            Element* root;
            String* parseStr;
            std::stack<Element*> eStack;
            std::stack<std::map<const String, String> *> nsStack;
            enum { START = 1, DOC, TAG, START_NS_NAME, START_NAME, START_TAG, TAG_NS, TAG_NS_L, TAG_NS_M, TAG_NS_R, TEXT, END_NS_NAME, END_NAME, END_TAG, DONE } state;
    };
}

#endif