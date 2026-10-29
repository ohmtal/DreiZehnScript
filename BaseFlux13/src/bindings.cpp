#include <string>
#include <format>
#include <SDL3/SDL.h>
#include "BaseFlux/Main.h"
#include "BaseFlux/Draw.h"
#include "BaseFlux/Collision.h"
#include "ColorConstants.h"
#include "core/VectorFunctions.h"

#include "Globals.h"
#include <span>
#include "core/FunctionMap.h"
#include "core/ValueMarshaller.h"

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
        color.r =  vec->mElements[0].getU8();
        color.g =  vec->mElements[1].getU8();
        color.b =  vec->mElements[2].getU8();
        color.a =  vec->mElements[3].getU8();
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
void RegisterBaseFluxFunctions(Environment& env) {

    // StructMarshaller<SDL_Point> SDL_PointMarshall;
    // SDL_PointMarshall.registerField(&SDL_Point::x);
    // SDL_PointMarshall.registerField(&SDL_Point::y);

    // ValueMarshaller<BaseFlux::Settings> SettingsMarshall;
    // SettingsMarshall.registerField(&BaseFlux::Settings);
    // SettingsMarshall.registerField(&BaseFlux::Settings::EnableVSync)
    // uint32_t smyId = SymbolTable::insert("flux::EnableVSync");
    // Globals::Marshall->registerSymbol(smyId, &app.getSettings().EnableVSync);
    // Globals::Va

    // NOTE THIS ONLY WORKS WITH Interpreter!!
    // VariableMarshaller SettingsMarshall;
    // uint32_t symId = SymbolTable::insert("flux::EnableVSync");
    // SettingsMarshall.registerSymbol(symId, &app.getSettings().EnableVSync);
    // static Value valEnableVSync;
    // Globals::MasterVariableFrame->setVariable(symId,Value(&valEnableVSync),true );
    // SettingsMarshall.Load(symId, valEnableVSync);

    FunctionMap::RegisterFunction("flux::size", [](std::span<Value> args, Value& ret) -> bool {
        int logiX = 0;
        int logiY = 0;

        if (args.size() >= 2) {
            SDL_Point oldSize = app.getSettings().ScreenSize;
            app.getSettings().ScreenSize.x = args[0].getInt();
            app.getSettings().ScreenSize.y = args[1].getInt();
            if (oldSize.x != app.getSettings().ScreenSize.x
                || oldSize.y != app.getSettings().ScreenSize.y) {
                SDL_SetWindowSize(app.getWindow(), app.getSettings().ScreenSize.x , app.getSettings().ScreenSize.y);
            }

            if (args.size() >= 4) {
                logiX = args[2].getInt();
                logiY = args[3].getInt();
                SDL_SetRenderLogicalPresentation(app.getRenderer(), logiX, logiY ,
                                                 SDL_LOGICAL_PRESENTATION_STRETCH);
                                                 // SDL_LOGICAL_PRESENTATION_LETTERBOX);
            }

        }
         SDL_RendererLogicalPresentation mode;
        SDL_GetRenderLogicalPresentation(app.getRenderer(),&logiX, &logiY, &mode);
        auto obj = new VectorValueObject();
        obj->mElements.push_back(Value(app.getSettings().ScreenSize.x));
        obj->mElements.push_back(Value(app.getSettings().ScreenSize.y));
        obj->mElements.push_back(Value(logiX));
        obj->mElements.push_back(Value(logiY));
        ret = Value (obj);

        return true;
    }, "set / get the screen size and logical size");
    // ---------------------------------
    FunctionMap::RegisterFunction("flux::maxfps", [](std::span<Value> args, Value& ret) -> bool {
        DirectVariableAccessor vsyncer(&app.getSettings().FpsLimit);
        if (args.size() == 1) {
            vsyncer.toCpp(args[0]);
        }
        vsyncer.toValue(ret);
        return true;
    }, "set / get the max fps");
    // ---------------------------------
    FunctionMap::RegisterFunction("flux::Close", [](std::span<Value> args, Value& ret) -> bool {
        app.TerminateApplication(); return true;
    }, "Close the Application");
    // ---------------------------------
//     void DrawDebugText( SDL_Renderer *renderer,
//                         float x, float y, const char* text,
//                         float scale = 1.f, const SDL_Color& color = {100,100,100,255},
//                         bool doShadow = false, const SDL_Color& shadowColor = { 60,60,60,255});
//
    FunctionMap::RegisterFunction("flux::Text", [](std::span<Value> args, Value& ret) -> bool {
        if (args.size() < 3 || !args[2].isString()) {
            Tools::PrintRuntimeError("flux::Text: Missing Parameter");
            return false;
        }
        float x = args[0].getFloat();
        float y = args[1].getFloat();
        const char* text = args[2].getStringChars();

        float scale = 1.f;
        SDL_Color color = {200,200,200,255};
        bool doShadow = false;
        SDL_Color shadowcolor = {60,60,60,255};

        if (args.size() > 3)
            scale = args[3].getFloat();

        if (args.size() > 4)
            MapVectorColor(args[4], color);

        if (args.size() > 5)
            doShadow = args[5].getBool();

        if (args.size() > 6)
            MapVectorColor(args[6], shadowcolor);

        BaseFlux::DrawDebugText(app.getRenderer(),x,y,text, scale, color, doShadow, shadowcolor);

        return true;
    }, "Draw a Text on the Screen. x,y,text optional: scale,color,doShadow,shadowColor");


    // ---------------------------------
    FunctionMap::RegisterFunction("flux::DrawRect", [](std::span<Value> args, Value& ret) -> bool {
        if (args.size() < 3 || !args[0].isPointer() || !args[1].isPointer()) {
            Tools::PrintRuntimeError("flux::DrawRect: Missing Parameter");
            return false;
        }
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

        float fontSize  = 1.0;
        if (args.size() > 2) fontSize = args[2].getFloat();


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

         BaseFlux::DrawDebugText(app.getRenderer(),x,y,fps_text.c_str(), fontSize, color, true/*, shadowColor*/);
        return true;
    }, "Draw FPS as position x y ");

} // RegisterBaseFluxFunctions
