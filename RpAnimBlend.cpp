#include "vc_anim.h"

WRAPPER RwFrame *FrameForAllChildrenCountCallBack(RwFrame*, void*) { EAXJMP(0x407980); }
WRAPPER RpAtomic *GetFirstAtomic(RpClump*) { EAXJMP(0x57F980); }
WRAPPER RpHAnimHierarchy *GetAnimHierarchyFromSkinClump(RpClump*) { EAXJMP(0x57F250); }
WRAPPER const char *GetFrameNodeName(RwFrame *frame) { EAXJMP(0x580600); }
WRAPPER const char *ConvertBoneTag2BoneName(int tag) { EAXJMP(0x405E70); }
WRAPPER int ConvertPedNode2BoneTag(int node) { EAXJMP(0x405DE0); }
WRAPPER int CVisibilityPlugins__GetFrameHierarchyId(RwFrame*) { EAXJMP(0x581810); }
WRAPPER RpAtomic *RpClumpGetFirstAtomicCB(RpAtomic*, void*) { EAXJMP(0x57F9B0); }
WRAPPER RpAtomic *RpClumpGetSkinAnimHierarchyCB(RpAtomic*, void*) { EAXJMP(0x57F280); }

int &ClumpOffset = *(int*)0x978798;
CAnimBlendClumpData *&gpAnimBlendClump = *(CAnimBlendClumpData**)0x6F7000;

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(RpClump *clump)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if(RpAnimBlendClumpIsInitialized(clump) && clumpData->nextAssoc)
		return (CAnimBlendAssociation*)((void**)clumpData->nextAssoc - 1);
	return NULL;
}

void
RpAnimBlendClumpCheckKeyFrames(AnimBlendFrameData *bones, CAnimBlendNode **nodes, int numNodes)
{
	for(CAnimBlendNode **node = nodes+1; *node; node++){
		CAnimBlendAssociation *a = (*node)->blendAssoc;
		for(int i = 0; i < numNodes; i++)
			if((!(bones[i].flag & 8) || !gpAnimBlendClump->d) && (*node)[i].sequence)
					(*node)[i].FindKeyFrame(a->currentTime - a->timeStep);
	}
}

void
RpAnimBlendClumpUpdateAnimations(RpClump *clump, float timeDelta, bool doRender)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	float totalLength = 0.0f;
	float totalBlend = 0.0f;
	gpAnimBlendClump = clumpData;

	CAnimBlendNode *nodes[12];
	int j = 0;
	nodes[0] = 0;
	void *next;
	for(void *link = clumpData->nextAssoc; link; link = next){
		CAnimBlendAssociation *a = (CAnimBlendAssociation*)((void**)link - 1);
		// have to get next pointer before calling UpdateBlend()
		next = *(void**)link;
		if(!a->UpdateBlend(timeDelta) || a->hierarchy->blendSequences == NULL)
			continue;
		CAnimManager::UncompressAnimation(a->hierarchy);
		if(j < 11)
			nodes[++j] = a->GetNode(0);
		if(a->flags & CAnimBlendAssociation::Movement){
			totalLength += a->hierarchy->totalLength / a->speed * a->blendAmount;
			totalBlend += a->blendAmount;
		}else
			nodes[0] = (CAnimBlendNode*)1;
	}

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

	nodes[++j] = NULL;
	if(doRender){
		if(clumpData->frames[0].flag & CAnimBlendAssociation::Movement)
			RpAnimBlendClumpCheckKeyFrames(clumpData->frames, nodes, clumpData->numFrames);
		clumpData->ForAllFrames(IsClumpSkinned(clump) ? FrameUpdateCallBackSkinned : FrameUpdateCallBackNonSkinned, nodes);
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
SkinGetBonePositionsToTable(RpClump *clump, RwV3d *boneTable)
{
	const RwMatrix *mats;
	RwMatrix m, invmat;
	int stack[32];
	if(boneTable == NULL)
		return;

	RpAtomic *atomic = NULL;
	RpClumpForAllAtomics(clump, RpClumpGetFirstAtomicCB, &atomic);
	RpSkin *skin = RpSkinGeometryGetSkin(atomic->geometry);
	RpHAnimHierarchy *hier = NULL;
	RpClumpForAllAtomics(clump, RpClumpGetSkinAnimHierarchyCB, &hier);
	boneTable[0].x = boneTable[0].y = boneTable[0].z = 0.0f;
	RwUInt32 numBones = RpSkinGetNumBones(skin);
	RwV3d *out = boneTable+1;
	int j = 0;
	int sp = 0;
	for(uint i = 1; i < numBones; i++){
		mats = RpSkinGetSkinToBoneMatrices(skin);
		RwMatrixCopy(&m, &mats[i]);
		RwMatrixInvert(&invmat, &m);
		RwV3dTransformPoints(out++, &invmat.pos, 1, &mats[j]);
		if(hier->pNodeInfo[i].flags & 2)
			stack[++sp] = j;
		if(hier->pNodeInfo[i].flags & 1)
			j = stack[sp--];
		else
			j = i;
	}
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
		frames[i].hanimframe = (RpHAnimKeyFrame*)rtANIMGETINTERPFRAME(hier->currentAnim, i);
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