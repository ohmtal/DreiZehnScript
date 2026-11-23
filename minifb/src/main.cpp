#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <MiniFB.h>

#include "core/Types.h"
#include "toolbox/PixelBuffer.h"


// see also >>> MiniFB_enums.h <<<
// Window Flags:
// -------------
// MFB_WF_RESIZABLE          = 0x01,
// MFB_WF_FULLSCREEN         = 0x02,
// MFB_WF_FULLSCREEN_DESKTOP = 0x04,
// MFB_WF_BORDERLESS         = 0x08,
// MFB_WF_ALWAYS_ON_TOP      = 0x10,

// Update states:
// -------------
// MFB_STATE_OK             =  0,
// MFB_STATE_EXIT           = -1,
// MFB_STATE_INVALID_WINDOW = -2,
// MFB_STATE_INVALID_BUFFER = -3,
// MFB_STATE_INTERNAL_ERROR = -4,


using namespace DreiZehn;

U32 W = 640;
U32 H = 400;

struct PixelObject : public PixelBuffer::pixel_buffer {
private:
    bool initialized = false;
    struct mfb_window *window = nullptr;
    mfb_update_state state;
public:
    size_t pixelcount = 0;

    bool Init (const char* caption, U32 w, U32 h, U32 flags = MFB_WF_RESIZABLE) {
        window = mfb_open_ex(caption, w, h, flags);
        if (!window) return false;
        width = w;
        height = h;
        pixelcount = w * h;
        buffer = new uint32_t[pixelcount]();
        if (!buffer) return false;
        initialized = true;
        return true;
    }
    ~PixelObject() {
        window = nullptr;
        if (buffer) delete[] buffer;
        buffer = nullptr;
        initialized = false;
    }
    void clear (U32 color) {
          std::fill(buffer, buffer + pixelcount, color);
    }
    static U32 rgb(U8 r, U8 g, U8 b) {
        return MFB_RGB(r,g,b);
    }

    size_t getCount() {return pixelcount;}

    const mfb_update_state& Update() {
        if (!initialized) {
            state =  MFB_STATE_INTERNAL_ERROR;
        } else {
            state =  mfb_update_ex(window, buffer, width, height);
        }
        return state;
    }
    bool Loop() {
        if (!initialized) return false;
        return mfb_wait_sync(window);
    }

    void Line( U32 x1, U32 y1,U32 x2,U32 y2, U32 color ){
        line(this,x1,y1,x2,y2,color);
    }
    void Rect( U32 x, U32 y,U32 w,U32 h, U32 color ){
        rect(this,x,y,w,h,color);
    }
    void Circle(U32 x, U32 y, U32 r, U32 color) {
        circle(this,x,y,r,color);
    }
    void Text(U32 x, U32 y, const char *caption, U32 scale, U32 color) {
        text(this, x, y, caption, scale, color);
    }




};

PixelObject Pixels;


void DrawOverlay() {
    Pixels.Line(0,0, W, H, Pixels.rgb(180, 200, 255));
    Pixels.Line(0,H, W, 0, Pixels.rgb(180, 200, 255));
    Pixels.Rect(50, 60, W - 100,H - 100, Pixels.rgb(0, 0, 200));
    Pixels.Circle((U32)(W / 2), (U32)(H / 2), (U32)(H/3), Pixels.rgb(180, 0, 0));
    Pixels.Circle((U32)(W / 2), (U32)(H / 2), (U32)(H/4), Pixels.rgb(0, 160, 160));
    Pixels.Text(10,10, "* Pixels *", 12 , 0);
    Pixels.Text(11,11, "* Pixels *", 12 , 0x00FFFF);
}

int main() {

    if (!Pixels.Init("Hello FB",W,H)) return -1;
    Pixels.clear(Pixels.rgb(30,30,30));



    size_t      i;
    uint32_t    noise, carry, seed = 0xbeef;
    do {

        for (i = 0; i < Pixels.pixelcount; ++i) {
            noise = seed;
            noise >>= 3;
            noise ^= seed;
            carry = noise & 1;
            noise >>= 1;
            seed >>= 1;
            seed |= (carry << 30);
            noise &= 0xFF;
            Pixels.buffer[i] = noise;
            // Pixels.buffer[i] = MFB_RGB(0, noise, 0);
        }
        DrawOverlay();

        if (Pixels.Update() != MFB_STATE_OK) break;
    } while(Pixels.Loop());


    return 0;
}
