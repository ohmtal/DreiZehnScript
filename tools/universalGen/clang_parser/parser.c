#include <clang-c/Index.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char* escape_json_string(const char* input) {
    if (!input) return NULL;

    size_t len = strlen(input);
    char *escaped = malloc(len * 2 + 1);
    if (!escaped) return NULL;

    char *dst = escaped;
    while (*input) {
        if (*input == '"') {
            *dst++ = '\\';
            *dst++ = '"';
        } else if (*input == '\\') {
            *dst++ = '\\';
            *dst++ = '\\';
        } else if (*input == '\n') {
            *dst++ = '\\';
            *dst++ = 'n';
        } else if (*input == '\r') {
            *dst++ = '\\';
            *dst++ = 'r';
        } else if (*input == '\t') {
            *dst++ = '\\';
            *dst++ = 't';
        } else {
            *dst++ = *input;
        }
        input++;
    }
    *dst = '\0';
    return escaped;
}


// // Helper to construct the full namespace/class qualifier path
// void get_qualified_prefix(CXCursor cursor, char *buffer, size_t max_len) {
//     buffer[0] = '\0';
//
//     CXCursor parent = clang_getCursorSemanticParent(cursor);
//
//     char parts[16][64];
//     int part_count = 0;
//
//     while (!clang_Cursor_isNull(parent) && part_count < 16) {
//         enum CXCursorKind kind = clang_getCursorKind(parent);
//
//         if (kind == CXCursor_TranslationUnit) {
//             break;
//         }
//         if (kind == CXCursor_Namespace || kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) {
//             CXString name = clang_getCursorSpelling(parent);
//             const char *name_str = clang_getCString(name);
//
//             if (name_str && strlen(name_str) > 0) {
//                 strncpy(parts[part_count++], name_str, 63);
//             }
//
//             clang_disposeString(name);
//         }
//
//         parent = clang_getCursorSemanticParent(parent);
//     }
//
//     for (int i = part_count - 1; i >= 0; i--) {
//         strncat(buffer, parts[i], max_len - strlen(buffer) - 1);
//         strncat(buffer, "::", max_len - strlen(buffer) - 1);
//     }
// }

