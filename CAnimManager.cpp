#include "vc_anim.h"

int &CAnimManager::ms_numAnimations = *(int*)0x978540;
CAnimBlendHierarchy *CAnimManager::ms_aAnimations = (CAnimBlendHierarchy*)0x7F3D28;
int &CAnimManager::ms_numAnimBlocks = *(int*)0x94DDC4;
CAnimBlock *CAnimManager::ms_aAnimBlocks = (CAnimBlock*)0x7DB428;
CAnimBlendAssocGroup *&CAnimManager::ms_aAnimAssocGroups = *(CAnimBlendAssocGroup**)0x9B5F0C;
AnimAssocDefinition *CAnimManager::ms_aAnimAssocDefinitions = (AnimAssocDefinition*)0x6857B0;
CLinkList_CAnimBlendHierarchy *CAnimManager::ms_animCache = (CLinkList_CAnimBlendHierarchy*)0xA0D96C;

WRAPPER void CStreaming__RemoveModel(int) { EAXJMP(0x40D6E0); }

int
CAnimManager::GetNumRefsToAnimBlock(int i)
{
	return CAnimManager::ms_aAnimBlocks[i].refCount;
}

void
CAnimManager::RemoveAnimBlockRefWithoutDelete(int i)
{
	CAnimManager::ms_aAnimBlocks[i].refCount--;
}

void
CAnimManager::RemoveAnimBlockRef(int i)
{
	if(--CAnimManager::ms_aAnimBlocks[i].refCount == 0)
		CStreaming__RemoveModel(i + 0x1EEC);
}

void
CAnimManager::AddAnimBlockRef(int i)
{
	CAnimManager::ms_aAnimBlocks[i].refCount++;
}

void
CAnimManager::RemoveAnimBlock(int i)
{
	CAnimBlock *animBlock = &CAnimManager::ms_aAnimBlocks[i];
	for(int i = 0; i < 61; i++)
		if(CAnimManager::ms_aAnimAssocGroups[i].animBlock == animBlock)
			CAnimManager::ms_aAnimAssocGroups[i].DestroyAssociations();
	for(int i = 0; i < animBlock->numAnims; i++)
		CAnimManager::ms_aAnimations[animBlock->animIndex + i].Shutdown();
	animBlock->isLoaded = 0;
	animBlock->refCount = 0;
}

void
CAnimManager::RemoveLastAnimFile(void)
{
	CAnimManager::ms_numAnimBlocks--;
	CAnimManager::ms_numAnimations = CAnimManager::ms_aAnimBlocks[CAnimManager::ms_numAnimBlocks].animIndex;
	for(int i = 0; i < CAnimManager::ms_aAnimBlocks[CAnimManager::ms_numAnimBlocks].numAnims; i++)
		CAnimManager::ms_aAnimations[CAnimManager::ms_aAnimBlocks[CAnimManager::ms_numAnimBlocks].animIndex + i].Shutdown();
	CAnimManager::ms_aAnimBlocks[CAnimManager::ms_numAnimBlocks].isLoaded = 0;
}

WRAPPER void
CAnimManager::LoadAnimFile(RwStream *stream, bool a2, const char (*a3)[32])
{
	EAXJMP(0x404A50);
}

