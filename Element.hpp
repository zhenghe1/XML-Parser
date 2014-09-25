#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <iostream>
#include <vector>
#include "Node.hpp"
#include "String.hpp"

namespace xml {
    class Visitor;
    class Element : public Node {
        friend class Parser;

        public:
            Element();
            ~Element();
            const String &name() const;
            const String &nmspace() const;
            const String &nmspaceId() const;
            size_t n_children() const;
            const Node *child(size_t i) const;
            void accept(Visitor *) const;
            static bool is_Element(const Node *);
            static const Element *to_Element(const Node *);

        private:
            const String *sName;
            const String *sNS;
            const String *sNSID;
            std::vector<Node *> elements;
    };
}

#include "Visitor.hpp"

#endif