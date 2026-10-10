//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FileStream inital r/w stdin stdout
//-----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"




namespace DreiZehn {

const int TypeFileStreamObject = RegisterUserObjectType("FileStream");


struct FileStreamObject : public ValueObject {
    std::istream* input = nullptr;
    std::ostream* output = nullptr;

    std::unique_ptr<std::ifstream> fileInput = nullptr;
    std::unique_ptr<std::ofstream> fileOutput = nullptr;
    std::unique_ptr<std::fstream> fileStream = nullptr;

    FileStreamObject() : ValueObject(TypeFileStreamObject) { }
    ~FileStreamObject() { }

    inline static ValueObjectProperty writeProp, writelnProp, readProp, readlnProp
        , seekProp, tellProp, flushProp, closeProp;

    // -------------------------------------------------------------------------
    bool file_seek(std::streamoff offset, std::ios_base::seekdir whence) {
        if (this->fileStream) {
            this->fileStream->seekg(offset, whence);
            this->fileStream->seekp(offset, whence);
            return !this->fileStream->fail();
        }
        else if (this->fileInput) {
            this->fileInput->seekg(offset, whence);
            return !this->fileInput->fail();
        }
        else if (this->fileOutput) {
            this->fileOutput->seekp(offset, whence);
            return !this->fileOutput->fail();
        }
        return false;
    }
    // -------------------------------------------------------------------------
    std::streampos file_tell() {
        if (this->fileStream) {
            std::streampos pos = this->fileStream->tellp();
            if (pos == std::streampos(-1)) {
                pos = this->fileStream->tellg();
            }
            return pos;
        }
        else if (this->fileInput) {
            return this->fileInput->tellg();
        }
        else if (this->fileOutput) {
            return this->fileOutput->tellp();
        }
        return std::streampos(-1);
    }
    // -------------------------------------------------------------------------
    inline static void RegisterSymbols() {
        static bool mSymbolsLoaded = false;
        if (mSymbolsLoaded) return;

        writeProp = ValueObjectProperty("write", 1, 1, "write to stream, @param size", TypeFileStreamObject);
        writelnProp = ValueObjectProperty("writeln", 1, 1, "write a line to stream, @param size", TypeFileStreamObject);
        readProp = ValueObjectProperty("read", 1, 1, "read from stream, @param size", TypeFileStreamObject);
        readlnProp = ValueObjectProperty("readln", 1, 1, "read a line from stream, @param size", TypeFileStreamObject);
        seekProp = ValueObjectProperty("seek", 2, 2, "seek in stream, @param offset seekdir", TypeFileStreamObject);
        tellProp = ValueObjectProperty("tell", 0, 0, "seek in stream, @return -1 if failed", TypeFileStreamObject);
        flushProp = ValueObjectProperty("flush", 0, 0, "flush the stream", TypeFileStreamObject);
        closeProp = ValueObjectProperty("close", 0, 0, "close the stream", TypeFileStreamObject);
        mSymbolsLoaded = true;
    }
    // -------------------------------------------------------------------------

    inline bool onMethodCall(uint32_t methodId, std::vector<Value>& args, Value& ret) override {
        // ------- write
        if (writeProp.matchMethod(methodId, args) == 1) {
            if (output && args[0].isStringId()) {
                *output << args[0].getStringRef();
                ret = Value(1);
            } else {
                ret = Value(0);
            }
            return true;
        }
        // ------- writeln
        if (writelnProp.matchMethod(methodId, args) == 1) {
            if (output && args[0].isStringId()) {
                *output << args[0].getStringRef() << "\r\n";
                ret = Value(1);
            } else {
                ret = Value(0);
            }
            return true;
        }


        // ------- readln (like  PHP's fgets mit Buffersize)
        else if (readlnProp.matchMethod(methodId, args) == 1) {
            uint32_t buffer_size = args[0].getUInt();

            if (input && buffer_size > 0) {
                std::vector<char> buffer(buffer_size + 1, '\0');

                input->getline(buffer.data(), buffer.size());

                bool isEOF = input->eof();
                std::streamsize bytesRead = input->gcount();

                if (input->fail()) {
                    input->clear(input->rdstate() & ~std::ios_base::failbit);
                }

                if (bytesRead > 0) {
                    std::string line(buffer.data());

                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }

                    ret = Value(line);
                } else {
                    ret = Value(0);
                }
            } else {
                ret = Value(0);
            }
            return true;
        }



        // else if (readlnProp.matchMethod(methodId, args) == 1) {
        //     uint32_t buffer_size = args[0].getUInt();
        //
        //     if (input && buffer_size > 0) {
        //         std::string line;
        //
        //         if (std::getline(*input, line)) {
        //
        //             if (line.length() > buffer_size) {
        //                 line.resize(buffer_size);
        //             }
        //
        //             if (!line.empty() && line.back() == '\r') {
        //                 line.pop_back();
        //             }
        //
        //             ret = Value(line);
        //         } else {
        //             ret = Value(0);
        //         }
        //     } else {
        //         ret = Value(0);
        //     }
        //     return true;
        // }

