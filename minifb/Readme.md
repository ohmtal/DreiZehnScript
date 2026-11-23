# planing minifb binding 

- Small memory footprint: `cmake -B build -DMINIFB_USE_OPENGL_API=OFF -DMINIFB_USE_WAYLAND_API=OFF`

| mfb:: function | desc |
| --- | ---| 
|`obj = mfb::new W H [MFB_WF_RESIZABLE]` | create the main object with pixelbuffer |
|`state=mfb::update obj` | return the current update state should be MFB_STATE_OK| 
|`ok=mfb::running` | return bool, used in main loop | 
|`color = mfb::get obj x y` | get the color at x y  | 
|`ok = mfb::set obj x y hexColor` | set a color at x y  | 
|`mfb::text obj x y "text" scale hexColor` | write text to screen | 
|`mfb::line obj x1 y1 x2 y2  hexColor` | draw a line | 
|`mfb::rect obj x y w h hexColor` | draw a rectangle | 
|`mfb::rect obj x y r hexColor` | draw a circle | 

