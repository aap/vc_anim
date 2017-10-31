#include "vc_anim.h"

WRAPPER RwFrame *FrameForAllChildrenCountCallBack(RwFrame*, void*) { EAXJMP(0x407980); }
WRAPPER RpAtomic *GetFirstAtomic(RpClump*) { EAXJMP(0x57F980); }
WRAPPER RpHAnimHierarchy *GetAnimHierarchyFromSkinClump(RpClump*) { EAXJMP(0x57F250); }
WRAPPER const char *GetFrameNodeName(RwFrame *frame) { EAXJMP(0x580600); }
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
			if((!(bones[i].updateFlag & CAnimBlendAssociation::Flag8) || gpAnimBlendClump->pedPosition == NULL) && (*node)[i].sequence)
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
		if(clumpData->frames[0].updateFlag & CAnimBlendAssociation::Movement)
			RpAnimBlendClumpCheckKeyFrames(clumpData->frames, nodes, clumpData->numFrames);
		clumpData->ForAllFrames(IsClumpSkinned(clump) ? FrameUpdateCallBackSkinned : FrameUpdateCallBackNonSkinned, nodes);
		clumpData->frames->updateFlag &= ~CAnimBlendAssociation::Movement;
	}else{
		clumpData->ForAllFrames(FrameUpdateCallbackNoRender, nodes);
		clumpData->frames->updateFlag |= CAnimBlendAssociation::Movement;
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


enum PedNode {
	PED_Spine1 = 1,
	PED_Head,
	PED_L_UpperArm,
	PED_R_UpperArm,
	PED_L_Hand,
	PED_R_Hand,
	PED_L_Thigh,
	PED_R_Thigh,
	PED_L_Foot,
	PED_R_Foot,
	PED_R_Calf,
	PED_L_Calf,
	PED_L_Forearm,
	PED_R_Forearm,
	PED_L_Clavicle,
	PED_R_Clavicle,
	PED_Neck
};

enum BoneTag {
	BONE_Root = 0,
	BONE_Pelvis = 1,
	BONE_Spine = 2,
	BONE_Spine1 = 3,
	BONE_Neck = 4,
	BONE_Head = 5,
	BONE_R_Clavicle = 21,
	BONE_R_UpperArm = 22,
	BONE_R_Forearm = 23,
	BONE_R_Hand = 24,
	BONE_R_Fingers = 25,
	BONE_L_Clavicle = 31,
	BONE_L_UpperArm = 32,
	BONE_L_Forearm = 33,
	BONE_L_Hand = 34,
	BONE_L_Fingers = 35,
	BONE_L_Thigh = 41,
	BONE_L_Calf = 42,
	BONE_L_Foot = 43,
	BONE_R_Thigh = 51,
	BONE_R_Calf = 52,
	BONE_R_Foot = 53,
};

const char*
ConvertBoneTag2BoneName(int tag)
{
	switch(tag){
	case BONE_Root: return "Root";
	case BONE_Pelvis: return "Pelvis";
	case BONE_Spine: return "Spine";
	case BONE_Spine1: return "Spine1";
	case BONE_Neck: return "Neck";
	case BONE_Head: return "Head";
	case BONE_R_Clavicle: return "Bip01 R Clavicle";
	case BONE_R_UpperArm: return "R UpperArm";
	case BONE_R_Forearm: return "R Forearm";
	case BONE_R_Hand: return "R Hand";
	case BONE_R_Fingers: return "R Fingers";
	case BONE_L_Clavicle: return "Bip01 L Clavicle";
	case BONE_L_UpperArm: return "L UpperArm";
	case BONE_L_Forearm: return "L Forearm";
	case BONE_L_Hand: return "L Hand";
	case BONE_L_Fingers: return "L Fingers";
	case BONE_L_Thigh: return "L Thigh";
	case BONE_L_Calf: return "L Calf";
	case BONE_L_Foot: return "L Foot";
	case BONE_R_Thigh: return "R Thigh";
	case BONE_R_Calf: return "R Calf";
	case BONE_R_Foot: return "R Foot";
	default: return NULL;
	}
}

int
ConvertPedNode2BoneTag(int node)
{
	switch(node){
	case PED_Spine1: return BONE_Spine1;
	case PED_Head: return BONE_Head;
	case PED_L_UpperArm: return BONE_L_UpperArm;
	case PED_R_UpperArm: return BONE_R_UpperArm;
	case PED_L_Hand: return BONE_L_Hand;
	case PED_R_Hand: return BONE_R_Hand;
	case PED_L_Thigh: return BONE_L_Thigh;
	case PED_R_Thigh: return BONE_R_Thigh;
	case PED_L_Foot: return BONE_L_Foot;
	case PED_R_Foot: return BONE_R_Foot;
	case PED_R_Calf: return BONE_R_Calf;
	case PED_L_Calf: return BONE_L_Calf;
	case PED_L_Forearm: return BONE_L_Forearm;
	case PED_R_Forearm: return BONE_R_Forearm;
	case PED_L_Clavicle: return BONE_L_Clavicle;
	case PED_R_Clavicle: return BONE_R_Clavicle;
	case PED_Neck: return BONE_Neck;
	default: return -1;
	}
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

static RwFrame*
FrameForAllChildrenFillFrameArrayCallBack(RwFrame *frame, void *arg)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	(*frames)->frame = frame;
	++*frames;
	RwFrameForAllChildren(frame, FrameForAllChildrenFillFrameArrayCallBack, arg);
	return frame;
}

static void
FrameInitCallBack(AnimBlendFrameData *frameData, void*)
{
	frameData->updateFlag = 0;
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
		clumpData->frames[0].updateFlag |= CAnimBlendAssociation::Flag8;
	}
}