static enum CXChildVisitResult visit(
    CXCursor cursor, CXCursor parent, CXClientData data)
{
    enum CXCursorKind kind = clang_getCursorKind(cursor);
    enum CXCursorKind parentKind = clang_getCursorKind(parent);

    CXSourceLocation loc = clang_getCursorLocation(cursor);
    CXFile file = NULL;
    clang_getFileLocation(loc, &file, NULL, NULL, NULL);

    if (file) {
        CXString path = clang_getFileName(file);
        const char *filename = clang_getCString(path);

        if (filename ) {

            // ** Filenames **
            const char *short_filename = strrchr(filename, '/');
            short_filename = short_filename ? short_filename + 1 : filename;

            CXString nameStr = clang_getCursorSpelling(cursor);
            const char *name = clang_getCString(nameStr);

             // ** functions **
            if (kind == CXCursor_FunctionDecl) {
                CXType funcType = clang_getCursorType(cursor);
                CXType returnType = clang_getResultType(funcType);
                CXString retTypeStr = clang_getTypeSpelling(returnType);

                int num_args = clang_Cursor_getNumArguments(cursor);

                // --- 1. Start JSON output and basic metadata ---
                printf("{\"file\":\"%s\",\"kind\":\"function\",\"name\":\"%s\",", short_filename, name);

                // --- 2. Extract and print namespaces as a JSON array ---
                printf("\"namespaces\":[");

                // Temporary storage to hold parent names up to 16 levels deep
                CXString namespace_strings[16];
                int ns_count = 0;

                CXCursor parent = clang_getCursorSemanticParent(cursor);
                while (!clang_Cursor_isNull(parent) && ns_count < 16) {
                    enum CXCursorKind parentKind = clang_getCursorKind(parent);

                    if (parentKind == CXCursor_TranslationUnit) {
                        break;
                    }

                    if (parentKind == CXCursor_Namespace) {
                        namespace_strings[ns_count++] = clang_getCursorSpelling(parent);
                    }

                    parent = clang_getCursorSemanticParent(parent);
                }

                // Print names in reverse order (outermost namespace first)
                for (int i = ns_count - 1; i >= 0; i--) {
                    printf("\"%s\"", clang_getCString(namespace_strings[i]));
                    if (i > 0) {
                        printf(",");
                    }
                    clang_disposeString(namespace_strings[i]); // Clean up Clang string memory
                }
                printf("],");

                // --- 3. Continue with return type and params ---
                printf("\"return_type\":\"%s\",\"params\":[", clang_getCString(retTypeStr));

                for (int i = 0; i < num_args; i++) {
                    CXCursor param_cursor = clang_Cursor_getArgument(cursor, i);
                    CXString param_name_str = clang_getCursorSpelling(param_cursor);
                    CXString param_type_str = clang_getTypeSpelling(clang_getCursorType(param_cursor));

                    printf("{\"name\":\"%s\",\"type\":\"%s\"}",
                           clang_getCString(param_name_str), clang_getCString(param_type_str));

                    if (i < num_args - 1) {
                        printf(",");
                    }

                    clang_disposeString(param_name_str);
                    clang_disposeString(param_type_str);
                }

                printf("]}\n");

                clang_disposeString(retTypeStr);
            }

            //  if (kind == CXCursor_FunctionDecl) {
            //     CXType funcType = clang_getCursorType(cursor);
            //     CXType returnType = clang_getResultType(funcType);
            //     CXString retTypeStr = clang_getTypeSpelling(returnType);
            //
            //     int num_args = clang_Cursor_getNumArguments(cursor);
            //
            //     printf("{\"file\":\"%s\",\"kind\":\"function\",\"name\":\"%s\",\"return_type\":\"%s\",\"params\":[",
            //            short_filename, name, clang_getCString(retTypeStr));
            //
            //     for (int i = 0; i < num_args; i++) {
            //         CXCursor param_cursor = clang_Cursor_getArgument(cursor, i);
            //         CXString param_name_str = clang_getCursorSpelling(param_cursor);
            //         CXString param_type_str = clang_getTypeSpelling(clang_getCursorType(param_cursor));
            //
            //         printf("{\"name\":\"%s\",\"type\":\"%s\"}",
            //                clang_getCString(param_name_str), clang_getCString(param_type_str));
            //
            //         if (i < num_args - 1) {
            //             printf(",");
            //         }
            //
            //         clang_disposeString(param_name_str);
            //         clang_disposeString(param_type_str);
            //     }
            //
            //     printf("]}\n");
            //
            //     clang_disposeString(retTypeStr);
            // }


            // ** function params **
            else if (kind == CXCursor_ParmDecl && parentKind == CXCursor_FunctionDecl) {
                CXString parentNameStr = clang_getCursorSpelling(parent);
                CXString paramTypeStr = clang_getTypeSpelling(clang_getCursorType(cursor));

                printf("{\"file\":\"%s\",\"kind\":\"param\",\"function\":\"%s\",\"name\":\"%s\",\"type\":\"%s\"}\n",
                       short_filename, clang_getCString(parentNameStr), name, clang_getCString(paramTypeStr));

                clang_disposeString(parentNameStr);
                clang_disposeString(paramTypeStr);
            }

            // ** struct fields **
            else if (kind == CXCursor_StructDecl && clang_isCursorDefinition(cursor)) {
                printf("{\"file\":\"%s\",\"kind\":\"struct\",\"name\":\"%s\"}\n", short_filename, name);
            }
            else if (kind == CXCursor_FieldDecl && parentKind == CXCursor_StructDecl) {
                CXString parentNameStr = clang_getCursorSpelling(parent);
                CXString fieldTypeStr = clang_getTypeSpelling(clang_getCursorType(cursor));

                printf("{\"file\":\"%s\",\"kind\":\"struct_field\",\"struct\":\"%s\",\"field\":\"%s\",\"type\":\"%s\"}\n",
                       short_filename, clang_getCString(parentNameStr), name, clang_getCString(fieldTypeStr));

                clang_disposeString(parentNameStr);
                clang_disposeString(fieldTypeStr);
            }


            else if (kind == CXCursor_EnumConstantDecl && parentKind == CXCursor_EnumDecl) {
                CXString parentNameStr = clang_getCursorSpelling(parent);
                long long enumValue = clang_getEnumConstantDeclValue(cursor);

                printf("{\"file\":\"%s\",\"kind\":\"enum_constant\",\"enum\":\"%s\",\"name\":\"%s\",\"value\":\"%lld\"}\n",
                       short_filename, clang_getCString(parentNameStr), name, enumValue);

                clang_disposeString(parentNameStr);
            }

            // *** macros ***
            else if (kind == CXCursor_MacroDefinition) {

                if (clang_Cursor_isMacroFunctionLike(cursor)) {
                    return CXChildVisit_Continue;
                }

                CXSourceRange range = clang_getCursorExtent(cursor);
                CXSourceLocation startLoc = clang_getRangeStart(range);
                CXSourceLocation endLoc = clang_getRangeEnd(range);
                unsigned startOffset = 0, endOffset = 0;

                clang_getFileLocation(startLoc, &file, NULL, NULL, &startOffset);
                clang_getFileLocation(endLoc, &file, NULL, NULL, &endOffset);

                if (endOffset > startOffset) {
                    FILE *f = fopen(filename, "r");
                    if (f) {
                        unsigned length = endOffset - startOffset;
                        char *buffer = malloc(length + 1);
                        if (buffer) {
                            fseek(f, startOffset, SEEK_SET);
                            if (fread(buffer, 1, length, f) == length) {
                                buffer[length] = '\0';
                                char *ptr = buffer;
                                if (strncmp(ptr, "#define", 7) == 0) ptr += 7;
                                while (*ptr == ' ' || *ptr == '\t') ptr++;
                                if (strncmp(ptr, name, strlen(name)) == 0) ptr += strlen(name);
                                while (*ptr == ' ' || *ptr == '\t') ptr++;

                                char *end = ptr + strlen(ptr) - 1;
                                while (end >= ptr && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
                                    *end = '\0';
                                    end--;
                                }

                                if (strlen(ptr) > 0) {
                                     char *safe_value = escape_json_string(ptr);
                                    printf("{\"file\":\"%s\",\"kind\":\"constant_macro\",\"name\":\"%s\",\"value\":\"%s\"}\n",
                                           short_filename, name, safe_value ? safe_value : "");

                                    // printf("{\"file\":\"%s\",\"kind\":\"constant_macro\",\"name\":\"%s\",\"value\":\"%s\"}\n",
                                           // short_filename, name, ptr);
                                }
                            }
                            free(buffer);
                        }
                        fclose(f);
                    }
                }
            }

            clang_disposeString(nameStr);
        }
        clang_disposeString(path);
    }

    return CXChildVisit_Recurse;
}

