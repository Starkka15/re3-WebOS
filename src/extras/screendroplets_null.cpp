#include "common.h"

#ifdef RW_NULL

#include "rwcore.h"

// NULL platform screen droplets stubs

struct Im2DVertexUV2 {};

void openim2d_uv2(void)
{
    // 2D immediate mode with UV2 - not used on NULL platform
}

void closeim2d_uv2(void)
{
    // Close 2D immediate mode - not used on NULL platform
}

void RenderIndexedPrimitive_UV2(RwPrimitiveType primType, Im2DVertexUV2 *vertices, int numVertices, unsigned short *indices, int numIndices)
{
    // Indexed primitive rendering - not used on NULL platform
}

#endif
