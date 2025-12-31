#include "common.h"

#ifdef RW_NULL

#include "custompipes.h"

// NULL platform CustomPipes stubs - webOS uses its own rendering

namespace CustomPipes {

void CreateVehiclePipe(void)
{
    // Vehicle rendering pipe - not used on NULL platform
}

void CreateWorldPipe(void)
{
    // World rendering pipe - not used on NULL platform
}

void CreateGlossPipe(void)
{
    // Gloss rendering pipe - not used on NULL platform
}

void CreateRimLightPipes(void)
{
    // Rim light pipes - not used on NULL platform
}

void DestroyVehiclePipe(void)
{
    // Vehicle pipe cleanup - not used on NULL platform
}

void DestroyWorldPipe(void)
{
    // World pipe cleanup - not used on NULL platform
}

void DestroyGlossPipe(void)
{
    // Gloss pipe cleanup - not used on NULL platform
}

void DestroyRimLightPipes(void)
{
    // Rim light pipes cleanup - not used on NULL platform
}

}

#endif
