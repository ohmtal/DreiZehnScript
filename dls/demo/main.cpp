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
#include "functions/MathFunctions.h"


using namespace DLS;

std::string Run(std::string code) {
    Lexer lex(code);
    std::vector<Token> token = lex.tokenize();
    // for (auto& tok: token) {
    //     std::cout << tokenTypeToString(tok.mType) << "  ::  " << tok.mValue << "\n";
    // }
    VM vm;
    Parser parser(token,&vm);
    if (!parser.parse()) {
        return "ERROR!";
    }
    vm.run();
    std::string result = "";
    int iter = 0;
    for (auto val: vm.stack) {
        if (iter > 0) result = result  + ", ";
        result = result + val.toString();
        iter++;
    }
    return result;
}


int main(int argc, char* argv[]) {

    RegisterMathFunctions();

    std::cout << "=============  TAPE TEST ===============\n";
    std::cout << "------------ BASIC MATH ---------------\n";
    std::string mySrc = "";
    mySrc = R"( (4.f + 5) * 20 )";
    std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( 3 * 4 + 1 )";
    std::cout << mySrc << " =  " << Run(mySrc) << "\n";
    mySrc = R"( 3 * (4 + 1) )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";

    std::cout << "------------ COMMA (multi result) ---------------\n";
    mySrc = R"( 3 * (4 + 1), 5 + 5 )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";
    std::cout << "------------ Math ---------------\n";

    mySrc = R"( math::randomf )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";
    mySrc = R"( math::random 10 20 )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";

}
