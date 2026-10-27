//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Lexer / Tokenizer
//-----------------------------------------------------------------------------
#include "Lexer.h"

namespace DreiZehn {

// =============================================================================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (peek() != '\0') {
        // // ----------------------------------------------------------------
        if (std::isspace(peek())) { advance(); continue; }
        // ----------------------------------------------------------------
        if (peek() == '.' && peekNext() == '.') { advance();advance(); tokens.push_back({TokenType::DotDot, ".."}); continue; }
        if (peek() == '.' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::DotAssign, ".="}); continue; }
        if (peek() == '=' && peekNext() == '>') { advance();advance(); tokens.push_back({TokenType::KeyValue, "=>"}); continue; }
        if (peek() == '>' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::GreaterEqual, ">="}); continue; }
        if (peek() == '<' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::LowerEqual, "<="}); continue; }

        if (peek() == '+' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignPlus, "+="}); continue; }
        if (peek() == '-' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignMinus, "-="}); continue; }
        if (peek() == '*' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignMul, "*="}); continue; }
        if (peek() == '/' && peekNext() == '=') { advance();advance(); tokens.push_back({TokenType::AssignDiv, "/="}); continue; }

        if (peek() == '=') {
            advance();
            if (peek() == '=') { // "=="
                advance();
                tokens.push_back({TokenType::Equal, "=="});
            } else {
                tokens.push_back({TokenType::Assign, "="});
            }
            continue;
        }
        if (peek() == '!' && peekNext() == '=') {
            advance();
            tokens.push_back({TokenType::NotEqual, "!="});
        }

        if (peek() == '+' && peekNext() == '+') { advance();advance(); tokens.push_back({TokenType::PlusPlus, "++"}); continue; }
        if (peek() == '-' && peekNext() == '-') { advance();advance(); tokens.push_back({TokenType::MinusMinus, "--"}); continue; }

        if (peek() == '(') { advance(); tokens.push_back({TokenType::LParen, "("}); continue; }
        if (peek() == ')') { advance(); tokens.push_back({TokenType::RParen, ")"}); continue; }
        if (peek() == '+') { advance(); tokens.push_back({TokenType::Plus, "+"}); continue; }
        if (peek() == '*') { advance(); tokens.push_back({TokenType::Mul,   "*"}); continue; }
        if (peek() == '/') { advance(); tokens.push_back({TokenType::Div,   "/"}); continue; }

        if (peek() == '!') { advance(); tokens.push_back({TokenType::Not,   "!"}); continue; }
        if (peek() == '%') { advance(); tokens.push_back({TokenType::Modulo,"%"}); continue; }

        if (peek() == '[') { advance(); tokens.push_back({TokenType::LSquare,"["}); continue; }
        if (peek() == ']') { advance(); tokens.push_back({TokenType::RSquare,"]"}); continue; }


        if (peek() == '{') { advance(); tokens.push_back({TokenType::LCurly,"{"}); continue; }
        if (peek() == '}') { advance(); tokens.push_back({TokenType::RCurly,"}"}); continue; }

        // ----------------------------------------------------------------
        if (peek() == '-' && peekNext() == '>') {
            advance();advance();
            tokens.push_back({TokenType::Arrow, "->"});
            continue;
        }

        if (peek() == '>' && peekNext() == '>') { advance();advance(); tokens.push_back({TokenType::SHR, ">>"}); continue; }
        if (peek() == '<' && peekNext() == '<') { advance();advance(); tokens.push_back({TokenType::SHL, "<<"}); continue; }

        if (peek() == '>') { advance(); tokens.push_back({TokenType::Greater, ">"}); continue; }
        if (peek() == '<') { advance(); tokens.push_back({TokenType::Less, "<"}); continue; }
        // ----------------------------------------------------------------

        if (peek() == '^' ) { advance(); tokens.push_back({TokenType::BitXOr, "^"}); continue; }
        if (peek() == '|' && peekNext() != '|') { advance(); tokens.push_back({TokenType::BitOr, "|"}); continue; }
        if (peek() == '&' && peekNext() != '&') { advance(); tokens.push_back({TokenType::BitAnd, "&"}); continue; }

        if (peek() == '|' && peekNext() == '|') { advance();advance(); tokens.push_back({TokenType::Or, "||"}); continue; }
        if (peek() == '&' && peekNext() == '&') { advance();advance(); tokens.push_back({TokenType::And, "&&"}); continue; }

        // ----------------------------------------------------------------
        // Numbers
        //  - hex numbers  ( 0xFFFFFF / 0x1a2b)
        if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X') && std::isxdigit(peekOffset(+2)) ) {
            std::string hexNum;
            hexNum += advance(); hexNum += advance();
            while (std::isxdigit(peek())) { hexNum += advance(); }
            unsigned long long rawValue = std::stoull(hexNum, nullptr, 16);
            double numVal = static_cast<double>(rawValue);
            tokens.push_back({TokenType::Number, std::to_string(numVal)});
            // int32_t intVal = static_cast<int32_t>(rawValue);
            // tokens.push_back({TokenType::Number, std::to_string(intVal)});
            continue;
        }

        // --- Normal number
        if (std::isdigit(peek()) || (peek() == '.' && std::isdigit(peekNext()))) {
            std::string num;
            bool hasDot = false;
            bool hasExponent = false;

            while (peek() != '\0') {
                char c = peek();

                if (std::isdigit(c)) {
                    num += advance();
                }
                else if (c == '.' && !hasDot && !hasExponent) {
                    hasDot = true;
                    num += advance();
                }
                else if ((c == 'e' || c == 'E') && !hasExponent) {
                    hasExponent = true;
                    num += advance();
                    if (peek() == '+' || peek() == '-') {
                        num += advance();
                    }
                }
                else {
                    break;
                }
            }
            tokens.push_back({TokenType::Number, num});
            continue;
        }


        // if ( std::isdigit(peek())
        //     || (std::isdigit(peekPrev()) && peek() == '.' && std::isdigit(peekNext()))
        //     || (peek() == '-' && std::isdigit(peekNext()) && (tokens.size()>0 && isMathType(tokens.back())) )
        //     // || (!std::isdigit(peekPrev()) && peek() == '-' && std::isdigit(peekNext()))
        //
        //
        // ){
        //     std::string num;
        //     bool isFirst = true;
        //     while (std::isdigit(peek()) || (peek() == '.' && !isFirst) || (peek() == '-' && isFirst)) {
        //         num += advance();
        //         isFirst = false;
        //     }
        //     tokens.push_back({TokenType::Number, num});
        //     continue;
        // }

        // minus after number
        if (peek() == '-') { advance(); tokens.push_back({TokenType::Minus, "-"}); continue; }

        if (peek() == '.') { advance(); tokens.push_back({TokenType::Dot, "."}); continue; }

        // ----------------------------------------------------------------
        // Identifier and Keywords scan
        // 0.6c only allow "::" not more or less
        if (std::isalpha(peek()) || peek() == '_' || ( peek() == ':' && peekNext() == ':') ) {
            std::string id;

            if (peek() == ':'  ) {
                if (peekNext() == ':') {
                    advance();
                }
            }

            while (std::isalnum(peek()) || peek() == '_' ||/* peek() == '.' ||*/ peek() == ':') {
                id += advance();
            }

            // early out to enable fields and methods with the registered names
            TokenType prevType = tokens.size() > 0 ? tokens.back().mType : TokenType::NoToken;
            if (prevType == TokenType::Dot || prevType == TokenType::Arrow)
            {
                tokens.push_back({TokenType::Identifier, id});
                continue;
            }

            if (id == "if") { tokens.push_back({TokenType::If, "if"}); }
            else if (id == "else") { tokens.push_back({TokenType::Else, "else"}); }
            else if (id == "elif") { tokens.push_back({TokenType::ElIf, "elif"});  }
            else if (id == "fn") { tokens.push_back({TokenType::Fn, "fn"});  }
            else if (id == "function") { tokens.push_back({TokenType::Fn, "function"});  }
            else if (id == "end") { tokens.push_back({TokenType::End, "end"}); }
            else if (id == "for") { tokens.push_back({TokenType::For, "for"});  }
            else if (id == "break") { tokens.push_back({TokenType::Break, "break"});  }
            else if (id == "return") { tokens.push_back({TokenType::Return, "return"});  }
            else if (id == "continue") { tokens.push_back({TokenType::Continue, "continue"});  }
            else if (id == "while") { tokens.push_back({TokenType::While, "while"});  }
            else if (id == "forRange") { tokens.push_back({TokenType::forRange, "forRange"});  }
            else if (id == "forrange") { tokens.push_back({TokenType::forRange, "forrange"});  }
            else if (id == "forEach") { tokens.push_back({TokenType::forEach, "forEach"});  }
            else if (id == "foreach") { tokens.push_back({TokenType::forEach, "foreach"});  }
            else if (id == "not") { tokens.push_back({TokenType::Not, "not"});  }
            else if (id == "and") { tokens.push_back({TokenType::And, "and"});  }
            else if (id == "or") { tokens.push_back({TokenType::Or, "or"});  }
            else if (id == "import") { tokens.push_back({TokenType::Import, "import"});  }
            else if (id == "include") { tokens.push_back({TokenType::Include, "include"});  }
            else if (id == "define") { tokens.push_back({TokenType::Define, "define"});  }
            else if (id == "global") { tokens.push_back({TokenType::Global, "global"});  }
            else {
                tokens.push_back({TokenType::Identifier, id});
            }
            continue;
        }

        if (peek() == '?') { advance(); tokens.push_back({TokenType::Question, "?"}); continue; }
        if (peek() == ':') { advance(); tokens.push_back({TokenType::Colon, ":"}); continue; }
        if (peek() == ',') { advance(); tokens.push_back({TokenType::Comma, ","}); continue; }

        // ----------------------------------------------------------------
        // StringLiteral
        if (peek() == '"') {
            advance(); // eat '"'
            std::string strValue;

            while (peek() != '"' && peek() != '\0') {
                if (peek() == '\\') {
                    advance(); // eat '\\'

                    if (peek() == '\0') break;

                    if (peek() == 'n')      { strValue += '\n'; advance(); }
                    else if (peek() == 't') { strValue += '\t'; advance(); } // \t
                    else if (peek() == 'r') { strValue += '\r'; advance(); } // \r
                    else if (peek() == '"') { strValue += '"';  advance(); }
                    else if (peek() == '\\'){ strValue += '\\'; advance(); } // Backslash-Escaping
                    else {
                        strValue += advance();
                    }
                } else {
                    strValue += advance();
                }
            }

            if (peek() == '"') {
                advance(); // eat '"'
            } else {
                Tools::errorf("[Lexer-ERROR] String not closed!\n");
                tokens.clear();
                return tokens;
            }

            tokens.push_back({TokenType::StringLiteral, strValue});
            continue;
        }
        // StringLiteral 2
        if (peek() == '\'') {
            advance(); // eat
            std::string strValue;

            while (peek() != '\'' && peek() != '\0') {
                if (peek() == '\\') {
                    advance(); // eat '\\'

                    if (peek() == '\0') break;

                    if (peek() == 'n')      { strValue += '\n'; advance(); }
                    else if (peek() == 't') { strValue += '\t'; advance(); } // \t
                    else if (peek() == 'r') { strValue += '\r'; advance(); } // \r
                    else if (peek() == '"') { strValue += '"';  advance(); }
                    else if (peek() == '\'') { strValue += '\'';  advance(); }
                    else if (peek() == '\\'){ strValue += '\\'; advance(); } // Backslash-Escaping
                    else {
                        strValue += advance();
                    }
                } else {
                    strValue += advance();
                }
            }

            if (peek() == '\'') {
                advance(); // eat
            } else {
                Tools::errorf("[Lexer-ERROR] String not closed!\n");
                tokens.clear();
                return tokens;
            }

            tokens.push_back({TokenType::StringLiteral, strValue});
            continue;
        }

        // ---------- Comment ---------------
        if (peek() == '#') {
            advance(); // eat '#'
            while ( peek() != '\0') {
                if (peek() == '\\' && peekNext() == 'n' ) {
                    advance();advance();
                    break; //end of line
                }
                if (peek() == '#') {
                    advance();
                    break;
                }
                advance();
            }
            continue;
        }


        if (peek() == ';' && peekNext() == ';') { advance(); tokens.push_back({TokenType::End, ";;"}); continue; }
        if (peek() == ';') { advance(); tokens.push_back({TokenType::Semicolon, ";"}); continue; }

        advance(); // skip unknown
    }
    tokens.push_back({TokenType::EOFToken, ""});
    return tokens;
}









} //namespace DreiZehn
