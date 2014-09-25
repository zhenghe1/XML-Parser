#include <vector>
#include "Element.hpp"

namespace xml {

Element::Element() {
  sName = NULL;
  sNS = NULL;
  sNSID = NULL;
}

Element::~Element() {
  delete sName;
  delete sNS;
  delete sNSID;
  for(std::vector<Node*>::iterator it = elements.begin(); it != elements.end(); ++it) {
    delete *it;
  }
}

const String &Element::name() const {
  return *sName;
}

const String &Element::nmspace() const {
  return *sNS;
}

const String &Element::nmspaceId() const {
  return *sNSID;
}

size_t Element::n_children() const {
  return elements.size();
}

const Node *Element::child(size_t i) const {
  std::vector<Node *>::const_iterator it = elements.begin();
  it += i;
  return *it;
}

void Element::accept(Visitor *v) const {
  v->start_element_visit(*this);
  for(std::vector<Node *>::const_iterator it = elements.begin(); it != elements.end(); it++) {
    if(is_Element(*it)) {
      dynamic_cast<Element *>(*it)->accept(v);
    } else if(Text::is_Text(*it)) {
      v->visit_text(*dynamic_cast<Text *>(*it));
    }
  }
  v->end_element_visit(*this);
}

bool Element::is_Element(const Node *n) {
  return dynamic_cast<const Element*>(n) != NULL;
}

const Element* Element::to_Element(const Node *n) {
  return dynamic_cast<const Element*>(n);
}
}