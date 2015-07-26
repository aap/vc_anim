#include "vc_anim.h"

WRAPPER RwFrame *FrameForAllChildrenCountCallBack(RwFrame*, void*) { EAXJMP(0x407980); }
WRAPPER RpAtomic *GetFirstAtomic(RpClump*) { EAXJMP(0x57F980); }
WRAPPER RpHAnimHierarchy *GetAnimHierarchyFromSkinClump(RpClump*) { EAXJMP(0x57F250); }
WRAPPER void SkinGetBonePositionsToTable(RpClump *clump, RwV3d boneTab[]) { EAXJMP(0x57EF00); }
WRAPPER const char *GetFrameNodeName(RwFrame *frame) { EAXJMP(0x580600); }
WRAPPER const char *ConvertBoneTag2BoneName(int tag) { EAXJMP(0x405E70); }
WRAPPER int ConvertPedNode2BoneTag(int node) { EAXJMP(0x405DE0); }
WRAPPER int CVisibilityPlugins__GetFrameHierarchyId(RwFrame*) { EAXJMP(0x581810); }

WRAPPER void FrameUpdateCallbackNoRender(AnimBlendFrameData *frame, void *arg) { EAXJMP(0x4042A0); }
//WRAPPER void FrameUpdateCallBackSkinned(AnimBlendFrameData *frame, void *arg) { EAXJMP(0x4042D0); }
WRAPPER void FrameUpdateCallBackNonSkinned(AnimBlendFrameData *frame, void *arg) { EAXJMP(0x403700); }

int &ClumpOffset = *(int*)0x978798;

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if(RpAnimBlendClumpIsInitialized(clump) && clumpData->nextAssoc)
		return (CAnimBlendAssociation*)((void**)clumpData->nextAssoc - 1);
	return NULL;
}

CAnimBlendClumpData *&pAnimClumpToUpdate = *(CAnimBlendClumpData**)0x6F7000;

void
RpAnimBlendClumpCheckKeyFrames(AnimBlendFrameData *bones, CAnimBlendNode **nodes, int numNodes)
{
	for(CAnimBlendNode **node = nodes+1; *node; node++){
		CAnimBlendAssociation *a = (*node)->blendAssoc;
		for(int i = 0; i < numNodes; i++)
			if((!(bones[i].flag & 8) || !pAnimClumpToUpdate->d) && (*node)[i].sequence)
					(*node)[i].FindKeyFrame(a->currentTime - a->timeStep);
	}
}


WRAPPER void
FrameUpdateCallBackSkinnedWith3dVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes) { EAXJMP(0x4039D0); }
/*
WRAPPER void
FrameUpdateCallBackSkinnedWithVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes) { EAXJMP(0x403DF0); }
*/