CAnimBlendAssociation*
CAnimManager::BlendAnimation(RpClump *clump, int groupId, int animId, float delta)
{
	int removePrevAnim = 0;
	int isMovement, isPartial;
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	CAnimBlendAssociation *anim = CAnimManager::ms_aAnimAssocGroups[groupId].GetAnimation(animId);
	isMovement = !!(anim->flags & 0x20);
	isPartial = !!(anim->flags & 0x10);
	void *next;
	CAnimBlendAssociation *found = NULL, *movementAnim = NULL;
	for(next = clumpData->nextAssoc; next; next = *(void**)next){
		anim = (CAnimBlendAssociation*)((void**)next - 1);
		if(isMovement && (anim->flags & 0x20))
			movementAnim = anim;
		if(anim->animId == animId)
			found = anim;
		else{
			if(isPartial == !!(anim->flags & 0x10)){
				if(anim->blendAmount <= 0.0f)
					anim->blendDelta = -1.0f;
				else{
					float x = -delta*anim->blendAmount;
					if(x < anim->blendDelta || !isPartial)
						anim->blendDelta = x;
				}
				anim->flags |= 4;
				removePrevAnim = 1;
			}
		}
	}
	if(found){
		found->blendDelta = (1.0f - found->blendAmount)*delta;
		if(!(found->flags & 1) && found->currentTime == found->hierarchy->totalLength)
			found->Start(0.0f);
	}else{
		found = CAnimManager::ms_aAnimAssocGroups[groupId].CopyAnimation(animId);
		if((found->flags & 0x20) && movementAnim){
			found->SyncAnimation(movementAnim);
			found->flags |= 1;
		}else
			found->Start(0.0f);

		void *tmp = &found->next;
		if(clumpData->nextAssoc)
			*((void**)clumpData->nextAssoc + 1) = tmp;
		*(void**)tmp = clumpData->nextAssoc;
		found->prev = clumpData;
		clumpData->nextAssoc = tmp;

		if(!removePrevAnim && !isPartial){
			found->blendAmount = 1.0f;
			return found;
		}
		found->blendAmount = 0.0f;
		found->blendDelta = delta;
	}
	CAnimManager::UncompressAnimation(found->hierarchy);
	return found;
}


CAnimBlendAssociation*
CAnimManager::AddAnimation(RpClump *clump, int groupId, int animId)
{
	CAnimBlendAssociation *anim = CAnimManager::ms_aAnimAssocGroups[groupId].CopyAnimation(animId);
	CAnimBlendClumpData *clumpData = *RWPLUGINOFFSET(CAnimBlendClumpData*, clump, ClumpOffset);
	if (anim->flags & 0x20){
		CAnimBlendAssociation *syncanim = NULL;
		void *next;
		for(next = clumpData->nextAssoc; next; next = *(void**)next){
			syncanim = (CAnimBlendAssociation*)((void**)next - 1);
			if(syncanim->flags & 0x20)
				break;
		}
		if(next){
			anim->SyncAnimation(syncanim);
			anim->flags |= 1;
		}else
			anim->Start(0.0f);
	}else
		anim->Start(0.0f);

	// insert into linked list;
	void *tmp = &anim->next;
	if(clumpData->nextAssoc)
		*((void**)clumpData->nextAssoc + 1) = tmp;
	*(void**)tmp = clumpData->nextAssoc;
	anim->prev = clumpData;
	clumpData->nextAssoc = tmp;
	return anim;
}

int
CAnimManager::RegisterAnimBlock(const char *name)
{
	CAnimBlock *animBlock = GetAnimationBlock(name);
	if(animBlock == NULL){
		animBlock = &CAnimManager::ms_aAnimBlocks[CAnimManager::ms_numAnimBlocks++];
		strncpy(animBlock->name, name, 20);
		animBlock->numAnims = 0;
	}
	return animBlock - CAnimManager::ms_aAnimBlocks;
}

void
CAnimManager::CreateAnimAssocGroups(void)
{
	for(int i = 0; i < 61; i++){
		CAnimBlock *animBlock = CAnimManager::GetAnimationBlock(CAnimManager::ms_aAnimAssocDefinitions[i].blockName);
		if(animBlock == NULL || !animBlock->isLoaded)
			continue;
		void *model = CModelInfo__ms_modelInfoPtrs[CAnimManager::ms_aAnimAssocDefinitions[i].modelIndex];
		// wtf?
		//((void (__thiscall*)(void*)) (*(void***)model)[5])(model);	// no-op
		// CClumpModelInfo::CreateInstance()
		RpClump *clump = ((RpClump* (__thiscall*)(void*)) (*(void***)model)[3])(model);
		RpAnimBlendClumpInit(clump);
		CAnimBlendAssocGroup *group = &CAnimManager::ms_aAnimAssocGroups[i];
		AnimAssocDefinition *def = &CAnimManager::ms_aAnimAssocDefinitions[i];
		group->groupId = i;
		group->baseIndex = def->animInfoList->animId;
		group->CreateAssociations(def->blockName, clump, def->animNames, def->animCount);
		for(int j = 0; j < group->numAssociations; j++)
			group->GetAnimation(def->animInfoList[j].animId)->flags |= def->animInfoList[j].flags;
		if(IsClumpSkinned(clump))
			RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, 0);
		RpClumpDestroy(clump);
	}
}

