//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS Demo
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>
#include "core/VM.h"
#include "core/Lexer.h"
#include "core/Parser.h"
using namespace DLS;

int main(int argc, char* argv[]) {


    std::string mySrc = R"(
        4.f + 5 * 20
    )";

    Lexer lex(mySrc);
    std::vector<Token> token = lex.tokenize();
    for (auto& tok: token) {
        std::cout << tokenTypeToString(tok.mType) << "::" << tok.mValue << "\n";
    }
    VM vm;
    Parser parser(token,&vm);
    parser.parse();

    std::cout << "result: " << vm.run().toString() << "\n";

}
