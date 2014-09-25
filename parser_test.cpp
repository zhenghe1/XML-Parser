#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include "Parser.hpp"
#include "Element.hpp"
#include "Text.hpp"
#include "Visitor.hpp"
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <new>
#include <execinfo.h>
#include <stdio.h>
#include <regex.h>
#include <cxxabi.h>

using namespace xml;
using namespace std;



bool mem_trace = false;



#ifdef CS540_MEMTRACE



regex_t pat;

void comp_pat() __attribute__((constructor(101)));
void free_pat() __attribute__((destructor(101)));

void
comp_pat() {
    // This is the pattern for the system-supplied backtrace.
    const char *const pat_str = 
     "^([^(]*\\()"// Before and including the left paren.
     "([^)+]*)" // Everything up to ) or +.
     "(.*)$"; // Everything after that.
    /*
    // This is the pattern for the backtrace in the code by Jeff Muizelaar.
    // String looks like: Element.cpp:55  _ZNK3xml7Element6acceptEPNS_7VisitorE()
    // Don't know whether or not there can be something in the parantheses.
    const char *const pat_str =
     "^([^[:space:]]+[[:space:]]+)" // Sequence of nonspace then space.
     "([[:alnum:]_]+)" // One or more identifier characters.
     "(\\(.*\\))$"; // Currently looks like ().
    */
     
    int ec = regcomp(&pat, pat_str, REG_EXTENDED); assert(ec == 0);
}

void
free_pat() {
    regfree(&pat);
}

void
stack_trace(size_t sz) {

    int ec;

    if (mem_trace) {

        fprintf(stderr, "Allocating %zu bytes, stack trace:\n", sz);

        // Grab the addresses.
        void *bt_buf[1000];
        int n = backtrace(bt_buf, 1000); assert(n >= 1);
        if (n == 1000) {
            fprintf(stderr, "    STACK TRACE TRUNCATED TO 1000!\n");
        }

        // Lookup the symbols.
        char **syms = backtrace_symbols(bt_buf, n); assert(syms != 0);

        // Now print out the stack, skip the first one, since it is
        // stack_trace() itself.
        for (int i = 1; i < n; i++) {

            // printf("symbol: %s\n", syms[i]);

            // First match is whole string, second is prefix, 3rd is mangled
            // name, 4th is suffix.
            regmatch_t matches[4];
            ec = regexec(&pat, syms[i], 4, matches, 0);

            if (ec == 0) {

                // If match succeeded, then demangle.

                regmatch_t &the_match(matches[2]); assert(the_match.rm_so != -1);
                size_t mangled_name_len =  the_match.rm_eo - the_match.rm_so;
                const char *mangled_name_begin = syms[i] + the_match.rm_so;
                char *mangled_name = (char *) malloc(mangled_name_len + 1);
                strncpy(mangled_name, mangled_name_begin, mangled_name_len);
                mangled_name[mangled_name_len] = '\0';
                //printf("mangled name: %s\n", mangled_name);

                // Demangle  extracted mangled name.
                const char *demangled_name = abi::__cxa_demangle(mangled_name, 0, 0, &ec);
                // Assume that it is either empty or C name.
                assert((ec == -2 && demangled_name == 0)
                 || (ec == 0 && demangled_name != 0));

                fprintf(stderr, "    %.*s\"%s\"%.*s\n", 
                 matches[1].rm_eo - matches[1].rm_so, syms[i] + matches[1].rm_so,
                 // If demangling failed, just print the mangled name.
                 demangled_name ? demangled_name : mangled_name,
                 matches[3].rm_eo - matches[3].rm_so, syms[i] + matches[3].rm_so);

                free(mangled_name);
                free((void *) demangled_name);

            } else {

                // If match fails, just print the symbol as is.
                fprintf(stderr, "    %s\n", syms[i]);
            }
        }

        free(syms);
    }
}

void *
operator new(size_t sz) throw (std::bad_alloc) {
    stack_trace(sz);
    void *vp = malloc(sz);
    if (vp == 0) {
        throw std::bad_alloc();
    }
    return vp;
}

void *
operator new(size_t sz, const std::nothrow_t &) throw () {
    stack_trace(sz);
    return malloc(sz);
}

void *
operator new[] (std::size_t sz) throw (std::bad_alloc) {
    stack_trace(sz);
    void *vp = malloc(sz);
    if (vp == 0) {
        throw std::bad_alloc();
    }
    return vp;
}

void *
operator new[] (std::size_t sz, const std::nothrow_t &) throw() {
    stack_trace(sz);
    return malloc(sz);
}

void
operator delete(void *vp) throw () {
    free(vp);
}

void
operator delete(void *vp, const std::nothrow_t &) throw () {
    free(vp);
}

void
operator delete[](void *vp) throw () {
    free(vp);
}
void
operator delete[] (void *vp, const std::nothrow_t &) throw() {
    free(vp);
}



#endif



// Escape strings for clearer printing and debugging.
std::string
escape_string(const std::string &in) {

    std::string s;
    // Escaped string won't be bigger than double the original size.
    s.reserve(2*s.size());

    for (std::string::const_iterator it = in.begin(); it < in.end(); ++it) {
        switch (*it) {
            case '\n':
                s.append("\\n");
                break;
            case '\r':
                s.append("\\r");
                break;
            case '\t':
                s.append("\\t");
                break;
            case '\f':
                s.append("\\f");
                break;
            case '\v':
                s.append("\\v");
                break;
            case '"':
                s.append("\\\"");
                break;
            default:
                s.push_back(*it);
                break;
        }
    }

    return s;
}

