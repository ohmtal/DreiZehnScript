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


// Marshall + onGetFieldPtr is much to compicated where !
// but i may do it with the shadows and dont need the rect.
// // struct SDL_FRect_Object: public ValueObject {
// //     SDL_FRect mRect;
// //     static constexpr uint32_t mSymCount = 4;
// //     //FIXME mSymList should be static!
// //     uint32_t mSymList[4];
// //     VariableMarshaller mMarhall;
// //     std::unordered_map<uint32_t, Value> mShadows;
// //     SDL_FRect_Object() : ValueObject(TypeSDL_FRect) {
// //         mAutoCreateDynamicFields = false;
// //        mSymList[0] =  mMarhall.registerName("x", &mRect.x);
// //        mSymList[1] =  mMarhall.registerName("y", &mRect.y);
// //        mSymList[2] =  mMarhall.registerName("w", &mRect.w);
// //        mSymList[3] =  mMarhall.registerName("h", &mRect.h);
// //        for (int i = 0 ; i <  mSymCount; i++) {
// //             mShadows[mSymList[i]] = Value(0.f);
// //        }
// //     }
// //
// //     bool onGetField(uint32_t fieldSymbolId, Value& ret) override {
// //         Value* shadow = &mShadows[fieldSymbolId];
// //         if (!shadow) return false;
// //         ret = *shadow;
// //         return true;
// //     }
// //
// //     Value* onGetFieldPtr(uint32_t fieldSymbolId) override{
// //         return &mShadows[fieldSymbolId];
// //     }
// //
// //     bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
// //         //NOTE it would work but i've broken it with using onGetFieldPtr on every assign !!!
// //         // return mMarhall.Save(fieldSymbolId,value);
// //
// //         Value* shadow = &mShadows[fieldSymbolId];
// //         if (!shadow) return false;
// //         *shadow = value;
// //         return true;
// //     }
// //
// //     void Dump() override {
// //         // Value val ;
// //         // for (size_t i = 0;  i < mSymCount; i++){
// //         //     mMarhall.Load(mSymList[i], val);
// //         //     Tools::printf("%s => %s\n",SymbolTable::getName( mSymList[i]).c_str(), val.toString().c_str()) ;
// //         // }
// //         for (size_t i = 0;  i < mSymCount; i++){
// //             const Value& val = mShadows[mSymList[i]];
// //             Tools::printf("%s => %s\n",SymbolTable::getName( mSymList[i]).c_str(), val.toString().c_str()) ;
// //         }
// //     }
// //
// //     SDL_FRect* get() {
// //         for (size_t i = 0;  i < mSymCount; i++){
// //              mMarhall.Load(mSymList[i], mShadows[mSymList[i]]);
// //         }
// //         return &mRect;
// //     }
// //
// //     static bool Construct(std::span<Value> args, Value& ret) {
// //         auto obj = new SDL_FRect_Object();
// //         ret =  Value(obj);
// //         for (size_t i = 0; i < args.size() && i < mSymCount; i++){
// //             // // obj->mMarhall.Save(obj->mSymList[i],args[i]);
// //             obj->mShadows[obj->mSymList[i]] = args[i];
// //         }
// //         return true;
// //     }
// // };

//FIXME mSymList should be static via RegisterSymbols! and i need to rework my "offset" marshall

struct SDL_BaseObject: public ValueObject {
    bool onGetField(uint32_t fieldSymbolId, Value& ret) override {
        Value* shadow = onGetFieldPtr(fieldSymbolId);
        if (!shadow) return false;
        ret = *shadow;
        return true;
    }


    bool onSetField(uint32_t fieldSymbolId, const Value& value) override {
        Value* shadow = onGetFieldPtr(fieldSymbolId);
        if (!shadow) return false;
        *shadow = value;
        return true;
    }
protected:
    SDL_BaseObject(int t): ValueObject(t) { }

};
// ---------------------
inline const int TypeSDL_FRect =  RegisterUserObjectType("SDL_FRect");
struct SDL_FRect_Object: public SDL_BaseObject {
    static constexpr uint32_t sSymCount = 4;
    inline static uint32_t sSymList[4];

    std::unordered_map<uint32_t, Value> mShadows;

    SDL_FRect_Object() : SDL_BaseObject(TypeSDL_FRect) {
        mAutoCreateDynamicFields = false;
        for (int i = 0 ; i <  sSymCount; i++) {
            mShadows[sSymList[i]] = Value(0.f);
        }
    }

    Value* onGetFieldPtr(uint32_t fieldSymbolId) override{
        return &mShadows[fieldSymbolId];
    }

    void Dump() override {
        for (size_t i = 0;  i < sSymCount; i++){
            const Value& val = mShadows[sSymList[i]];
            Tools::printf("%s => %s\n",SymbolTable::getName( sSymList[i]).c_str(), val.toString().c_str()) ;
        }
    }

    // here i would need my offset marshall !
    void get(SDL_FRect& rect) {
        rect.x = mShadows[sSymList[0]].getFloat();
        rect.y = mShadows[sSymList[1]].getFloat();
        rect.w = mShadows[sSymList[2]].getFloat();
        rect.h = mShadows[sSymList[3]].getFloat();
    }
    void set(const SDL_FRect& rect) {
        mShadows[sSymList[0]] = Value(rect.x);
        mShadows[sSymList[1]] = Value(rect.y);
        mShadows[sSymList[2]] = Value(rect.w);
        mShadows[sSymList[3]] = Value(rect.h);
    }

    static void RegisterSymbols() {
        std::string fieldnames[4] = {"x","y","w","h"};
        for (int i = 0 ; i <  sSymCount; i++) {
            sSymList[i] = SymbolTable::insert(fieldnames[i]);
            // only for help
            ValueObjectProperty(fieldnames[i], "property " + fieldnames[i], TypeSDL_FRect);
        }
    }


    static bool Construct(std::span<Value> args, Value& ret) {
        auto obj = new SDL_FRect_Object();
        ret =  Value(obj);
        for (size_t i = 0; i < args.size() && i < sSymCount; i++){
            obj->mShadows[obj->sSymList[i]] = args[i];
        }
        return true;
    }
};
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
    static bool mSymbolsLoaded = false; if (mSymbolsLoaded) return; mSymbolsLoaded = true;


    SDL_FRect_Object::RegisterSymbols();
    FunctionMap::RegisterFunction("flux::rect", [](std::span<Value> args, Value& ret) -> bool {
        return SDL_FRect_Object::Construct(args,ret);
    }, "return an new SDL_FRect");

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
