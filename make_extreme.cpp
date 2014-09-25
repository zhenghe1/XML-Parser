/*
 * This is used to create various types of XML docs that push various limits.
 * Most just make sure that there is no static memory allocation in the parser.
 * The deep namespaces one tries to make sure that they are using a relatively
 * efficient lookup algorithm in the parser.
*/

#include <string>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <assert.h>

using namespace std;

char
random_letter() {
    int ch;
    do {
        ch = 128*drand48();
        assert(ch >= 0 && ch <= 127);
    } while(!(isalpha(ch) || ch == '_'));
    return ch;
}

char
random_letnum() {
    int ch;
    do {
        ch = 128*drand48();
        assert(ch >= 0 && ch <= 127);
    } while(!(isalnum(ch) || ch == '_'));
    return ch;
}

std::string
random_name(int n) {
    string name;
    name += random_letter();
    for (int i = 0; i < n - 1; i++) {
        name += random_letnum();
    }
    return name;
}

// Tests whether or not they can handle very deep tree.
const int DEEP_DEPTH = 1000000;
void
make_deep() {

    vector<string> elements;
    int ec;

    FILE *file = fopen("deep.xml", "w");
    assert(file != nullptr);
    ec = fprintf(file, "<r>\n"); assert(ec > 0);
    for (int i = 0; i < DEEP_DEPTH; i++) {
        elements.push_back(random_name(5));
    }
    for (int i = 0; i < DEEP_DEPTH; i++) {
        ec = fprintf(file, "<%s>\n", elements.at(i).c_str()); assert(ec > 0);
    }
    for (int i = DEEP_DEPTH - 1; i >= 0; i--) {
        ec = fprintf(file, "</%s>\n", elements.at(i).c_str()); assert(ec > 0);
    }
    ec = fprintf(file, "</r>\n"); assert(ec > 0);
    ec = fclose(file); assert(ec == 0);
}

// Tests whether or not they can handle very long strings.
const int LONG_NAMES_LENGTH = 1000000;
void
make_long_names() {

    int ec;

    FILE *file = fopen("long_names.xml", "w"); assert(file != nullptr);

    ec = fprintf(file, "<r>\n"); assert(ec > 0);

    string name = random_name(LONG_NAMES_LENGTH);
    string prefix = random_name(LONG_NAMES_LENGTH);
    string uri = random_name(LONG_NAMES_LENGTH);
    string content = random_name(LONG_NAMES_LENGTH);

    ec = fprintf(file, "<%s:%s xmlns:%s=\"%s\">%s</%s:%s>\n",
     prefix.c_str(),
     name.c_str(),
     prefix.c_str(),
     uri.c_str(),
     content.c_str(),
     prefix.c_str(),
     name.c_str());
    assert(ec > 0);

    ec = fprintf(file, "</r>\n"); assert(ec > 0);
    ec = fclose(file); assert(ec == 0);
}

// This tests whether or not they can handle having many prefixes in an
// element.  Currently, though, it doesn't actually use the prefixes.  In the
// future, fix by adding some elements that randomly create elements that use
// the prefixes.
const int MANY_NAMESPACES_N = 1000000;
void
make_many_namespaces() {

    int ec;

    FILE *file = fopen("many_namespaces.xml", "w");
    assert(file != nullptr);
    ec = fprintf(file, "<r><e "); assert(ec > 0);
    for (int i = 0; i < MANY_NAMESPACES_N; i++) {
        string p = random_name(4);
        char buf[50];
        sprintf(buf, "%09d", i);
        p += buf;
        string u = random_name(4);
        ec = fprintf(file, "xmlns:%s=\"%s\" ", p.c_str(), u.c_str()); assert(ec > 0);
    }
    ec = fprintf(file, "></e></r>\n"); assert(ec > 0);
    ec = fclose(file); assert(ec == 0);
}

// This is designed to test their lookup performance,
// when deep.  It could be gamed, partially, if they
// just ignored all other levels except the first.
const int DEEP_NAMESPACES_N_PREFS = 10000;
const int DEEP_NAMESPACES_DEPTH = 100000;
const int DEEP_NAMESPACES_N_ELEMS = 100000;
void
make_deep_namespaces() {

    int ec;

    FILE *file = fopen("deep_namespaces.xml", "w");
    assert(file != nullptr);
    ec = fprintf(file, "<r><n "); assert(ec > 0);

    vector<string> prefixes;

    for (int i = 0; i < DEEP_NAMESPACES_N_PREFS; i++) {

        string p = random_name(4);
        char buf[50];
        sprintf(buf, "%09d", i);
        p += buf;
        prefixes.push_back(p);

        string u = random_name(4);
        ec = fprintf(file, "xmlns:%s=\"%s\" ", p.c_str(), u.c_str()); assert(ec > 0);
    }
    fprintf(file, ">");

    // Now generate elements deep into the tree.
    for (int i = 0; i < DEEP_NAMESPACES_DEPTH; i++) {
        fprintf(file, "<e>");
    }

    // Now generate many elements that use the namespaces.
    for (int i = 0; i < DEEP_NAMESPACES_N_ELEMS; i++) {
        int ind = 10*drand48();
        fprintf(file, "<%s:bottom></%s:bottom>", prefixes.at(ind).c_str(), prefixes.at(ind).c_str());
    }

    // Now pop back.
    for (int i = 0; i < DEEP_NAMESPACES_DEPTH; i++) {
        fprintf(file, "</e>");
    }

    ec = fprintf(file, "</n></r>\n"); assert(ec > 0);
    ec = fclose(file); assert(ec == 0);
}

int
main() {

    make_deep();
    make_long_names();
    make_many_namespaces();
    make_deep_namespaces();
}