void
ZeroFlag(AnimBlendFrameData *frame, void*)
{
	frame->updateFlag = 0;
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
	int parent = 0;
	int sp = 0;
	for(uint i = 1; i < numBones; i++){
		mats = RpSkinGetSkinToBoneMatrices(skin);
		RwMatrixCopy(&m, &mats[i]);
		RwMatrixInvert(&invmat, &m);

		RwV3dTransformPoints(out++, &invmat.pos, 1, &mats[parent]);
		if(hier->pNodeInfo[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[++sp] = parent;
		if(hier->pNodeInfo[i].flags & rpHANIMPOPPARENTMATRIX)
			parent = stack[sp--];
		else
			parent = i;
	}
}

void
SkinGetBoneMatricesToTable(RpClump *clump, RwMatrix *matTable)
{
	const RwMatrix *mats;
	RwMatrix invmat;
	int stack[32];
	if(matTable == NULL)
		return;

	RpAtomic *atomic = NULL;
	RpClumpForAllAtomics(clump, RpClumpGetFirstAtomicCB, &atomic);
	RpSkin *skin = RpSkinGeometryGetSkin(atomic->geometry);
	RpHAnimHierarchy *hier = NULL;
	RpClumpForAllAtomics(clump, RpClumpGetSkinAnimHierarchyCB, &hier);
	RwMatrixSetIdentity(&matTable[0]);
	RwUInt32 numBones = RpSkinGetNumBones(skin);
	RwMatrix *out = matTable+1;
	int parent = 0;
	int sp = 0;
	for(uint i = 1; i < numBones; i++){
		mats = RpSkinGetSkinToBoneMatrices(skin);
		RwMatrixInvert(&invmat, &mats[i]);
		RwMatrixMultiply(out++, &invmat, &mats[parent]);

		if(hier->pNodeInfo[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[++sp] = parent;
		if(hier->pNodeInfo[i].flags & rpHANIMPOPPARENTMATRIX)
			parent = stack[sp--];
		else
			parent = i;
	}
}

RtQuat
RwMatrixExtractRotation(RwMatrix *m)
{
	RtQuat q = { { 0.0f, 0.0f, 0.0f }, 1.0f };
	float tr = m->right.x + m->up.y + m->at.z;
	float s;
	if(tr > 0.0f){
		s = sqrt(1.0f + tr) * 2.0f;
		q.real = s / 4.0f;
		q.imag.x = (m->up.z - m->at.y) / s;
		q.imag.y = (m->at.x - m->right.z) / s;
		q.imag.z = (m->right.y - m->up.x) / s;
	}else if(m->right.x > m->up.y && m->right.x > m->at.z){
		s = sqrt(1.0f + m->right.x - m->up.y - m->at.z) * 2.0f;
		q.real = (m->up.z - m->at.y) / s;
		q.imag.x = s / 4.0f;
		q.imag.y = (m->up.x + m->right.y) / s;
		q.imag.z = (m->at.x + m->right.z) / s;
	}else if(m->up.y > m->at.z){
		s = sqrt(1.0f + m->up.y - m->right.x - m->at.z) * 2.0f;
		q.real = (m->at.x - m->right.z) / s;
		q.imag.x = (m->up.x + m->right.y) / s;
		q.imag.y = s / 4.0f;
		q.imag.z = (m->at.y + m->up.z) / s;
	}else{
		s = sqrt(1.0f + m->at.z - m->right.x - m->up.y) * 2.0f;
		q.real = (m->right.y - m->up.x) / s;
		q.imag.x = (m->at.x + m->right.z) / s;
		q.imag.y = (m->at.y + m->up.z) / s;
		q.imag.z = s / 4.0f;
	}
	return q;
}

void
RpAnimBlendClumpInitSkinned(RpClump *clump)
{
	RwV3d boneTab[64];
	RwMatrix matTab[64];
	RpAnimBlendAllocateData(clump);
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	RpAtomic *atomic = GetFirstAtomic(clump);
	RpSkin *skin = RpSkinGeometryGetSkin(atomic->geometry);
	RwUInt32 numBones = RpSkinGetNumBones(skin);
	clumpData->SetNumberOfBones(numBones);
	RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
	memset(boneTab, 0, sizeof(boneTab));
	SkinGetBonePositionsToTable(clump, boneTab);
	SkinGetBoneMatricesToTable(clump, matTab);

	AnimBlendFrameData *frames = clumpData->frames;
	for(uint i = 0; i < numBones; i++){
		frames[i].nodeID = hier->pNodeInfo[i].nodeID;
		frames[i].pos = boneTab[i];
//		frames[i].pos = matTab[i].pos;
		frames[i].hanimframe = (RpHAnimKeyFrame*)rtANIMGETINTERPFRAME(hier->currentAnim, i);

		frames[i].hanimframe->t = matTab[i].pos;
		frames[i].hanimframe->q = RwMatrixExtractRotation(&matTab[i]);

#ifdef FRAMEEXT
		clumpData->frameext[i].pos = matTab[i].pos;
		clumpData->frameext[i].rot = RwMatrixExtractRotation(&matTab[i]);
#endif
	}
	clumpData->ForAllFrames(ZeroFlag, NULL);
	clumpData->frames[0].updateFlag |= CAnimBlendAssociation::Flag8;
}

void
FillFrameArrayCallback(AnimBlendFrameData *frame, void *arg)
{
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	frames[CVisibilityPlugins__GetFrameHierarchyId(frame->frame)] = frame;
}

// Fills a frame array inside CPed
void
RpAnimBlendClumpFillFrameArray(RpClump *clump, AnimBlendFrameData **frames)
{
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if(IsClumpSkinned(clump)){
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(clump);
		for(int i = PED_Spine1; i <= PED_Neck; i++)
		      frames[i] = &clumpData->frames[RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(i))];
	}else
		clumpData->ForAllFrames(FillFrameArrayCallback, frames);
}

static AnimBlendFrameData *&foundFrame = *(AnimBlendFrameData**)0x6F7180;

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