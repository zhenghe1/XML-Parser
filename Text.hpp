#ifndef TEXT_HPP
#define TEXT_HPP

#include <iostream>
#include "Node.hpp"
#include "String.hpp"

namespace xml {
    class Text : public Node {
        friend class Parser;
        public:
            Text();
            ~Text();
            const String &str() const;
            static bool is_Text(const Node *);
            static const Text *to_Text(const Node *);
        private:
            String *data;
    };
}

#endif