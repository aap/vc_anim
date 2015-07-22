#include "vc_anim.h"

int &CAnimBlendAssociation_VTable = *(int*)0x68308C;

WRAPPER void CAnimBlendAssociation::Init(CAnimBlendAssociation &a) { EAXJMP(0x401820); }
WRAPPER void CAnimBlendAssociation::Init(RpClump *clump, CAnimBlendHierarchy *anim) { EAXJMP(0x4018F0); }

CAnimBlendAssociation::CAnimBlendAssociation(void) { ctor(); }
CAnimBlendAssociation::CAnimBlendAssociation(CAnimBlendAssociation &a) { ctor(); this->Init(a); }
CAnimBlendAssociation::~CAnimBlendAssociation(void) { dtor(); }

void
CAnimBlendAssociation::ctor(void)
{
	this->vtable = &CAnimBlendAssociation_VTable;
	this->groupId = -1;
	this->nodes = 0;
	this->blendAmount = 1.0f;
	this->blendDelta = 0.0f;
	this->currentTime = 0.0f;
	this->speed = 1.0f;
	this->timeStep = 0.0f;
	this->animId = -1;
	this->flags = 0;
	this->callbackType = 0;
	this->next = 0;
	this->prev = 0;
}

void
CAnimBlendAssociation::dtor(void)
{
	this->vtable = &CAnimBlendAssociation_VTable;	//?
	if(this->nodes)
		RwFreeAlign(this->nodes);
	if(this->prev)
		*(void**)this->prev = this->next;
	if(this->next)
		*((void**)this->next + 1) = this->prev;
	this->next = 0;
	this->prev = 0;
}

void
CAnimBlendAssociation::dtor2(char flag)
{
	if(this)
		if(flag & 2)
			destroy_array(this, &CAnimBlendAssociation::dtor);
		else{
			dtor();
			if(flag & 1)
				gtadelete(this);
		}
}
