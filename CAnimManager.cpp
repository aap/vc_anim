#include "vc_anim.h"

int &CAnimManager::ms_numAnimations = *(int*)0x978540;
CAnimBlendHierarchy *CAnimManager::ms_aAnimations = (CAnimBlendHierarchy*)0x7F3D28;
int &CAnimManager::ms_numAnimBlocks = *(int*)0x94DDC4;
CAnimBlock *CAnimManager::ms_aAnimBlocks = (CAnimBlock*)0x7DB428;
CAnimBlendAssocGroup *&CAnimManager::ms_aAnimAssocGroups = *(CAnimBlendAssocGroup**)0x9B5F0C;
AnimAssocDefinition *CAnimManager::ms_aAnimAssocDefinitions = (AnimAssocDefinition*)0x6857B0;

void
CAnimManager::Initialise(void)
{
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
//	return CLinkList_CAnimBlendHierarchy_Init((int)CLinkList_CAnimBlendHierarchy, 0x19u);
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

WRAPPER void
CAnimManager::UncompressAnimation(CAnimBlendHierarchy *hier) { EAXJMP(0x405AC0); }
