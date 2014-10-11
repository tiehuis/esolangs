/*
   bft.cpp - A brainfuck -> C transpiler
 */

#include <fstream>
#include <istream>
#include <iostream>
#include <iterator>
#include <vector>

const std::string array = "array";
const std::string celln = "30000";
const std::string ptr   = "ptr";
const std::string type  = "char";
const size_t indentsize = 4;
 
void emit_line(std::vector<std::string> &output, std::string line, int indent_update = 0)
{
    static size_t indent = 0;

    if (indent_update == -1)
        indent -= indentsize;

    std::string add = "";
    for (size_t i = 0; i < indent; ++i)
        add += " ";
    add += line;
    output.push_back(add);

    if (indent_update == 1)
        indent += indentsize;
}

std::vector<std::string>* gen_c_code(std::vector<char> &input)
{
    std::vector<std::string>* out = new std::vector<std::string>();

    /* Default program start */
    emit_line(*out, "int main(void) {", 1);
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
            emit_line(*out, "*" + ptr + " = getchar();");
            break;
        case '[':
            emit_line(*out, "while (*" + ptr + ") {", 1);
            break;
        case ']':
            emit_line(*out, "}", -1);
            break;
        }
    }

    /* Default program end */
    emit_line(*out, "return 0;");
    emit_line(*out, "}", -1);
    return out;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "Usage: bft <input.bf>" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::istream_iterator<char> start(infile), end;
    std::vector<char> v(start, end);
    std::vector<std::string>* output = gen_c_code(v);

    for (auto i : *output)
        std::cout << i << std::endl;

    return 0;
}
