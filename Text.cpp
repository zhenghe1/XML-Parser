#include "Text.hpp"

namespace xml {

Text::Text() {
	data = NULL;
}

Text::~Text() {
  	delete data;
}

bool Text::is_Text(const Node *n) {
  	return dynamic_cast<const Text *>(n) != NULL;
}

const String &Text::str() const {
  	return *data;
}

const Text *Text::to_Text(const Node *n) {
  	return dynamic_cast<const Text *>(n);
}
}