void
FrameUpdateCallBackSkinnedWithVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes)
{
	CVector vec, pos;
	CQuaternion q, rot;
	float totalBlendAmount = 0.0f;
	float x = 0.0f, y = 0.0f;
	float curx = 0.0f, cury = 0.0f;
	float endx = 0.0f, endy = 0.0f;
	rot.x = rot.y = rot.z = rot.w = 0.0f;
	pos.x = pos.y = pos.z = 0.0f;
	bool looped = false;
	float *frameData = (float*)frame->frame;
	CAnimBlendNode **node;

	if(*nodes){
		node = nodes+1;
		do{
			if((*node)->sequence && ((*node)->blendAssoc->flags & CAnimBlendAssociation::Partial))
				totalBlendAmount += (*node)->blendAssoc->blendAmount;
			node++;
		}while(*node);
	}

	node = nodes+1;
	do{
		if((*node)->sequence && (*node)->sequence->flag & 2 && (*node)->blendAssoc->flags & 0x40){
			(*node)->GetCurrentTranslation(&vec, 1.0f-totalBlendAmount);
			cury += vec.y;
			if((*node)->blendAssoc->flags & 0x80)
				curx += vec.x;
		}
		node++;
	}while(*node);

	node = nodes+1;
	do{
		if((*node)->sequence){
			bool nodelooped = (*node)->Update(vec, q, 1.0f-totalBlendAmount);
			if(q.x*rot.x + q.y*rot.y + q.z*rot.z + q.w*rot.w < 0.0f){
				rot.x -= q.x;
				rot.y -= q.y;
				rot.z -= q.z;
				rot.w -= q.w;
			}else{
				rot.x += q.x;
				rot.y += q.y;
				rot.z += q.z;
				rot.w += q.w;
			}
			if((*node)->sequence->flag & 2){
				pos.x += vec.x;
				pos.y += vec.y;
				pos.z += vec.z;
				if((*node)->blendAssoc->flags & 0x40){
					y += vec.y;
					if((*node)->blendAssoc->flags & 0x80)
						x += vec.x;
					looped |= nodelooped;
					if(nodelooped){
						(*node)->GetEndTranslation(&vec, 1.0f-totalBlendAmount);
						endy += vec.y;
						if((*node)->blendAssoc->flags & 0x80)
							endx += vec.x;
					}
				}
			}
		}
		++*node;
		node++;
	}while(*node);

	if(!(frame->flag & 2)){
		float norm = rot.x*rot.x + rot.y*rot.y + rot.z*rot.z + rot.w*rot.w;
		if(norm == 0.0f)
			rot.w = 1.0f;
		else{
			float r = 1.0f/sqrt(norm);
			rot.x *= r;
			rot.y *= r;
			rot.z *= r;
			rot.w *= r;
		}
		frameData[2] = rot.x;
		frameData[3] = rot.y;
		frameData[4] = rot.z;
		frameData[5] = rot.w;
	}

	if(!(frame->flag & 4)){
		pAnimClumpToUpdate->d[0] = x - curx;
		pAnimClumpToUpdate->d[1] = y - cury;
		if(looped){
			pAnimClumpToUpdate->d[0] += endx;
			pAnimClumpToUpdate->d[1] += endy;
		}
		frameData[6] = pos.x - x;
		frameData[7] = pos.y - y;
		frameData[8] = pos.z;
		if(frameData[8] >= -0.8f)
			if(frameData[8] >= -0.4f)
				frameData[8] += frame->pos.z;
			else
				frameData[8] += (2.5 * frameData[8] + 2.0f) * frame->pos.z;
		frameData[6] += frame->pos.x;
		frameData[7] += frame->pos.y;
	}	
}

