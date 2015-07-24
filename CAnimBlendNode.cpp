#include "vc_anim.h"

WRAPPER void
CAnimBlendNode::SetupKeyFrameCompressed(void)
{
	EAXJMP(0x4021C0);
}

WRAPPER bool
CAnimBlendNode::FindKeyFrame(float time)
{
	EAXJMP(0x402240);
}

void
CAnimBlendNode::Init(void)
{
	this->blendAssoc = NULL;
	this->sequence = NULL;
	this->f = 0.0f;
	this->frame1 = -1;
	this->frame0 = -1;
}
