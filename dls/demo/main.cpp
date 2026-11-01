//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS Demo
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>

#include "DirtyLittleScript.h"



using namespace DLS;

// bool DEBUG_TOKEN = false;

namespace DLS {
extern void RegisterMathFunctions();
extern void RegisterStringFunctions();

}

// std::string Run(std::string code) {
//     Lexer lex(code);
//     std::vector<Token> token = lex.tokenize();
//     if (DEBUG_TOKEN) {
//         for (auto& tok: token) {
//             std::cout << tokenTypeToString(tok.mType) << "  ::  " << tok.mValue << "\n";
//         }
//     }
//
//     VM vm;
//     Parser parser(token,&vm);
//     if (!parser.parse()) {
//         return "ERROR!";
//     }
//     vm.run();
//     std::string result = "";
//     int iter = 0;
//     for (auto val: vm.stack) {
//         if (iter > 0) result = result  + ", ";
//         result = result + val.toString();
//         iter++;
//     }
//     return result;
// }
//
// std::string loadScript(const std::string& scriptPath) {
//     std::ifstream file(scriptPath, std::ios::in | std::ios::binary);
//     if (!file.is_open()) {
//         Tools::errorf("Script file cant be opened: %s\n", scriptPath.c_str());
//         return "";
//     }
//     return std::string((std::istreambuf_iterator<char>(file)),
//                        std::istreambuf_iterator<char>());
// }


int main(int argc, char* argv[]) {

    DLS::Init();
    RegisterMathFunctions();
    RegisterStringFunctions();

    int minargs = 1; // for more params
    if (argc > minargs) {
        Run(loadScript( argv[minargs]));
        return 0;
    }

    std::string mySrc = "";
    std::cout << "============= Dirty Little Script ===============\n";
goto jump_to_test;

    std::cout << "Value size check:" << sizeof(Value) << "\n";


    std::cout << "=============  TAPE TEST ===============\n";
    std::cout << "------------ String  ---------------\n";
    mySrc = R"( "Hello World!" )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";

    std::cout << "------------ BASIC MATH ---------------\n";
    mySrc = R"( (4.f + 5) * 20 )";
    std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( 3 * 4 + 1 )";
    std::cout << mySrc << " =  " << Run(mySrc) << "\n";
    mySrc = R"( 3 * (4 + 1) )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";
    std::cout << "------------ (multi result) ---------------\n";
    mySrc = R"( 3 * (4 + 1); 5 + 5 )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";

    std::cout << "------------ C-Function Call ---------------\n";
    mySrc = R"( math::randomf )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";
    mySrc = R"( math::random 10 20 )";
    std::cout << mySrc << " = " << Run(mySrc) << "\n";



    std::cout << "------------ Comparison ---------------\n";
    mySrc = R"( (4.f + 5) > 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( (4.f + 5) < 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( (4.f + 5) == 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( (4.f + 5) != 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( (4.f + 5) <= 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( (4.f + 5) >= 20 )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";

    std::cout << "------------ If ---------------\n";
    mySrc = R"( if 1 > 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( if 1 > 2; 40; else 20; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( if 1 > 2; 40; elif 2 > 1; 100; else; 20; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( if 1 < 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    mySrc = R"( if; 1 < 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    std::cout << "------------ Semicolon test  ---------------\n";
    // skip empty:
    mySrc = R"( ;50; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    // semi + linebreak
    mySrc = R"(
        if 1 > 2;
            50;
        elif 2 > 1;
            100;
        else
            150;
        ;;
    )";
    std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    // only linebreak
    mySrc = R"(
        if 1 > 2
            50
        elif 2 == 2
            100
        else
            150
        ;;
    )";
    std::cout << mySrc << "  =  " << Run(mySrc) << "\n";

    std::cout << "---- nested func call and reverse POP_STACK and request params ----\n";
    // mySrc = R"( print (math::randomf) (math::rand);)"; Run(mySrc);
    mySrc = R"( print "Hello" "World" 47.11 815 )";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).c_str());
    mySrc = R"( print math::randomf, math::rand;)";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).c_str());
    mySrc = R"( print "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" )";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).c_str());

    std::cout << "---- push constant ----\n";
    mySrc = R"( define foo = 47.11 )"; Run(mySrc);
    mySrc = R"( define bar = 6.66; print "PI is" math::pi "foo:" foo "bar:" bar)"; Run(mySrc);

    std::cout << "---- Global Variables ----\n";
    mySrc = R"( a = 12.0; print a; )"; Run(mySrc);
    mySrc = R"( i = 12.0; i = i + 1; print "i ==" i (i < 10); )"; Run(mySrc);

    std::cout << "---- While ----\n";
    mySrc = R"( itr = 0;while itr < 10; itr = itr + 1; print "iter=" itr;;  )"; Run(mySrc);
    mySrc = R"(
# a Comment!
        itr = 10
        while itr > 0
            itr = itr - 1
            print "iter=" itr
            break
        end
    )"; Run(mySrc);


   std::cout << "---- Array ----\n";
    mySrc = R"( foo[110] = 10; print "10==" foo[110] )"; Run(mySrc);

    std::cout << "------- Modulo --------\n";
    mySrc = R"( i = 0; while i < 10; if i % 2; print "mod2!" i;; i = i + 1;; )"; Run(mySrc);

    std::cout << "-------- And / Or --------\n";
    mySrc = R"( print 0 || 1 )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i == 0 || i == 10; print "i is:" i;else print "not"; end  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i > 9 && i < 20; print "i is " i; else print "not";;  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i == 0 || i == 9; print "i is:" i;else print "not"; end  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i > 11 && i < 20; print "i is " i; else print "not";;  )"; Run(mySrc);


    std::cout << "-------- Lambda Fn --------\n";
    mySrc = R"(
        foo = fn ; print str::strlen "HUHU";;
        foo
        return
        print "still here ?"
    )"; Run(mySrc);

jump_to_test:
std::cout << "-------- Array again--------\n";
mySrc = R"(
i = 0
minW = -200
maxW = 200
minH = -100
maxH = 100
while i < 1;
    Points_X[i] =  math::random minW maxW
    print i ":X:" Points_X[i]
    Points_Y[i] =  math::random minH maxH
    print i ":X Y:" Points_X[i] Points_Y[i]
    Points_Z[i] =  math::randomf
    print i ":Z:" Points_Z[i]
    print i ":X Y Z:" Points_X[i] Points_Y[i] Points_Z[i]
    i = i + 1
end

)"; Run(mySrc);


}
