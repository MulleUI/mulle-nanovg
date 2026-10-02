### 0.1.1

feature: add platform-specific backend routing and Android support

* Metal, DirectX 11 and Vulkan backend sources are now selected automatically per platform
* backend public headers (GL, rlgl, Metal, D3D11, Vulkan) are classified and installed individually
* Android is documented as supported, with the library cross-compiling via `mulle-sde craft --platform android`
* add `android/debug.sh` to attach the NDK lldb to the running demo on an emulator

## 0.1.0



feature: add color emoji/ClearType support and external texture ownership across backends

* Vulkan backend can now wrap caller-owned textures (`renderDefineTexture`, `renderForgetTexture`, `renderGetTextureInfo`) and gains ``NVG_IMAGE_NODELETE`` so external resources are never destroyed
* Metal backend exposes ClearType and color-emoji create flags `(`NVG_CLEARTYPE`,` ``NVG_EMOJI`)`
* emoji texture rendering now premultiplies alpha, fixing blending on antialiased edges
* font fallback prefers a color font over an earlier monochrome fallback for emoji-range codepoints
* unified shader-type numbering (SIMPLE=2, IMG=3, CLEARTYPE=4, EMOJI=5) across GL, DirectX 11, Metal and Vulkan backends









  add type 4/5 branches to shader/fill.frag, regenerate fill.frag.inc (SPIR-V).
  both fragmentShader and fragmentShaderAA, regenerate `mnvg_bitcode/macos.h.`
  D3D11PixelShader(.AA).hlsl sources with the branches (fxc regen on Windows).
