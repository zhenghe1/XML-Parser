#include <stack>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Element.hpp"
#include "Parser.hpp"
#include "Text.hpp"

namespace xml {

Parser::Parser() {
    state = START;
    root = NULL;
    parseStr = NULL;
}

void Parser::addE(Element *e) {
    if(e->sNSID == NULL) {
        e->sNSID = new String();
        e->sNS = new String();
    } 
    else {
        e->sNS = new String((*nsStack.top())[*e->sNSID]);
    }
    if(eStack.size()) {
        eStack.top()->elements.push_back(e);
    }
    eStack.push(e);
    parseStr = NULL;
}

void Parser::addT(Text *t) {
    t->data = parseStr;
    if(eStack.size()) {
        eStack.top()->elements.push_back(t);
    }
    parseStr = NULL;
}

void Parser::getParseStr(const char *c) {
    if(parseStr == NULL) parseStr = new String(c, 0);
}

const Element* Parser::parse(const char *doc, size_t sz) {
    bool whitespace = false;
    state = START;
    Node* node = new Element();
    String* pStr; 

    for(unsigned int i = 0; i < sz; ++i) {
        const char c = doc[i];
        if(c == EOF) break;

        while(1) {
            try {
                switch(state) {
                    case START:
                        if(isspace(c)) {} 
                        else if(c == '<') {
                            root = dynamic_cast<Element *>(node);
                            state = TAG;
                        } 
                        else {
                            throw "Text not inside root";
                        }
                        break;

                    case DOC:
                        if(eStack.size() == 0) {
                            state = DONE;
                            continue;
                        } 
                        else if(c == '<') {
                            node = new Element();
                            state = TAG;
                        } 
                        else {
                            node = new Text();
                            state = TEXT;
                            continue;
                        }
                        break;

                    case TAG:
                        if(c == '/') {
                            state = END_NS_NAME;
                        } 
                        else {
                            state = START_NS_NAME;
                            std::map<const String, String> *map;
                            if(nsStack.size() == 0) {
                                map = new std::map<const String, String>;
                            } 
                            else {
                                map = new std::map<const String, String>(*nsStack.top());
                            }
                            nsStack.push(map);
                            continue;
                        }
                        break;

                    case START_NS_NAME:
                        getParseStr(doc + i);
                        if(c == ':' && parseStr->size() != 0) {
                            dynamic_cast<Element *>(node)->sNSID = parseStr;
                            parseStr = NULL;
                            state = START_NAME;
                        } 
                        else if(isspace(c) || c == '>') {
                            state = START_NAME;
                            continue;
                        } 
                        else if(isalnum(c) || c == '_') {
                            parseStr->append(1);
                        } 
                        else {
                            throw "Bad name or namespace.";
                        }
                        break;

                    case START_NAME:
                        getParseStr(doc + i);
                        if(isalnum(c) || c == '_') {
                            parseStr->append(1);
                        } 
                        else if(isspace(c) || c == '>') {
                            dynamic_cast<Element *>(node)->sName = parseStr;
                            parseStr = NULL;
                            state = START_TAG;
                            continue;
                        } 
                        else {
                            throw "Bad name.";
                        }
                        break;

                    case START_TAG:
                        if(isspace(c)) {} 
                        else if(isalnum(c) || c == '_') {
                            state = TAG_NS;
                            continue;
                        } 
                        else if(c == '>') {
                            addE((Element *)node);
                            state = DOC;
                        } 
                        else {
                            throw "Bad text.";
                        }
                        break;

                    case TAG_NS:
                        getParseStr(doc + i);
                        if(isalnum(c) || c == '_') {
                            parseStr->append(1);
                        } 
                        else if(c == ':' && *parseStr == "xmlns") {
                            delete parseStr;
                            parseStr = NULL;
                            state = TAG_NS_L;
                        } 
                        else {
                            throw "Bad text.";
                        }
                        break;

                    case TAG_NS_L:
                        getParseStr(doc + i);
                        if(isalnum(c) || c == '_') {
                            parseStr->append(1);
                        } 
                        else if(c == '=') {
                            pStr = parseStr;
                            parseStr = NULL;
                            state = TAG_NS_M;
                        } 
                        else {
                            throw "Bad text.";
                        }
                        break;

                    case TAG_NS_M:
                        if(c == '"') {
                            state = TAG_NS_R;
                        } 
                        else {
                            throw "Bad namespace.";
                        }
                        break;

                    case TAG_NS_R:
                        getParseStr(doc + i);
                        if(c != '"') {
                            parseStr->append(1);
                        } 
                        else if(c == '"') {
                           (*nsStack.top())[*pStr] = *parseStr;
                            delete pStr;
                            pStr = NULL;
                            delete parseStr;
                            parseStr = NULL;
                            state = START_TAG;
                        } 
                        else {
                            throw "Bad URI.";
                        }
                        break;

                    case END_NS_NAME:
                        getParseStr(doc + i);
                        if(c == ':' && parseStr->size() != 0 && eStack.top()->nmspaceId() == *parseStr) {
                            delete parseStr;
                            parseStr = NULL;
                            state = END_NAME;
                        } 
                        else if(isspace(c) || c == '>') {
                            state = END_NAME;
                            continue;
                        } 
                        else if(isalnum(c) || c == '_') {
                            parseStr->append(1);
                        } 
                        else {
                            throw "Bad end tag name or namespace.";
                        }
                        break;

                    case END_NAME:
                        getParseStr(doc + i);
                        if(isspace(c)) {} 
                        else if((isalnum(c) || c == '_') && !whitespace) {
                            parseStr->append(1);
                        } 
                        else if(isspace(c) || c == '>') {
                            state = END_TAG;
                            continue;
                        } 
                        else {
                            throw "Bad text.";
                        }
                        break;

                    case END_TAG:
                        if(isspace(c)) {}  
                        else if(c == '>' && eStack.top()->name() == *parseStr) {
                            eStack.pop();
                            delete nsStack.top();
                            nsStack.pop();
                            delete parseStr;
                            parseStr = NULL;
                            delete node;
                            node = NULL;
                            state = DOC;
                        } 
                        else {
                            throw "Bad end tag.";
                        }
                        break;

                    case TEXT:
                        if(eStack.size() == 0) throw "Text not inside root.";
                        getParseStr(doc + i);
                        if(c == '<') {
                            addT((Text *)node);
                            state = DOC;
                            continue;
                        } 
                        else if(c != '<' && c != '>') {
                            parseStr->append(1);
                        } 
                        else {
                            throw "Bad text.";
                        }
                        break;

                    case DONE:
                        if(!isspace(c)) throw "Text not inside root.";
                        break;
                }
            } 
            catch(const char *s) {
                std::cerr << s << std::endl;
                //exit(1);
            }
            whitespace = isspace(c);
            break;
        }
    }
    if(eStack.size() != 0) throw "Unclosed tags.";

    delete parseStr;
    parseStr = NULL;
    while(!nsStack.empty()) nsStack.pop();

    return root;
}
}