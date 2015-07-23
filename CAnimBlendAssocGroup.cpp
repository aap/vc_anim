#include "vc_anim.h"

CAnimBlendAssociation*
CAnimBlendAssocGroup::GetAnimation(uint i)
{
	return &this->assocList[i - this->baseIndex];
}

CAnimBlendAssociation*
CAnimBlendAssocGroup::GetAnimation(const char *name)
{
	for(int i = 0; i < this->numAssociations; i++)
		if(gtastrcmp(this->assocList[i].hierarchy->name, name) == 0)
			return &this->assocList[i];
	return NULL;
}

CAnimBlendAssociation*
CAnimBlendAssocGroup::CopyAnimation(uint i)
{
	CAnimBlendAssociation *anim;
	anim = &this->assocList[i - this->baseIndex];
	if(anim){
		CAnimManager::UncompressAnimation(anim->hierarchy);
		CAnimBlendAssociation *copy = (CAnimBlendAssociation *)gta_nw(sizeof(CAnimBlendAssociation));
		if(copy)
			copy = new (copy) CAnimBlendAssociation(*anim);
		return copy;
	}
	return NULL;
}

CAnimBlendAssociation*
CAnimBlendAssocGroup::CopyAnimation(const char *name)
{
	CAnimBlendAssociation *anim = GetAnimation(name);
	if(anim){
		CAnimManager::UncompressAnimation(anim->hierarchy);
		CAnimBlendAssociation *copy = (CAnimBlendAssociation *)gta_nw(sizeof(CAnimBlendAssociation));
		if(copy)
			copy = new (copy) CAnimBlendAssociation(*anim);
		return copy;
	}
	return NULL;
}

void
CAnimBlendAssocGroup::CreateAssociations(const char *name, RpClump *clump, char **names, int numAnims)
{
	DestroyAssociations();

	this->animBlock = CAnimManager::GetAnimationBlock(name);
	void *mem = gta_nw(sizeof(CAnimBlendAssociation) * numAnims + 8);
	this->assocList = (CAnimBlendAssociation *)construct_array(
		mem, &CAnimBlendAssociation::ctor, &CAnimBlendAssociation::dtor,
		sizeof(CAnimBlendAssociation), numAnims);
	this->numAssociations = 0;

	for(int i = 0; i < numAnims; i++){
		CAnimBlendHierarchy *anim = CAnimManager::GetAnimation(names[i], this->animBlock);
		this->assocList[i].Init(clump, anim);
		this->assocList[i].animId = this->baseIndex + i;
		this->assocList[i].groupId = this->groupId;
	}
	this->numAssociations = numAnims;
}

void
CAnimBlendAssocGroup::CreateAssociations(const char *name)
{
	this->DestroyAssociations();

	this->animBlock = CAnimManager::GetAnimationBlock(name);
	int numAnims = this->animBlock->numAnims;
	void *mem = gta_nw(sizeof(CAnimBlendAssociation) * numAnims + 8);
	this->assocList = (CAnimBlendAssociation *)construct_array(
		mem, &CAnimBlendAssociation::ctor, &CAnimBlendAssociation::dtor,
		sizeof(CAnimBlendAssociation), numAnims);
	this->numAssociations = 0;

	for(int i = 0; i < numAnims; i++){
		CAnimBlendHierarchy *anim = &CAnimManager::ms_aAnimations[this->animBlock->animIndex + i];
		void *model = GetModelFromName(anim->name);
		if(model){
			// wtf?
			// CClumpModelInfo::CreateInstance()
			RpClump *clump = ((RpClump* (__thiscall*)(void*)) (*(void***)model)[3])(model);
			RpAnimBlendClumpInit(clump);
			this->assocList[i].Init(clump, anim);
			if(IsClumpSkinned(clump))
				RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, 0);
			RpClumpDestroy(clump);
			this->assocList[i].animId = this->baseIndex + i;
			this->assocList[i].groupId = this->groupId;
		}
	}
	this->numAssociations = numAnims;
}

void
CAnimBlendAssocGroup::DestroyAssociations(void)
{
	if(this->assocList){
		destroy_array(this->assocList, &CAnimBlendAssociation::dtor);
		this->assocList = 0;
		this->numAssociations = 0;
	}
}

CAnimBlendAssocGroup::CAnimBlendAssocGroup(void) { ctor(); }
CAnimBlendAssocGroup::~CAnimBlendAssocGroup(void) { DestroyAssociations(); }

void
CAnimBlendAssocGroup::ctor(void)
{
	this->animBlock = 0;
	this->assocList = 0;
	this->numAssociations = 0;
	this->baseIndex = 0;
	this->groupId = -1;
}
