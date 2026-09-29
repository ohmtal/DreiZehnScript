# FIXME: not all fields should be double!!!!
import json
import os
import re

def generate_raylib_bindings(json_path, output_cpp_path):
    if not os.path.exists(json_path):
        print(f"Error: {json_path} not found! Please download it from the Raylib repository.")
        return

    # JSON Clean
    cleaned_lines = []
    with open(json_path, "r", encoding="utf-8") as f:
        for line in f:
            if '"description"' in line:
                trailing_comma = "," if line.strip().endswith(",") else ""
                line = re.sub(r'"description"\s*:.*$', f'"description": ""{trailing_comma}', line)
            cleaned_lines.append(line)

    raw_data = "".join(cleaned_lines)

    raw_data = re.sub(r'CLITERAL\s*\([^)]*\)\s*', '', raw_data)

    raw_data = re.sub(r'(?<!\\)\\(?!"|\\|/|b|f|n|r|t|u)', r'\\\\', raw_data)

    try:
        api = json.loads(raw_data)
    except json.decoder.JSONDecodeError as e:
        print(f"JSON Parsing failed at line {e.lineno}, column {e.colno}: {e.msg}")
        lines = raw_data.splitlines()
        start = max(0, e.lineno - 3)
        end = min(len(lines), e.lineno + 3)
        print("\n--- Problematic Section ---")
        for idx in range(start, end):
            marker = ">>> " if idx == e.lineno - 1 else "    "
            print(f"{marker}{idx + 1}: {lines[idx]}")
        print("---------------------------\n")
        return

    structs = api.get("structs", [])
    functions = api.get("functions", [])
    enums = api.get("enums", [])
    defines = api.get("defines", [])

    struct_names = {s["name"] for s in structs}

    cpp = []
    cpp.append("//-----------------------------------------------------------------------------")
    cpp.append("// Copyright (c) 2026 Thomas Hühn (XXTH)")
    cpp.append("// SPDX-License-Identifier: MIT")
    cpp.append("//-----------------------------------------------------------------------------")
    cpp.append("// Automatically generated Raylib Bindings for DreiZehn")
    cpp.append("//-----------------------------------------------------------------------------")
    cpp.append("\n")
    cpp.append("#include <vector>")
    cpp.append("#include <string>")
    cpp.append("#include <unordered_map>")
    cpp.append("#include \"core/FunctionMap.h\"")
    cpp.append("#include \"core/VariableFrame.h\"")
    cpp.append("#include \"Globals.h\"")

    cpp.append("\n")
    cpp.append("namespace DreiZehn::Raylib {")
    cpp.append("#include \"raylib-6.0/src/raylib.h\"")
    cpp.append("}")

    cpp.append("\n")
    cpp.append("namespace DreiZehn {\n")
    cpp.append("using namespace DreiZehn::Raylib;")


    cpp.append("// Global Type IDs")
    for s in structs:
        cpp.append(f"static int TypeRaylib{s['name']} = 0;")
    cpp.append("")

    cpp.append("// Global Field Symbol IDs")
    for s in structs:
        for field in s.get("fields", []):
            cpp.append(f"static uint32_t sym_{s['name']}_{field['name']};")
    cpp.append("")

    # --------------------------------------------------------------------------
    # --- Struct Wrapper Classes -----------------------------------------------
    # --------------------------------------------------------------------------

    cpp.append("// ==========================================")
    cpp.append("// WRAPPER CLASSES FOR RAYLIB STRUCTURES")
    cpp.append("// ==========================================\n")

    for s in structs:
        s_name = s["name"]
        fields = s.get("fields", [])

        cpp.append(f"struct ValueObject{s_name} : public ValueObject {{")

        for f in fields:
            cpp.append(f"    Value {f['name']};")
        cpp.append("")

        cpp.append(f"    ValueObject{s_name}(const {s_name}& val): ValueObject(TypeRaylib{s_name}) {{")
        cpp.append(f"        mAssigned = 0;")
        for f in fields:
            cpp.append(f"        {f['name']} = Value((double)val.{f['name']});")
        cpp.append("    }\n")

        cpp.append(f"    {s_name} toRaylib() const {{")
        cpp.append(f"        {s_name} res;")
        for f in fields:
            cpp.append(f"        res.{f['name']} = ({f['type']}){f['name']}.getDouble();")
        cpp.append("        return res;")
        cpp.append("    }\n")

        cpp.append("    inline virtual Value* onGetFieldPtr(uint32_t fieldSymbolId) override {")
        for f in fields:
            cpp.append(f"        if (fieldSymbolId == sym_{s_name}_{f['name']}) return &{f['name']};")
        cpp.append(f"        Tools::errorf(\"Runtime Error: Field not found on {s_name}.\\n\");")
        cpp.append("        return nullptr;")
        cpp.append("    }\n")

        cpp.append("    inline virtual bool onGetField(uint32_t fieldSymbolId, Value& ret) override {")
        cpp.append("        Value* ptr = onGetFieldPtr(fieldSymbolId);")
        cpp.append("        if (ptr) { ret = *ptr; return true; }")
        cpp.append("        return false;")
        cpp.append("    }\n")

        cpp.append("    inline virtual bool onSetField(uint32_t fieldSymbolId, const Value& value) override {")
        cpp.append("        Value* ptr = onGetFieldPtr(fieldSymbolId);")
        cpp.append("        if (ptr) { *ptr = value; return true; }")
        cpp.append("        return false;")
        cpp.append("    }\n")

        cpp.append("};\n")

    # --------------------------------------------------------------------------
    # --- Initialization Function ----------------------------------------------
    # --------------------------------------------------------------------------


    cpp.append("// ==========================================")
    cpp.append("// INITIALIZATION AND REGISTRATION")
    cpp.append("// ==========================================\n")
    cpp.append("void RegisterRaylibFunctions() {")
    cpp.append("    using namespace FunctionMap;\n")

    cpp.append("    // Register User Object Types and Field Symbols")
    for s in structs:
        s_name = s["name"]
        cpp.append(f"    TypeRaylib{s_name} = RegisterUserObjectType(\"{s_name}\");")
        for f in s.get("fields", []):
            cpp.append(f"    sym_{s_name}_{f['name']} = SymbolTable::insert(\"{f['name']}\");")
    cpp.append("")

    # --------------------------------------------------------------------------
    # --- Constants Registration (Enums, Defines, and Colors) ------------------
    # --------------------------------------------------------------------------

    cpp.append("    // Register Raylib Constants")
    for enum in enums:
        for val in enum.get("values", []):
            cpp.append(f"    RegisterConstants(\"raylib::{val['name']}\", Value((double){val['value']}));")

    for d in defines:
        d_type = d.get("type", "")
        d_name = d.get("name", "")

        if d_type in ["INT", "FLOAT", "DOUBLE"]:
            cpp.append(f"    RegisterConstants(\"raylib::{d_name}\", Value((double){d_name}));")
        elif d_type == "COLOR":
            cpp.append(f"    {{")
            cpp.append(f"        ValueObjectColor* const_color = new ValueObjectColor({d_name});")
            cpp.append(f"        const_color->mAssigned = 1;")
            cpp.append(f"        RegisterConstants(\"raylib::{d_name}\", Value(const_color));")
            cpp.append(f"    }}")
    cpp.append("")

    # --------------------------------------------------------------------------
    # --- add Object Constructors ----------------------------------------------
    # --------------------------------------------------------------------------
    cpp.append("    // Register Struct Constructors")
    for s in structs:
        s_name = s["name"]
        fields = s.get("fields", [])
        num_fields = len(fields)

        cpp.append(f"    RegisterFunction(\"raylib::{s_name}::new\", [](std::vector<Value>& args, Value& ret) -> bool {{")

        cpp.append(f"        if (args.size() != 0 && args.size() != {num_fields}) {{")
        field_desc = ", ".join([f"{f['type']} {f['name']}" for f in fields])
        cpp.append(f"            Tools::errorf(\"Usage: raylib::{s_name}::new() or raylib::{s_name}::new({field_desc})\\n\");")
        cpp.append("            return false;")
        cpp.append("        }\n")

        cpp.append(f"        {s_name} temp_struct;")
        cpp.append("        if (args.size() == 0) {")
        cpp.append(f"            temp_struct = {{}};")
        cpp.append("        } else {")
        for i, f in enumerate(fields):
            cpp.append(f"            temp_struct.{f['name']} = ({f['type']})args[{i}].getDouble();")
        cpp.append("        }\n")
        cpp.append(f"        ValueObject{s_name}* wrapper_res = new ValueObject{s_name}(temp_struct);")
        cpp.append("")
        cpp.append("        ret = Value(wrapper_res);")
        cpp.append("        return true;")
        cpp.append("    });\n")


    # --------------------------------------------------------------------------
    # --- Function Wrappers ----------------------------------------------------
    # --------------------------------------------------------------------------

    cpp.append("    // Register Function Bindings")
    for func in functions:
        f_name = func["name"]
        ret_type = func.get("returnType", "void")
        params = func.get("params", [])

        has_unsupported_pointer = False
        for p in params:
            if "*" in p["type"] and "const char" not in p["type"]:
                has_unsupported_pointer = True
        if "*" in ret_type and "const char" not in ret_type:
            has_unsupported_pointer = True
        if has_unsupported_pointer:
            continue

        cpp.append(f"    RegisterFunction(\"raylib::{f_name}\", [](std::vector<Value>& args, Value& ret) -> bool {{")
        cpp.append(f"        if (args.size() != {len(params)}) {{")
        param_desc = ", ".join([f"{p['type']} {p['name']}" for p in params])
        cpp.append(f"            Tools::errorf(\"Usage: raylib::{f_name}({param_desc})\\n\");")
        cpp.append("            return false;")
        cpp.append("        }\n")

        call_args = []
        for i, p in enumerate(params):
            p_type = p["type"]
            p_name = p["name"]

            if p_type in ["int", "float", "double", "bool"] or p_type in [e["name"] for e in enums]:
                cpp.append(f"        {p_type} arg_{p_name} = ({p_type})args[{i}].getDouble();")
                call_args.append(f"arg_{p_name}")
            elif p_type == "const char *":
                cpp.append(f"        const char* arg_{p_name} = args[{i}].getStringRef().c_str();")
                call_args.append(f"arg_{p_name}")
            elif p_type in struct_names:
                cpp.append(f"        if (! args[{i}].isPointer() ) return false;")
                cpp.append(f"        ValueObject* obj_{p_name} = args[{i}].asPointerObject();")
                cpp.append(f"        if (!obj_{p_name} || obj_{p_name}->mType != TypeRaylib{p_type}) {{")
                cpp.append(f"            Tools::errorf(\"Arg {i} must be of type {p_type}\");")
                cpp.append("            return false;")
                cpp.append("        }")
                cpp.append(f"        {p_type} arg_{p_name} = static_cast<ValueObject{p_type}*>(obj_{p_name})->toRaylib();")
                call_args.append(f"arg_{p_name}")
            else:
                call_args.append(f"({p_type})0")

        args_str = ", ".join(call_args)
        if ret_type == "void":
            cpp.append(f"        {f_name}({args_str});")
            cpp.append("        ret = Value(1.0);")
        elif ret_type in ["int", "float", "double", "bool"]:
            cpp.append(f"        {ret_type} c_res = {f_name}({args_str});")
            cpp.append("        ret = Value((double)c_res);")
        elif ret_type in struct_names:
            cpp.append(f"        {ret_type} c_res = {f_name}({args_str});")
            cpp.append(f"        ValueObject{ret_type}* wrapper_res = new ValueObject{ret_type}(c_res);")
            cpp.append("        ret = Value(wrapper_res);")
        else:
            cpp.append(f"        {f_name}({args_str});")
            cpp.append("        ret = Value(0.0);")

        cpp.append("        return true;")
        cpp.append("    });\n")

    cpp.append("}")
    cpp.append("} // namespace DreiZehn ")

    with open(output_cpp_path, "w", encoding="utf-8") as f:
        f.write("\n".join(cpp))

    print(f"Successfully generated: {output_cpp_path}")

if __name__ == "__main__":
    generate_raylib_bindings("raylib_api.json", "RaylibFunctions.cpp")