void
FrameUpdateCallBackSkinned(AnimBlendFrameData *frame, void *arg)
{
	CAnimBlendNode **nodes = (CAnimBlendNode**)arg;
	CVector vec, pos;
	CQuaternion q, rot;
	float totalBlendAmount = 0.0f, posBlendAmount = 0.0f;
	rot.x = rot.y = rot.z = rot.w = 0.0f;
	pos.x = pos.y = pos.z = 0.0f;
	float *frameData = (float*)frame->frame;
	CAnimBlendNode **node;

	if (frame->flag & 8 && pAnimClumpToUpdate->d){
		if(frame->flag & 0x10)
			FrameUpdateCallBackSkinnedWith3dVelocityExtraction(frame, nodes);
		else
			FrameUpdateCallBackSkinnedWithVelocityExtraction(frame, nodes);
		return;
	}

	if(*nodes){
		node = nodes+1;
		do{
			if((*node)->sequence && ((*node)->blendAssoc->flags & CAnimBlendAssociation::Partial))
				totalBlendAmount += (*node)->blendAssoc->blendAmount;
			node++;
		}while(*node);
	}

	node = nodes+1;
	do{
		if((*node)->sequence){
			bool nodelooped = (*node)->Update(vec, q, 1.0f-totalBlendAmount);
			if(q.x*rot.x + q.y*rot.y + q.z*rot.z + q.w*rot.w < 0.0f){
				rot.x -= q.x;
				rot.y -= q.y;
				rot.z -= q.z;
				rot.w -= q.w;
			}else{
				rot.x += q.x;
				rot.y += q.y;
				rot.z += q.z;
				rot.w += q.w;
			}
			if((*node)->sequence->flag & 2){
				pos.x += vec.x;
				pos.y += vec.y;
				pos.z += vec.z;
				posBlendAmount += (*node)->blendAssoc->blendAmount;
			}
		}
		++*node;
		node++;
	}while(*node);

	if(!(frame->flag & 2)){
		float norm = rot.x*rot.x + rot.y*rot.y + rot.z*rot.z + rot.w*rot.w;
		if(norm == 0.0f)
			rot.w = 1.0f;
		else{
			float r = 1.0f/sqrt(norm);
			rot.x *= r;
			rot.y *= r;
			rot.z *= r;
			rot.w *= r;
		}
		frameData[2] = rot.x;
		frameData[3] = rot.y;
		frameData[4] = rot.z;
		frameData[5] = rot.w;
	}

	if(!(frame->flag & 4)){
		frameData[6] = posBlendAmount * pos.x;
		frameData[7] = posBlendAmount * pos.y;
		frameData[8] = posBlendAmount * pos.z;
		frameData[6] += frame->pos.x * (1.0f - posBlendAmount);
		frameData[7] += frame->pos.y * (1.0f - posBlendAmount);
		frameData[8] += frame->pos.z * (1.0f - posBlendAmount);
	}
}

void
checkNodes(CAnimBlendNode *nodes[])
{
	int i = 1;
	CAnimBlendNode **node = nodes+1;
	if(*node == NULL)
		printf("I think i'll crash now: %d %p\n", i, nodes);
}


void
RpAnimBlendClumpUpdateAnimations(RpClump *clump, float timeDelta, bool doRender)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	float totalLength = 0.0f;
	float totalBlend = 0.0f;
	pAnimClumpToUpdate = clumpData;

	CAnimBlendNode *nodes[40];
	int j = 0;
	nodes[0] = 0;
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->UpdateBlend(timeDelta))
			if(a->hierarchy->blendSequences){
				CAnimManager::UncompressAnimation(a->hierarchy);
				if(j < 11)
					nodes[j++ + 1] = a->GetNode(0);
				if(a->flags & CAnimBlendAssociation::Movement){
					totalLength += a->hierarchy->totalLength / a->speed * a->blendAmount;
					totalBlend += a->blendAmount;
				}else
					nodes[0] = (CAnimBlendNode*)1;
			}
	}

	if(!nodes[1] || !j)
		_asm int 3

	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->flags & 1){
			float f;
			if(a->flags & CAnimBlendAssociation::Movement)
				f = (totalLength == 0.0f ? 1.0f : totalBlend / totalLength) * a->hierarchy->totalLength;
			else
				f = a->speed;
			a->timeStep = f * timeDelta;
		}
	}

	nodes[j + 1] = NULL;
	if(doRender){
		if(clumpData->frames[0].flag & CAnimBlendAssociation::Movement)
			RpAnimBlendClumpCheckKeyFrames(clumpData->frames, nodes, clumpData->numFrames);
		if(IsClumpSkinned(clump))
			clumpData->ForAllFrames(FrameUpdateCallBackSkinned, nodes);
		else
			clumpData->ForAllFrames(FrameUpdateCallBackNonSkinned, nodes);
//		clumpData->ForAllFrames(IsClumpSkinned(clump) ? FrameUpdateCallBackSkinned : FrameUpdateCallBackNonSkinned, nodes);
		clumpData->frames->flag &= 0xDF;
	}else{
		clumpData->ForAllFrames(FrameUpdateCallbackNoRender, nodes);
		clumpData->frames->flag |= 0x20;
	}

	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		a->UpdateTime(timeDelta, totalLength == 0.0f ? 1.0f : totalBlend / totalLength);
	}
	RwFrameUpdateObjects(RpClumpGetFrame(clump));
}

