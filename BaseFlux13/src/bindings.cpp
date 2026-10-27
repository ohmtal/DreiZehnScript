#include <string>
#include <format>

#include "BaseFlux/Main.h"
#include "BaseFlux/Draw.h"
#include "BaseFlux/Collision.h"
#include "ColorConstants.h"
#include "functions/VectorFunctions.h"

#include "Globals.h"
#include <span>
#include "core/FunctionMap.h"

extern BaseFlux::Main app;
extern SDL_Point gMousePos;
using namespace DreiZehn;
typedef SDL_Color Color;

// -----------------------------------------------------------------------------
VectorValueObject* GetVectorObject(const Value& value, size_t minSize ){
    if (!value.isPointer()
        || value.asPointerObject()->mType != TypeVectorObject)
    return nullptr;
    VectorValueObject* result = static_cast<VectorValueObject*> (value.asPointerObject());
    if (result->mElements.size() < minSize) return nullptr;
    return result;
}
// -----------------------------------------------------------------------------
bool MapVectorFRect(const Value& value, SDL_FRect& rect) {
    VectorValueObject* vec = GetVectorObject(value, 4);
    if (vec) {
        rect.x = vec->mElements[0].getFloat();
        rect.y = vec->mElements[1].getFloat();
        rect.w = vec->mElements[2].getFloat();
        rect.h = vec->mElements[3].getFloat();
        if (rect.w == 0.f || rect.h == 0.f) return false;
        return true;
    }
    return false;
}
// -----------------------------------------------------------------------------
bool MapVectorColor(const Value& value, SDL_Color& color) {
    VectorValueObject* vec = GetVectorObject(value, 4);
    if (vec) {
        color.r = static_cast<uint8_t>( vec->mElements[0].getInt());
        color.g = static_cast<uint8_t>( vec->mElements[1].getInt());
        color.b = static_cast<uint8_t>( vec->mElements[2].getInt());
        color.a = static_cast<uint8_t>( vec->mElements[3].getInt());
        return true;
    }
    return false;
}
// -----------------------------------------------------------------------------
void RegisterBaseFluxFunctions(Environment& env) {

    // ---------------------------------
    FunctionMap::RegisterFunction("flux::Close", [](std::span<Value> args, Value& ret) -> bool {
        app.TerminateApplication(); return true;
    }, "Close the Application");


    // ---------------------------------
    FunctionMap::RegisterFunction("flux::DrawRect", [](std::span<Value> args, Value& ret) -> bool {
//NOTE FIXME ERROR MESSAGES
        if (args.size() < 3 || !args[0].isPointer() || !args[1].isPointer()) return false;

        SDL_FRect rect;
        if (!MapVectorFRect(args[0], rect)) return false;
        SDL_Color color;
        if  (!MapVectorColor(args[1], color)) return false;

        bool fill = args[2].getBool();

        BaseFlux::DrawRect(app.getRenderer(),rect, color, fill);
        return true;
    }, "Draw Rectangle Vector 4 floats, Vector 4 int, bool fill");
    // ---------------------------------
    FunctionMap::RegisterFunction("flux::DrawFPS", [](std::span<Value> args, Value& ret) -> bool {
        if (args.size() < 2) return false;

        float x = args[0].getFloat();
        float y = args[1].getFloat();

        SDL_Color color = GREEN;
        U32 fps =  BaseFlux::getFPS();

        static std::string fps_text = []{
            std::string s;
            s.reserve(32);
            return s;
        }();
        fps_text.clear();
        std::format_to(std::back_inserter(fps_text), "{} fps", fps);


        if (fps < 15) color = RED;
        else if (fps < 30) color = ORANGE;

         BaseFlux::DrawDebugText(app.getRenderer(),x,y,fps_text.c_str(), 1.0f, color, true/*, shadowColor*/);
        return true;
    }, "Draw FPS as position x y ");

} // RegisterBaseFluxFunctions
