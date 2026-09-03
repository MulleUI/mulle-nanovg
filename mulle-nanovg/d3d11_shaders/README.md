# D3D11 Shaders

The `.h` files here contain **precompiled** DXBC bytecode (produced by Microsoft's
`fxc` HLSL compiler, shader model `ps_4_0`). They are consumed directly by
`nanovg_d3d11.c` via `CreatePixelShader` / `CreateVertexShader`.

## ClearType + Emoji support

The pixel shaders were extended to match the OpenGL/Vulkan/Metal backends with
two additional shader types:

- `type == 4` — **ClearType** subpixel-antialiased text (uses `ddx`)
- `type == 5` — **Emoji** color-glyph text

The HLSL source for the pixel shaders is provided here:

- `D3D11PixelShader.hlsl`   (no anti-aliasing)
- `D3D11PixelShaderAA.hlsl` (anti-aliasing)

> NOTE: The vertex shader (`D3D11VertexShader.h`) did not change.

## Regenerating the bytecode headers (Windows only)

`fxc` ships with the Windows SDK and is required to reproduce the exact
`ps_4_0` DXBC format the backend expects. Run from a Developer Command Prompt:

```bat
fxc /T ps_4_0 /E main /Fh D3D11PixelShader.h   /Vn g_D3D11PixelShader_Main   D3D11PixelShader.hlsl
fxc /T ps_4_0 /E main /Fh D3D11PixelShaderAA.h /Vn g_D3D11PixelShaderAA_Main D3D11PixelShaderAA.hlsl
```

The `.hlsl` sources were validated with `dxc -T ps_6_0` on a non-Windows host,
but the shipped `.h` bytecode still needs to be regenerated with `fxc` on
Windows to pick up the ClearType/Emoji branches. The C-side enum
(`D3DNVGshaderType`) and the `renderTriangles` dispatch in `nanovg_d3d11.c`
already select types 4 and 5.