int main(int argc, char *argv[])
{

    if (argc < 3) {
        fprintf(stderr, "Usage: %s header_name.h /include/path \n", argv[0]);
        return 1;
    }

    const char *header_to_include = argv[1];

    char include_path[256] ;
    snprintf(include_path, sizeof(include_path), "-I%s", argv[2]);

    // const char *args[] = {
    //     "-x", "c",
    //     "-std=c11",
    //     include_path
    // };

    // for namespace:
    const char *args[] = {
        "-x", "c++",
        "-std=c++17",
        include_path
    };

    char source_code[256];
    snprintf(source_code, sizeof(source_code), "#include <%s>;\n", header_to_include);

    //DEBUG printf("\nCODE: %s\n", source_code);return 0;

    CXIndex index = clang_createIndex(0, 0);

    struct CXUnsavedFile unsaved_file;
    unsaved_file.Filename = "virtual_main.c";
    unsaved_file.Contents = source_code;
    unsaved_file.Length = strlen(source_code);

    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        "virtual_main.c",
        args, sizeof(args) / sizeof(args[0]),
            &unsaved_file, 1,
            CXTranslationUnit_DetailedPreprocessingRecord);

    if (!tu) {
        fprintf(stderr, "Translation error!\n");
        clang_disposeIndex(index);
        return 1;
    }

    unsigned num_diagnostics = clang_getNumDiagnostics(tu);
    if (num_diagnostics > 0) {
        fprintf(stderr, "--- Clang Parser-Warnings/Errors ---\n");
        for (unsigned i = 0; i < num_diagnostics; ++i) {
            CXDiagnostic diag = clang_getDiagnostic(tu, i);
            CXString string = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
            fprintf(stderr, "%s\n", clang_getCString(string));
            clang_disposeString(string);
            clang_disposeDiagnostic(diag);
        }
        fprintf(stderr, "-------------------------------------\n");
    }

    // run AST
    clang_visitChildren(
        clang_getTranslationUnitCursor(tu), visit, NULL);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return 0;
}
