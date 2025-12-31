#include "common.h"

#ifdef RW_NULL

#include "rwcore.h"
#include "Renderer.h"

// NULL platform WorldRender stubs - webOS uses its own rendering

namespace WorldRender {

int numBlendInsts = 0;

bool AtomicFullyTransparent(rw::Atomic *atomic, int pass, int flags)
{
    return false;
}

void AtomicFirstPass(rw::Atomic *atomic, int pass)
{
    // First pass rendering - not used on NULL platform
}

void RenderBlendPass(int pass)
{
    // Blend pass rendering - not used on NULL platform
}

}

#endif
