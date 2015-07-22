#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <rwcore.h>
#include <rwplcore.h>
#include <rpworld.h>
#include <new>
#include "MemoryMgr.h"

typedef unsigned int uint;

void RwFreeAlign(void*);
void gtadelete(void*);
void *gta_nw(int);

void *GetModelFromName(char *name);
int IsClumpSkinned(RpClump*);

template<typename CT, typename DT> inline void*
construct_array(void *mem, CT ctor, DT dtor, int sz, int nelem)
{
	_asm{
		push	nelem
		push	sz
		push	dtor
		push	ctor
		push	mem
		mov	eax, 0x640270	// _construct_new_array
		call	eax
		add	esp, 20
	}
}


template<typename AT, typename FT> inline void
destroy_array(AT mem, FT f)
{
	_asm{
		push	f
		push	mem
		mov	eax, 0x640340	// _destroy_new_array
		call	eax
		add	esp, 8
	}
}

int gtastrcmp(const char *s1, const char *s2);
int lcstrcmp(const char *s1, const char *s2);

void RpAnimBlendClumpInit(RpClump *);

class CAnimBlendSequence;
class CAnimBlendHierarchy;
class CAnimBlock;
class CAnimBlendNode;
class CAnimBlendAssociation;
class CAnimBlendAssocGroup;
struct AnimBlendFrameData;
struct CAnimBlendClumpData;
class CAnimManager;

class CAnimBlendSequence
{
public:
	void *vtable;
	int flag;
	char name[24];
	int numFrames;
	short boneTag;
	short k;
	void *framesA;
	void *framesB;
};

class CAnimBlendHierarchy
{
public:
	char name[24];
	CAnimBlendSequence *blendSequences;
	short numSequences;
	char loadSpecial;
	char compressed;
	float totalLength;
	int m;
};

class CAnimBlock
{
public:
	char name[20];
	byte isLoaded;
	byte pad1;
	short refCount;
	int animIndex;	// into CAnimBlock::ms_aAnimations[]
	int numAnims;
};


class CAnimBlendNode
{
public:
	float a;
	int b;
	int d;	// end frame?
	int e;	// start frame?
	float f;
	CAnimBlendSequence *sequence;		// !!
	CAnimBlendAssociation *blendAssoc;	// reference to owner
};

class CAnimBlendAssociation
{
	enum Flags {
		Partial = 0x10
	};
public:
	void *vtable;
	void *next;				// pointer to next "next"
	void *prev;				// pointer to to previous variable pointing to "next"
	short numNodes;				// taken from CAnimBlendClumpData::numFrames
	short groupId;				// reference to owner
	CAnimBlendNode *nodes; 
	CAnimBlendHierarchy *hierarchy;		// !!
	float blendAmount;
	float blendDelta;
	float currentTime;
	float speed;
	float timeStep;
	short animId;
	short flags;
	int callbackType;
	void (*callback)();
	int callbackArg;

	void Init(CAnimBlendAssociation&);
	void Init(RpClump *clump, CAnimBlendHierarchy *anim);
	CAnimBlendAssociation(void);
	CAnimBlendAssociation(CAnimBlendAssociation&);
	~CAnimBlendAssociation(void);
	void ctor(void);
	void dtor(void);
	void dtor2(char flag);
};

class CAnimBlendAssocGroup
{
public:
	CAnimBlock *animBlock;			// !!
	CAnimBlendAssociation *assocList;
	int numAssociations;
	int baseIndex;	// animId of first member in assocList
	int groupId;	// own index in CAnimManager::ms_aAnimAssocGroups

	CAnimBlendAssociation *GetAnimation(uint i);
	CAnimBlendAssociation *GetAnimation(const char *name);
	CAnimBlendAssociation *CopyAnimation(uint i);
	CAnimBlendAssociation *CopyAnimation(const char *name);
	void CreateAssociations(const char *name, RpClump *clump, char **names, int numAnims);
	void CreateAssociations(const char *name);
	void DestroyAssociations(void);
	CAnimBlendAssocGroup(void);
	~CAnimBlendAssocGroup(void);
	void ctor(void);
};

struct AnimBlendFrameData
{
	int flag;
	RwV3d pos;
	RwFrame *frame;
	int nodeID;
};

struct CAnimBlendClumpData
{
	void *blendAssociation;			// pointer to CAnimBlendAssociation::next
	int b;
	int numFrames;
	int d;
	AnimBlendFrameData *frames;
};

struct AnimAssocDefinition
{
	char *name;
	char *blockName;
	int modelIndex;
	int animCount;
	char **animNames;
	void *animInfoList;
};

class CAnimManager
{
public:
	static void Initialise(void);
	static CAnimBlock *GetAnimationBlock(const char *name);
	static int GetAnimationBlockIndex(const char *name);
	static CAnimBlendHierarchy *GetAnimation(const char *name, CAnimBlock *animBlock);
	static char *GetAnimGroupName(int i);
	static CAnimBlendAssociation *GetAnimAssociation(int groupId, int animId);
	static CAnimBlendAssociation *GetAnimAssociation(int groupId, const char *name);
	static void CreateAnimAssocGroups(void);
	static void UncompressAnimation(CAnimBlendHierarchy *hier);

	static int &ms_numAnimations;
	static CAnimBlendHierarchy *ms_aAnimations;
	static int &ms_numAnimBlocks;
	static CAnimBlock *ms_aAnimBlocks;
	static CAnimBlendAssocGroup *&ms_aAnimAssocGroups;
	static AnimAssocDefinition *ms_aAnimAssocDefinitions;
};
