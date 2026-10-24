#include <charconv>
#include <array>
#include <string>
#include "Value.h"

namespace DreiZehn {

std::string Value::toStringSlowPath() const {
    uint32_t tag = static_cast<uint32_t>((mBits & TAG_MASK) >> 48);

    if ((mBits & QNAN_MASK) != QNAN_MASK) {
        std::array<char, 64> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), std::bit_cast<double>(mBits));
        if (ec == std::errc{}) {
            return std::string(buffer.data(), ptr - buffer.data());
        }
        return "0.0";
    }

    constexpr uint32_t ID_INT = 1;
    constexpr uint32_t ID_PTR = 2;
    constexpr uint32_t ID_VALUE_PTR = 3;

    switch (tag) {
        case ID_INT: {
            std::array<char, 32> buffer;
            auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), asFastInt());
            if (ec == std::errc{}) {
                return std::string(buffer.data(), ptr - buffer.data());
            }
            return "0";
        }

        case ID_PTR: {
            if (ValueObject* obj = asPointerObject()) {
                return obj->toString();
            }
            return "nullptr";
        }

        case ID_VALUE_PTR: {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "[%p]", asValuePointer());
            return std::string(buffer);
        }

        default:
            return "unknown!";
    }
}
// -----------------------------------------------------------------------------
Value Value::handleGenericAdd(const DreiZehn::Value& a, const DreiZehn::Value& b) {
        const bool a_is_double = a.isDouble();
        const bool b_is_double = b.isDouble();
        const bool a_is_int    = a.isInt();
        const bool b_is_int    = b.isInt();
        const bool a_is_string = a.isString();
        const bool b_is_string = b.isString();

        // // if ((a_is_int && b_is_double) || (a_is_double && b_is_int)) {
        // //     return Value(a.getDouble() + b.getDouble());
        // // }
        // if (a_is_int && b_is_double) {
        //     return Value(static_cast<double>(a.asFastInt()) + b.asFastDouble());
        // }
        // if (a_is_double && b_is_int) {
        //     return Value(a.asFastDouble() + static_cast<double>(b.asFastInt()));
        // }
        // // if (a_is_int && b_is_double) {
        // //     double result = static_cast<double>(a.asFastInt()) + b.asFastDouble();
        // //     return Value(Unsafe{}, std::bit_cast<uint64_t>(result));
        // // }
        // // if (a_is_double && b_is_int) {
        // //     double result = a.asFastDouble() + static_cast<double>(b.asFastInt());
        // //     return Value(Unsafe{}, std::bit_cast<uint64_t>(result));
        // // }



        if (a_is_string || b_is_string) [[unlikely]] {
            std::string result_str;

            if (a_is_string)      result_str = a.getStringRef();
            else if (a_is_int)    result_str = std::to_string(a.asFastInt());
            else if (a_is_double) result_str = std::to_string(a.getDouble());
            else                  result_str = "Value";

            if (b_is_string)      result_str += b.getStringRef();
            else if (b_is_int)    result_str += std::to_string(b.asFastInt());
            else if (b_is_double) result_str += std::to_string(b.getDouble());
            else                  result_str += "Value";

            return Value(result_str);
        }

        double target_a = a_is_int ? static_cast<double>(a.asFastInt()) : a.asFastDouble();
        double target_b = a_is_int ? b.asFastDouble() : static_cast<double>(b.asFastInt());

        uint64_t result_bits = std::bit_cast<uint64_t>(target_a + target_b);
        return Value(Unsafe{}, result_bits);
    }


Value Value::handleStringAdd(const DreiZehn::Value& a, const DreiZehn::Value& b) {
    const bool a_is_double = a.isDouble();
    const bool b_is_double = b.isDouble();
    const bool a_is_int    = a.isInt();
    const bool b_is_int    = b.isInt();
    const bool a_is_string = a.isString();
    const bool b_is_string = b.isString();

        std::string result_str;

        if (a_is_string)      result_str = a.getStringRef();
        else if (a_is_int)    result_str = std::to_string(a.asFastInt());
        else if (a_is_double) result_str = std::to_string(a.getDouble());
        else                  result_str = "Value";

        if (b_is_string)      result_str += b.getStringRef();
        else if (b_is_int)    result_str += std::to_string(b.asFastInt());
        else if (b_is_double) result_str += std::to_string(b.getDouble());
        else                  result_str += "Value";

        return Value(result_str);
}

const std::string Value::getTypeName(){
       if (isInt()) return "int";
       else if (isDouble()) return "double";
       else if (isString()) return "string";
       else if (isPointer()) return GetObjectTypeName(asPointerObject());
       else if(isValuePointer()) return "ValuePointer";
       else return "unknown";
   }

const void Value::print(bool appendLineFeed){
       if (isInt()) Tools::printf("%d", asInt());
       else if (isDouble()) Tools::printf("%f", asDouble());
       else if (isString()) Tools::printf("%s", getStringRef().c_str());
       else if (isPointer()) {
           ValueObject* obj = asPointerObject();
           Tools::printf("%s",  obj->toString().c_str());
       }
       else if(isValuePointer()) {
           Tools::printf("ValuePtr [%p]",  asValuePointer());
       }
       if (appendLineFeed) Tools::printf("\n");
   }
} // namespace DreiZehn