        // ------- read
        else if (readProp.matchMethod(methodId, args) == 1) {
            uint32_t bytes = args[0].getUInt();
            if (input && bytes > 0) {
                std::string buffer(bytes, '\0');
                input->read(&buffer[0], bytes);
                std::streamsize bytesRead = input->gcount();

                if (bytesRead > 0) {
                    if (static_cast<uint32_t>(bytesRead) < bytes) {
                        buffer.resize(bytesRead);
                        if (input->fail()) {
                            input->clear(input->rdstate() & ~std::ios_base::failbit);
                        }
                    }
                    ret = Value(buffer);
                } else {
                    if (input->fail()) {
                        input->clear(input->rdstate() & ~std::ios_base::failbit);
                    }
                    ret = Value(0);
                }
            } else {
                ret = Value(0);
            }
            return true;
        }

        // else if (readProp.matchMethod(methodId, args) == 1) {
        //     uint32_t bytes = args[0].getUInt();
        //     if (input && bytes > 0) {
        //         std::string buffer(bytes, '\0');
        //         input->read(&buffer[0], bytes);
        //         std::streamsize bytesRead = input->gcount();
        //         if (bytesRead > 0) {
        //             if (static_cast<uint32_t>(bytesRead) < bytes) {
        //                 buffer.resize(bytesRead);
        //             }
        //             ret = Value(buffer);
        //         } else {
        //             ret = Value(0);
        //         }
        //     } else {
        //         ret = Value(0);
        //     }
        //     return true;
        // }
        // ------- seek
        else if (seekProp.matchMethod(methodId, args) == 1) {
            // bool file_seek(std::streamoff offset, std::ios_base::seekdir whence)

            std::streamoff offset = static_cast<std::streamoff>(args[0].getDouble());
            unsigned int whenceRaw = args[1].getUInt();
            std::ios_base::seekdir whence;

            switch (whenceRaw) {
                case 0: whence = std::ios_base::beg; break;
                case 1: whence = std::ios_base::cur; break;
                case 2: whence = std::ios_base::end; break;
                default:
                    ret = Value(false); // Ungültiger Modus übergeben
                    return true;
            }

            ret = Value(this->file_seek(offset, whence));
            return true;
        }
        // ------- tell
        else if (tellProp.matchMethod(methodId, args) == 1) {
            std::streampos pos = this->file_tell();

            if (pos == std::streampos(-1)) {
                ret = Value(-1.0);
            } else {
                ret = Value(static_cast<double>(pos));
            }
            return true;
        }
        // ------- flush
        else if (flushProp.matchMethod(methodId, args) == 1) {
            if (output) {
                output->flush();
                ret = Value(1);
            } else {
                ret = Value(0);
            }
            return true;
        }
        else if (closeProp.matchMethod(methodId, args) == 1) {
            if (this->fileInput)  this->fileInput.reset();
            if (this->fileOutput) this->fileOutput.reset();
            if (this->fileStream) this->fileStream.reset();

            this->input = nullptr;
            this->output = nullptr;

            ret = Value(1);
            return true;
        }

        ret = Value(0);
        return true;

    }
};

void RegisterFileFunctions(Environment& env) {
    static bool registered = false; if (registered) return; registered = true;
    using namespace FunctionMap;

    RegisterConstants("fs::seek_beg", Value(0));
    RegisterConstants("fs::seek_cur", Value(1));
    RegisterConstants("fs::seek_end", Value(2));




    FileStreamObject::RegisterSymbols();

    // Constructor
    RegisterFunction("FileStream::open", [](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 2 || !args[0].isStringId() || !args[1].isStringId()) {
            Tools::errorf("Usage: FileStream::open filename mode");
            ret = Value(0);
            return true;
        }

        std::string filename = args[0].getStringRef();
        std::string mode = args[1].getStringRef();

        FileStreamObject* fs = new FileStreamObject();

        if (filename == "fs://stdin" ) {
            fs->input = &std::cin;
        }
        else if (filename == "fs://stdout" ) {
            fs->output = &std::cout;
        }
        else {

            std::ios_base::openmode openMode = std::ios::binary;
            bool useStream = true;

            if (mode == "r") {
                fs->fileInput = std::make_unique<std::ifstream>(filename, openMode);
                if (!fs->fileInput->is_open()) { delete fs; ret = Value(0); return true; }
                fs->input = fs->fileInput.get();
                useStream = false; // Wir nutzen hier noch den alten ifstream
            }
            else if (mode == "r+") {
                openMode |= std::ios::in | std::ios::out;
            }
            else if (mode == "w") {
                openMode |= std::ios::out | std::ios::trunc;
            }
            else if (mode == "w+") {
                openMode |= std::ios::in | std::ios::out | std::ios::trunc;
            }
            else if (mode == "a") {
                openMode |= std::ios::out | std::ios::app;
            }
            else if (mode == "a+") {
                openMode |= std::ios::in | std::ios::out | std::ios::app;
            }

            if (useStream && mode != "r") {
                fs->fileStream = std::make_unique<std::fstream>(filename, openMode);
                if (!fs->fileStream->is_open()) {
                    delete fs;
                    ret = Value(0);
                    return true;
                }
                fs->input = fs->fileStream.get();
                fs->output = fs->fileStream.get();
            }

        }

        ret = Value(fs);
        return true;
    });



};

} //namespace
