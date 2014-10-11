/*
   bft.cpp - A brainfuck -> C transpiler
 */

#include <fstream>
#include <istream>
#include <iostream>
#include <iterator>
#include <vector>
#include <unistd.h>

enum class Indent { inc, dec, none };

std::string array = "array";
std::string celln = "30000";
std::string ptr   = "ptr";
std::string type  = "unsigned char";
size_t indentsize = 4;

void emit_line(std::vector<std::string> &output, std::string line, 
        Indent indent_update = Indent::none)
{
    static size_t indent = 0;

    if (indent_update == Indent::dec)
        indent -= indentsize;

    std::string add = "";
    for (size_t i = 0; i < indent; ++i)
        add += " ";
    add += line;
    output.push_back(add);

    if (indent_update == Indent::inc)
        indent += indentsize;
}

std::vector<std::string>* gen_c_code(std::vector<unsigned char> &input)
{
    auto out = new std::vector<std::string>();

    /* Default program start */
    emit_line(*out, "int main(void) {", Indent::inc);
    emit_line(*out, type + " " + array + "[" + celln + "]" + " = {0};");
    emit_line(*out, type + " " + "*" + ptr + " = " + array + ";");
    emit_line(*out, "");

    /* Parse input contents here */
    for (size_t i = 0; i < input.size(); ++i) {
        int jump = 0;

        switch (input[i]) {
        case '>':
        case '<':
            while (i < input.size() && (input[i] == '>' || input[i] == '<'))
                jump += input[i++] == '>' ? 1 : -1;
            if (i < input.size() && (input[i] != '<' || input[i] != '>')) 
                i--;

            emit_line(*out, ptr + " += " + std::to_string(jump) + ";");
            break;
        case '+':
        case '-':
            while (i < input.size() && (input[i] == '+' || input[i] == '-'))
                jump += input[i++] == '+' ? 1 : -1;
            if (i < input.size() && (input[i] != '<' || input[i] != '>')) 
                i--;

            emit_line(*out, "*" + ptr + " += " + std::to_string(jump) + ";");
            break;
        case '.':
            emit_line(*out, "putchar(*" + ptr + ");");
            break;
        case ',':
            emit_line(*out, "{", Indent::inc);
            emit_line(*out, "char c = getchar();");
            emit_line(*out, "if (c != -1) {", Indent::inc); /* Ignore EOF */
            emit_line(*out, "*" + ptr + " = c;");
            emit_line(*out, "}", Indent::dec);
            emit_line(*out, "}", Indent::dec);
            break;
        case '[':
            emit_line(*out, "while (*" + ptr + ") {", Indent::inc);
            break;
        case ']':
            emit_line(*out, "}", Indent::dec);
            break;
        }
    }

    /* Default program end */
    emit_line(*out, "return 0;");
    emit_line(*out, "}", Indent::dec);
    return out;
}

int main(int argc, char **argv)
{
    int c;
    int val;
    while ((c = getopt(argc, argv, "hi:t:c:")) != -1) {
        switch (c) {
        case 'h':
            std::cout << "Usage: bft <options> <input.bf>" << std::endl;
            return 1;
        case 'i':
            val    = std::stoi(std::string(optarg));
            indentsize = val <= 8 ? val : 8;
            break;
        case 't':
            type = std::string(optarg);
            break;
        case 'c':
            celln = std::string(optarg);
            break;
        }
    }

    if (optind >= argc) {
        std::cout << "Invalid file" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[optind]);
    std::istream_iterator<unsigned char> start(infile), end;
    std::vector<unsigned char> v(start, end);
    auto output = gen_c_code(v);

    for (auto i : *output)
        std::cout << i << std::endl;

    return 0;
}
