#include "common.h"

#ifdef RW_GL1

#include "RwHelper.h"
#include "custompipes.h"

// WorldRender namespace stubs for GL1 backend
// GL1 uses fixed function pipeline and doesn't need complex blend passes

namespace WorldRender {

// Blend instance counters
int numBlendInsts[3];

// First pass rendering - just render the atomic normally
void
AtomicFirstPass(RpAtomic *atomic, int pass)
{
	(void)pass;  // unused in GL1
	RpAtomicRender(atomic);
}

// Fully transparent rendering with fade alpha
void
AtomicFullyTransparent(RpAtomic *atomic, int pass, int fadeAlpha)
{
	(void)pass;  // unused in GL1
	(void)fadeAlpha;  // TODO: could implement alpha fade if needed
	RpAtomicRender(atomic);
}

// Render blend pass - stub for GL1
void
RenderBlendPass(int pass)
{
	(void)pass;  // no blend pass rendering in GL1 yet
	// TODO: Could implement sorted transparent object rendering if needed
}

}

#endif // RW_GL1