void
CAnimManager::LoadAnimFiles(void)
{
	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "ANIM\\PED.IFP");
	CAnimManager::LoadAnimFile(stream, 1, 0);
	RwStreamClose(stream, NULL);
	void *mem = gta_nw(sizeof(CAnimBlendAssocGroup) * 61 + 8);
	CAnimManager::ms_aAnimAssocGroups = (CAnimBlendAssocGroup *)construct_array(
		mem, &CAnimBlendAssocGroup::ctor, &CAnimBlendAssocGroup::DestroyAssociations,
		sizeof(CAnimBlendAssocGroup), 61);
	return CAnimManager::CreateAnimAssocGroups();
}

CAnimBlendAssociation*
CAnimManager::GetAnimAssociation(int groupId, const char *name)
{
	return CAnimManager::ms_aAnimAssocGroups[groupId].GetAnimation(name);
}

CAnimBlendAssociation*
CAnimManager::GetAnimAssociation(int groupId, int animId)
{
	return CAnimManager::ms_aAnimAssocGroups[groupId].GetAnimation(animId);
}

const char *
CAnimManager::GetAnimGroupName(int i)
{
	return CAnimManager::ms_aAnimAssocDefinitions[i].name;
}

CAnimBlock*
CAnimManager::GetAnimationBlock(const char *name)
{
	for(int i = 0; i < CAnimManager::ms_numAnimBlocks; i++)
		if(lcstrcmp(CAnimManager::ms_aAnimBlocks[i].name, name) == 0)
			return &CAnimManager::ms_aAnimBlocks[i];
	return NULL;
}

int
CAnimManager::GetAnimationBlockIndex(const char *name)
{
	for(int i = 0; i < CAnimManager::ms_numAnimBlocks; i++)
		if(lcstrcmp(CAnimManager::ms_aAnimBlocks[i].name, name) == 0)
			return i;
	return -1;
}

CAnimBlendHierarchy*
CAnimManager::GetAnimation(const char *name, CAnimBlock *animBlock)
{
	CAnimBlendHierarchy *anim = &CAnimManager::ms_aAnimations[animBlock->animIndex];
	for(int i = 0; i < animBlock->numAnims; i++){
		if(lcstrcmp(anim->name, name) == 0)
			return anim;
		anim++;
	}
	return NULL;
}

void
CAnimManager::RemoveFromUncompressedCache(CAnimBlendHierarchy *anim)
{
	if(anim->linkPtr){
		CAnimManager::ms_animCache->Remove(anim->linkPtr);
		anim->linkPtr = NULL;
	}
}

void
CAnimManager::UncompressAnimation(CAnimBlendHierarchy *anim)
{
	if(anim->compressed){
		if(anim->totalLength != 0.0f)
			anim->CalcTotalTimeCompressed();
	}else{
		if(!anim->loadSpecial){
			if(anim->linkPtr){
				anim->linkPtr->Remove();
				CAnimManager::ms_animCache->head.Insert(anim->linkPtr);
			}
		}else{
			CLink_CAnimBlendHierarchy *link = CAnimManager::ms_animCache->Insert(&anim);
			if(link == NULL){
				CAnimManager::ms_animCache->tail.prev->item->RemoveUncompressedData();
				CAnimManager::ms_animCache->Remove(CAnimManager::ms_animCache->tail.prev);
				CAnimManager::ms_animCache->tail.prev->item->linkPtr = NULL;
				link = CAnimManager::ms_animCache->Insert(&anim);
			}
			anim->linkPtr = link;
			anim->Uncompress();
		}
	}
}

void
CAnimManager::Shutdown(void)
{
	for(int i = 0; i < 35; i++)
		CStreaming__RemoveModel(i + 0x1EEC);
	for(int i = 0; i < CAnimManager::ms_numAnimations; i++)
		CAnimManager::ms_aAnimations[i].Shutdown();
	CAnimManager::ms_animCache->Shutdown();
	if(CAnimManager::ms_aAnimAssocGroups)
		destroy_array(CAnimManager::ms_aAnimAssocGroups, &CAnimBlendAssocGroup::DestroyAssociations);
}

void
CAnimManager::Initialise(void)
{
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
	CAnimManager::ms_animCache->Init(25);
}

