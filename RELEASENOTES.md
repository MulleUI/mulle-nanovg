## 0.1.0

* add color emoji/ClearType support and external texture ownership across backends

* Vulkan backend can now wrap caller-owned textures (`renderDefineTexture`, `renderForgetTexture`, `renderGetTextureInfo`) and gains ``NVG_IMAGE_NODELETE`` so external resources are never destroyed
* Metal backend exposes ClearType and color-emoji create flags `(`NVG_CLEARTYPE`,` ``NVG_EMOJI`)`
* emoji texture rendering now premultiplies alpha, fixing blending on antialiased edges
* font fallback prefers a color font over an earlier monochrome fallback for emoji-range codepoints
* unified shader-type numbering (SIMPLE=2, IMG=3, CLEARTYPE=4, EMOJI=5) across GL, DirectX 11, Metal and Vulkan backends
* port ClearType + color Emoji text shaders to Metal, Vulkan, D3D11


### 0.0.1

Various small improvements