void
RpAnimBlendAllocateData(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = (CAnimBlendClumpData*)gta_nw(sizeof(CAnimBlendClumpData));
	if(clumpData)
		clumpData->ctor();
	*RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset) = clumpData;
}

void
RpAnimBlendClumpDestroy(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if(clumpData){
		RpAnimBlendClumpRemoveAllAssociations(clump);
		clumpData->dtor();
		gtadelete(clumpData);
		*RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset) = NULL;
	}
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc, uint mask)
{
	for(void *link = assoc->next; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->flags & mask)
			return a;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc)
{
	if(assoc->next)
		return (CAnimBlendAssociation*)((void**)assoc->next - 1);
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(RpClump *clump, uint mask)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->flags & mask)
			return a;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation_N(RpClump *clump, int n)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	int i = 0;
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(i == n && (a->flags & CAnimBlendAssociation::Partial))
			return a;
		i++;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation_N(RpClump *clump, int n)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	int i = 0;
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(i == n && (a->flags & CAnimBlendAssociation::Partial) == 0)
			return a;
		i++;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	CAnimBlendAssociation *retval = NULL;
	float maxBlend = 0.0f;
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->blendAmount > maxBlend){
			maxBlend = a->blendAmount;
			retval = a;
		}
	}
	return retval;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation(RpClump *clump, CAnimBlendAssociation **outAssoc, float *outFloat)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	CAnimBlendAssociation *retval = NULL, *assoc2 = NULL;
	float acc1 = 0.0f, acc2 = 0.0f;
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->flags & CAnimBlendAssociation::Partial)
			continue;
		if(a->blendAmount <= acc2){
			if(a->blendAmount > acc1){
				acc1 = a->blendAmount;
				assoc2 = a;
			}
		}else{
			acc1 = acc2;
			acc2 = a->blendAmount;
			assoc2 = retval;
			retval = a;
		}
	}
	if(outAssoc)
		*outAssoc = assoc2;
	if(*outFloat)
		*outFloat = acc1;
	return retval;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetAssociation(RpClump *clump, uint id)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(a->animId == id)
			return a;
	}
	return NULL;
}

void
RpAnimBlendClumpRemoveAssociations(RpClump *clump, uint mask)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	void *next;
	for(void *link = clumpData->nextAssoc; link; link = next){
		next = *(void**)link;
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(mask == 0 || (a->flags & mask))
			if(a)
				(*(void(__thiscall**)(CAnimBlendAssociation*, int))a->vtable)(a, 1);
	}
}

void
RpAnimBlendClumpRemoveAllAssociations(RpClump *clump)
{
	RpAnimBlendClumpRemoveAssociations(clump, 0);
}

void
RpAnimBlendClumpSetBlendDeltas(RpClump *clump, uint mask, float delta)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	for(void *link = clumpData->nextAssoc; link; link = *(void**)link){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		if(mask == 0 || (a->flags & mask))
			a->blendDelta = delta;
	}
}

bool
RpAnimBlendClumpIsInitialized(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	return clumpData && clumpData->numFrames != 0;
}

RwFrame*
FrameForAllChildrenFillFrameArrayCallBack(RwFrame *frame, void *arg)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	(*frames)->frame = frame;
	++*frames;
	RwFrameForAllChildren(frame, FrameForAllChildrenFillFrameArrayCallBack, arg);
	return frame;
}

void FrameInitCallBack(AnimBlendFrameData *frameData, void*)
{
	frameData->flag = 0;
	RwV3d *pos = &frameData->frame->modelling.pos;
	frameData->pos.x = pos->x;
	frameData->pos.y = pos->y;
	frameData->pos.z = pos->z;
	frameData->nodeID = -1;
}

