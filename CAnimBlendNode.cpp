#include "vc_anim.h"

void
CAnimBlendNode::Init(void)
{
	this->blendAssoc = NULL;
	this->sequence = NULL;
	this->f = 0.0f;
	this->frame1 = -1;
	this->frame0 = -1;
}
