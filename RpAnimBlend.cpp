#include "vc_anim.h"

int &ClumpOffset = *(int*)0x978798;

WRAPPER void RpAnimBlendClumpInit(RpClump *) { EAXJMP(0x407890); }
WRAPPER AnimBlendFrameData *RpAnimBlendClumpFindFrame(RpClump *clump, char *name) { EAXJMP(0x407BB0); }
WRAPPER AnimBlendFrameData *RpAnimBlendClumpFindBone(RpClump *clump, int tag) { EAXJMP(0x407B60); }