void
RpAnimBlendClumpInit(RpClump *clump)
{
	if(IsClumpSkinned(clump))
		RpAnimBlendClumpInitSkinned(clump);
	else{
		RpAnimBlendAllocateData(clump);
		CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
		RwFrame *frame = RpClumpGetFrame(clump);
		int numFrames = 0;
		RwFrameForAllChildren(frame, FrameForAllChildrenCountCallBack, &numFrames);
		clumpData->SetNumberOfBones(numFrames);
		AnimBlendFrameData *frames = clumpData->frames;
		RwFrameForAllChildren(frame, FrameForAllChildrenFillFrameArrayCallBack, &frames);
		clumpData->ForAllFrames(FrameInitCallBack, NULL);
		clumpData->frames[0].flag |= 8;
	}
}

void
ZeroFlag(AnimBlendFrameData *frame, void*)
{
	frame->flag = 0;
}

void
RpAnimBlendClumpInitSkinned(RpClump *clump)
{
	RwV3d boneTab[64];
	RpAnimBlendAllocateData(clump);
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	RpAtomic *atomic = GetFirstAtomic(clump);
	RpSkin *skin = RpSkinGeometryGetSkin(atomic->geometry);
	RwUInt32 numBones = RpSkinGetNumBones(skin);
	clumpData->SetNumberOfBones(numBones);
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
	memset(boneTab, 0, sizeof(boneTab));
	SkinGetBonePositionsToTable(clump, boneTab);
	AnimBlendFrameData *frames = clumpData->frames;
	for(int i = 0; i < numBones; i++){
		frames[i].nodeID = hier->pNodeInfo[i].nodeID;
		frames[i].pos = boneTab[i];
		// wtf?
		frames[i].frame = (RwFrame *)((char *)&hier->currentAnim[1] + hier->currentAnim->currentKeyFrameSize * i);
	}
	clumpData->ForAllFrames(ZeroFlag, NULL);
	clumpData->frames[0].flag |= 8;
}

void
FillFrameArrayCallback(AnimBlendFrameData *frame, void *arg)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	frames[CVisibilityPlugins__GetFrameHierarchyId(frame->frame)] = frame;
}

void
RpAnimBlendClumpFillFrameArray(RpClump *clump, AnimBlendFrameData **frames)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if(IsClumpSkinned(clump)){
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
		for(int i = 1; i < 18; i++)
		      frames[i] = &clumpData->frames[RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(i))];
	}else
		clumpData->ForAllFrames(FillFrameArrayCallback, frames);
}

static AnimBlendFrameData *foundFrame;

void
FrameFindByIndexCallback(AnimBlendFrameData *frame, void *arg)
{
	uint tag = *(uint*)arg;
	if(frame->nodeID == tag)
		foundFrame = frame;
}

AnimBlendFrameData*
RpAnimBlendClumpFindBone(RpClump *clump, uint tag)
{
	foundFrame = NULL;
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	clumpData->ForAllFrames(FrameFindByIndexCallback, &tag);
	return foundFrame;
}

void
FrameFindByNameCallbackSkinned(AnimBlendFrameData *frame, void *arg)
{
	const char *name = ConvertBoneTag2BoneName(frame->nodeID);
	if(name && gtastrcmp(name, (char*)arg) == 0)
		foundFrame = frame;
}

void
FrameFindByNameCallback(AnimBlendFrameData *frame, void *arg)
{
	const char *name = GetFrameNodeName(frame->frame);
	if(gtastrcmp(name, (char*)arg) == 0)
		foundFrame = frame;
}

AnimBlendFrameData*
RpAnimBlendClumpFindFrame(RpClump *clump, const char *name)
{
	foundFrame = NULL;
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	clumpData->ForAllFrames(IsClumpSkinned(clump) ? FrameFindByNameCallbackSkinned : FrameFindByNameCallback, (void*)name);
	return foundFrame;
}