#include "vc_anim.h"

int &CAnimBlendAssociation_VTable = *(int*)0x68308C;

void
CAnimBlendAssociation::SetFinishCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg)
{
	this->callbackType = 1;
	this->callback = callback;
	this->callbackArg = arg;
}

void
CAnimBlendAssociation::SetDeleteCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg)
{
	this->callbackType = 2;
	this->callback = callback;
	this->callbackArg = arg;
}

void
CAnimBlendAssociation::SetBlend(float blendAmount, float blendDelta)
{
	this->blendAmount = blendAmount;
	this->blendDelta = blendDelta;
}

void
CAnimBlendAssociation::Start(float time)
{
	this->flags |= 1;
	this->SetCurrentTime(time);
}

CAnimBlendNode*
CAnimBlendAssociation::GetNode(int i)
{
	return &this->nodes[i];
}

void
CAnimBlendAssociation::SyncAnimation(CAnimBlendAssociation *anim)
{
	this->SetCurrentTime(anim->currentTime / anim->hierarchy->totalLength * this->hierarchy->totalLength);
}


WRAPPER void CAnimBlendAssociation::SetCurrentTime(float time) { EAXJMP(0x401770); }

void
CAnimBlendAssociation::Init(CAnimBlendAssociation &anim)
{
	this->hierarchy = anim.hierarchy;
	this->numNodes = anim.numNodes;
	this->flags = anim.flags;
	this->animId = anim.animId;
	this->groupId = anim.groupId;
	this->nodes = (CAnimBlendNode*)RwMallocAlign((sizeof(CAnimBlendNode)*this->numNodes + 0x3F)&~0x3F, 64);
	for(int i = 0; i < this->numNodes; i++){
		//this->nodes[i].Init();	// not needed
		this->nodes[i].theta0 = anim.nodes[i].theta0;
		this->nodes[i].theta1 = anim.nodes[i].theta1;
		this->nodes[i].frame0 = anim.nodes[i].frame0;
		this->nodes[i].frame1 = anim.nodes[i].frame1;
		this->nodes[i].f = anim.nodes[i].f;
		this->nodes[i].sequence = anim.nodes[i].sequence;
		this->nodes[i].blendAssoc = this;
	}
}

void
CAnimBlendAssociation::Init(RpClump *clump, CAnimBlendHierarchy *anim)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	this->numNodes = clumpData->numFrames;
	this->nodes = (CAnimBlendNode*)RwMallocAlign((sizeof(CAnimBlendNode)*this->numNodes + 0x3F)&~0x3F, 64);
	for(int i = 0; i < this->numNodes; i++){
		this->nodes[i].Init();
		this->nodes[i].blendAssoc = this;
	}
	this->hierarchy = anim;
	AnimBlendFrameData *frameData;
	for(int i = 0; i < anim->numSequences; i++){
		CAnimBlendSequence *seq = &anim->blendSequences[i];
		if(seq->boneTag == -1)
			frameData = RpAnimBlendClumpFindFrame(clump, seq->name);
		else
			frameData = RpAnimBlendClumpFindBone(clump, seq->boneTag);
		if(frameData && seq->numFrames > 0)
			this->nodes[frameData - clumpData->frames].sequence = seq;
	}
}

CAnimBlendAssociation::CAnimBlendAssociation(void) { ctor(); }
CAnimBlendAssociation::CAnimBlendAssociation(CAnimBlendAssociation &a)
{
	this->vtable = &CAnimBlendAssociation_VTable;
	this->nodes = NULL;
	this->blendAmount = 1.0f;
	this->blendDelta = 0.0f;
	this->currentTime = 0.0f;
	this->speed = 1.0f;
	this->timeStep = 0.0f;
	this->callbackType = 0;
	this->next = NULL;
	this->prev = NULL;
	this->Init(a);
}
CAnimBlendAssociation::~CAnimBlendAssociation(void) { dtor(); }

bool
CAnimBlendAssociation::UpdateBlend(float timeDelta)
{
	this->blendAmount += this->blendDelta * timeDelta;
	if(isnan(this->blendAmount) || this->blendAmount > 0.0f || this->blendDelta >= 0.0f)
		goto xyz;
	this->blendAmount = 0.0f;
	if(this->blendDelta < 0.0f)
		this->blendDelta = 0.0f;
	if(this->flags & 4){
		if(this->callbackType == 1 || this->callbackType == 2)
			this->callback(this, this->callbackArg);
		if(this)
			// destructor (dtor2)
			(*(void(__thiscall**)(CAnimBlendAssociation*, int))this->vtable)(this, 1);
		return 0;
	}else{
xyz:
		if(this->blendAmount > 1.0f){
			this->blendAmount = 1.0f;
			if(this->blendDelta > 0.0f)
				this->blendDelta = 0.0f;
		}
		return 1;
	}
}

void
CAnimBlendAssociation::UpdateTime(float f1, float f2)
{
	if((this->flags & 1) == 0)
		return;
	if(this->currentTime >= this->hierarchy->totalLength){
		this->flags &= ~1;
		return;
	}
	this->currentTime += this->timeStep;
	if(this->currentTime >= this->hierarchy->totalLength){
		if(this->flags & 2)
			this->currentTime -= this->hierarchy->totalLength;
		else{
			this->currentTime = this->hierarchy->totalLength;
			if(this->flags & 8){
				this->flags |= 4;
				this->blendDelta = -4.0f;
			}
			if(this->callbackType == 1){
				this->callbackType = 0;
				this->callback(this, this->callbackArg);
			}
		}
	}
}

void
CAnimBlendAssociation::ctor(void)
{
	this->vtable = &CAnimBlendAssociation_VTable;
	this->groupId = -1;
	this->nodes = NULL;
	this->blendAmount = 1.0f;
	this->blendDelta = 0.0f;
	this->currentTime = 0.0f;
	this->speed = 1.0f;
	this->timeStep = 0.0f;
	this->animId = -1;
	this->flags = 0;
	this->callbackType = 0;
	this->next = NULL;
	this->prev = NULL;
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
