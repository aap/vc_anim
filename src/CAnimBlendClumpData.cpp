#include "vc_anim.h"

void
CAnimBlendClumpData::ForAllFrames(void (*cb)(AnimBlendFrameData*, void*), void *arg)
{
	for(int i = 0; i < this->numFrames; i++)
		cb(&this->frames[i], arg);
}

void
CAnimBlendClumpData::SetNumberOfBones(int n)
{
	if(this->frames)
		RwFreeAlign(this->frames);
	this->frames = (AnimBlendFrameData*)RwMallocAlign((sizeof(AnimBlendFrameData)*n + 0x3F)&~0x3F, 64);
	if(this->frameext)
		gtadelete(this->frameext);
	this->frameext = (FrameExt*)gta_nw(sizeof(FrameExt)*n);
	this->numFrames = n;
}

CAnimBlendClumpData::~CAnimBlendClumpData(void) { dtor(); }

CAnimBlendClumpData::CAnimBlendClumpData(void) { ctor(); }

void
CAnimBlendClumpData::ctor(void)
{
	this->numFrames = 0;
	this->pedPosition = NULL;
	this->frames = NULL;
	this->frameext = NULL;
	this->nextAssoc = NULL;
	this->prevAssoc = NULL;
}

void
CAnimBlendClumpData::dtor(void)
{
	if(this->prevAssoc)
		*(void**)this->prevAssoc = this->nextAssoc;
	if(this->nextAssoc)
		*((void**)this->nextAssoc + 1) = this->prevAssoc;
	this->nextAssoc = NULL;
	this->prevAssoc = NULL;
	if(this->frames)
		RwFreeAlign(this->frames);
	if(this->frameext)
		gtadelete(this->frameext);
}