class MyVisitor : public xml::Visitor {
    public:
        MyVisitor(bool ind_f) : indent(0), indent_flag(ind_f) {}
        virtual void start_element_visit(const xml::Element &);
        virtual void end_element_visit(const xml::Element &);
        virtual void visit_text(const xml::Text &);
    private:
        size_t indent;
        const bool indent_flag;
};

void
MyVisitor::start_element_visit(const xml::Element &e) {
    string is(indent_flag ? string(indent, ' ') : "");
    cout << is << "Starting visit of element " << e.nmspace() << ":"
         << e.name() << "..." << endl;
    indent += 2;
}

void
MyVisitor::end_element_visit(const xml::Element &e) {
    indent -= 2;
    string is(indent_flag ? string(indent, ' ') : "");
    cout << is << "Ending visit of element " << e.nmspace() << ":"
         << e.name() << "." << endl;
}

void
MyVisitor::visit_text(const xml::Text &t) {
    string is(indent_flag ? string(indent, ' ') : "");
    cout << is << "Visiting text content \"" << escape_string(t.str()) << "\"." << endl;
}

void
traverse(const Node *n, size_t current_indent, bool indent_flag) {

    string indent(indent_flag ? string(current_indent, ' ') : "");

    if (Element::is_Element(n)) {

        const Element *e = Element::to_Element(n);
        cout << indent << "Start: " << e->nmspace() << ":" << e->name() << endl;
        for (size_t i = 0; i < e->n_children(); i++) {
            traverse(e->child(i), current_indent + 2, indent_flag);
        }
        cout << indent << "End: " << e->nmspace() << ":" << e->name() << endl;

    } else if (Text::is_Text(n)) {

        const Text *t = Text::to_Text(n);
        cout << indent <<  "Text: \"" << escape_string(t->str()) << "\"" << endl;

    } else {

        assert(false);
    }
}



int
main(int argc, char *argv[]) {

    // Make sure stdout is line buffered to keep stdout and stderr output synced.
    setlinebuf(stdout);

    int ec, opt, en;
    bool indent = true;
    bool correct_mode = true;
    bool performance_mode = false;
    int iteration = 1;
    char* infile = 0;
    if (argc < 3) {
        cerr << "USAGE: ./parser_test [-m] [-n] -f input_file_name options" << endl;
        cerr << "options :" << endl;
        cerr << "-m : Trace memory usage if compiled with -DCS540_MEMTRACE" << endl;
        cerr << "-n : Don't indent for every level" << endl;
        cerr << "-c : test correctness" << endl;
        cerr << "-p <#> : test performance and the number of times" << endl;
        cerr << "if there is no option, the default is test correctness" << endl;
        return 1;
    }

    while ((opt = getopt(argc, argv, "mnf:cp:")) != -1) {
        switch (opt) {
            case 'm':
                mem_trace = true;
                break;
            case 'n':
                indent = false;
                break;
            case 'f':
                infile = optarg;
                break;
            case 'c':
                correct_mode = true;
                performance_mode = false;
                break;
            case 'p':
                correct_mode = false;
                performance_mode = true;
                iteration = atoi(optarg);
                break;
            case '?':
                if(optopt == 'p')
                    cerr<<"option : "<<(char)optopt<<" needs a value"<<endl;
                else
                    cerr<<"Unknown option character : "<<(char)optopt<<endl;
                return 1;
                break;
        }
    }

    int fd = open(infile, O_RDONLY);
    if (fd < 0) {
        cerr << "Open file failed : " << strerror(errno) << endl;	
        return 1;
    }

    struct stat sb;
    ec = fstat(fd, &sb);
    if ( ec != 0) {
        cerr << "Get file information failed : " << strerror(errno)<<endl;
        return 1;
    }

    const char *doc = (const char *) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(doc == MAP_FAILED) {
        cerr << "mmap document failed : " << strerror(errno) << endl;
        return 1;
    }

    Parser parser;
    const Element *r;
    timeval start_tv, stop_tv;
    double dt;
    if(correct_mode == true) {
        r = parser.parse(doc, sb.st_size);
        traverse(r, 0, indent);
        MyVisitor v(indent);
        r->accept(&v);
        delete r;
    } else {
        r = parser.parse(doc, sb.st_size); // warm up
        delete r;
        ec = gettimeofday(&start_tv, 0);
        if (ec != 0) {
            cerr<<"gettimeofday failed : "<<strerror(errno)<<endl;
            return 1;
        }
        for(int i=0; i<iteration; i++) {
            r = parser.parse(doc, sb.st_size);
            delete r;
        }
        ec = gettimeofday(&stop_tv, 0);
        if (ec != 0) {
            cerr<<"gettimeofday failed : "<<strerror(errno)<<endl;
            return 1;
        }
        dt = (stop_tv.tv_sec + stop_tv.tv_usec/1000000.0)
                - (start_tv.tv_sec + start_tv.tv_usec/1000000.0);
        long long total_bytes = ((long long) iteration)*sb.st_size;
        printf("Elapsed time: %.6f,  Rate (MB/s): %f\n", dt, (total_bytes/(1024*1024))/dt);
    }

    return 0;
}
