#include "vc_anim.h"

WRAPPER void CLink_CAnimBlendHierarchy::Insert(CLink_CAnimBlendHierarchy*) { EAXJMP(0x405D10); }
WRAPPER void CLink_CAnimBlendHierarchy::Remove(void) { EAXJMP(0x405D30); }

WRAPPER void CLinkList_CAnimBlendHierarchy::Init(int n) { EAXJMP(0x405C20); }
WRAPPER void CLinkList_CAnimBlendHierarchy::Shutdown(void) { EAXJMP(0x405C90); }
WRAPPER CLink_CAnimBlendHierarchy *CLinkList_CAnimBlendHierarchy::Insert(CAnimBlendHierarchy**) { EAXJMP(0x405CB0); }
WRAPPER void CLinkList_CAnimBlendHierarchy::Remove(CLink_CAnimBlendHierarchy*) { EAXJMP(0x405CF0); }

void
CAnimBlendHierarchy::RemoveUncompressedData(void)
{
	this->loadSpecial = 1;
}

WRAPPER void
CAnimBlendHierarchy::Uncompress(void)
{
	EAXJMP(0x401C80);
}

void
CAnimBlendHierarchy::CalcTotalTimeCompressed(void)
{
	EAXJMP(0x401D00);
}

WRAPPER void CAnimBlendHierarchy::Shutdown(void) { EAXJMP(0x401F00); }
