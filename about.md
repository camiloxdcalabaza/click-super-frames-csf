# Click Super Frames Optimizer (meme) (real)

**The real replacement for Click Between Frames.**

CBF is dead. 400k downloads but broken on 2.2091 and no mobile. This is the fix.

### What it does - No visual bloat

- **CBF but with more than unlimited inputs:** Hooks `GJBaseGameLayer::handleButton` directly, bypassing `CCEGLView` 60hz limiter. 2 fast physical clicks = 3 logical clicks in the same tick. N inputs per gap, not 1.
- **Draw Divide:** Real dt scaling, not forced `1/240`. No duplicated draws. Less draw calls = less lag.
- **PC Selector:**
  - **Patata 0.75x:** 120 FPS real for potato PCs
  - **Normal 1.0x:** 240 FPS real, 360 if you have 240hz monitor
  - **Cheta up to 2.0x:** 480 FPS real, customizable

### Why it's 1000x better than CBF

CBF: 1 click between 2 frames, WndProc thread, stutter on 144/165hz, PC only.
CSF: N clicks between ALL frames, no thread, Hz cache every 2s, PC & Mobile, 2.2081 & 2.2091.

No particles. No shaders. No 92939 visual things. 1 file only.

### Performance

- No syscall per frame (Hz cached every 2s)
- Real dt * speed, not forced
- Atomic speed, no race condition
- Cooldown on notifications

Created by camiloxdcalabaza
