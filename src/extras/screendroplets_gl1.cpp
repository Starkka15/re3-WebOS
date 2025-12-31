#include "common.h"

#ifdef RW_GL1

#include "rwcore.h"
#include "screendroplets.h"

// GL1 platform screen droplets stubs
// Screen droplets are an advanced visual effect that requires features not available in GLES 1.1

struct Im2DVertexUV2 {};

void openim2d_uv2(void)
{
    // 2D immediate mode with UV2 - not implemented for GL1/GLES1.1
}

void closeim2d_uv2(void)
{
    // Close 2D immediate mode - not implemented for GL1/GLES1.1
}

void RenderIndexedPrimitive_UV2(RwPrimitiveType primType, Im2DVertexUV2 *vertices, int numVertices, unsigned short *indices, int numIndices)
{
    // Indexed primitive rendering with dual UVs - not implemented for GL1/GLES1.1
    // This is used for screen droplet effects which require shader support
}

// ScreenDroplets stubs
void ScreenDroplets::Initialise(void)
{
    // Not implemented for GL1
}

#endif
