#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <rwcore.h>
#include <rwplcore.h>
#include <rpworld.h>
#include <rpskin.h>
#include <new>
#include "MemoryMgr.h"

typedef unsigned int uint;

extern void **&RwEngineInst;

void *RwMallocAlign(uint size, int alignment);
void RwFreeAlign(void*);
void gtadelete(void*);
void *gta_nw(int);

void *GetModelFromName(char *name);
int IsClumpSkinned(RpClump*);
RpAtomic *AtomicRemoveAnimFromSkinCB(RpAtomic*, void*);

extern void **CModelInfo__ms_modelInfoPtrs;

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

class CQuaternion {
public:
	float x, y, z, w;

	void Slerp(CQuaternion &q1, CQuaternion &q2, float theta0, float theta1, float r);
	void Get(RwMatrix *mat);
	void Add(CQuaternion &q) {
		this->x += q.x;
		this->y += q.y;
		this->z += q.z;
		this->w += q.w;
	};
	void Sub(CQuaternion &q) {
		this->x -= q.x;
		this->y -= q.y;
		this->z -= q.z;
		this->w -= q.w;
	};
	void Mult(float r) {
		this->x *= r;
		this->y *= r;
		this->z *= r;
		this->w *= r;
	}
};

class CVector {
public:
	float x, y, z;

	void Add(CVector &v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
	};
};

class CAnimBlendSequence;
class CAnimBlendHierarchy;
class CAnimBlock;
class CAnimBlendNode;
class CAnimBlendAssociation;
class CAnimBlendAssocGroup;
struct AnimBlendFrameData;
class CAnimBlendClumpData;
class CAnimManager;

void FrameUpdateCallbackNoRender(AnimBlendFrameData *frame, void *arg);
void FrameUpdateCallBackNonSkinned(AnimBlendFrameData *frame, void *arg);
void FrameUpdateCallBackSkinned(AnimBlendFrameData *frame, void *arg);

extern int &ClumpOffset;
extern CAnimBlendClumpData *&gpAnimBlendClump;
CAnimBlendAssociation *RpAnimBlendClumpGetFirstAssociation(RpClump *clump);
void RpAnimBlendClumpCheckKeyFrames(AnimBlendFrameData *bones, CAnimBlendNode **nodes, int numBones);
void RpAnimBlendClumpUpdateAnimations(RpClump *clump, float timeDelta, bool doRender);
void RpAnimBlendAllocateData(RpClump *clump);
void RpAnimBlendClumpDestroy(RpClump *clump);
CAnimBlendAssociation *RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc, uint mask);
CAnimBlendAssociation *RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc);
CAnimBlendAssociation *RpAnimBlendClumpGetFirstAssociation(RpClump *clump, uint mask);
CAnimBlendAssociation *RpAnimBlendClumpGetMainPartialAssociation_N(RpClump *clump, int n);
CAnimBlendAssociation *RpAnimBlendClumpGetMainAssociation_N(RpClump *clump, int n);
CAnimBlendAssociation *RpAnimBlendClumpGetMainPartialAssociation(RpClump *clump);
CAnimBlendAssociation *RpAnimBlendClumpGetMainAssociation(RpClump *clump, CAnimBlendAssociation **, float *);
CAnimBlendAssociation *RpAnimBlendClumpGetAssociation(RpClump *clump, uint mask);
void RpAnimBlendClumpRemoveAssociations(RpClump *clump, uint mask);
void RpAnimBlendClumpRemoveAllAssociations(RpClump *clump);
void RpAnimBlendClumpSetBlendDeltas(RpClump *clump, uint mask, float delta);
bool RpAnimBlendClumpIsInitialized(RpClump *clump);
void RpAnimBlendClumpInit(RpClump *);
void RpAnimBlendClumpInitSkinned(RpClump *);
void RpAnimBlendClumpFillFrameArray(RpClump *clump, AnimBlendFrameData **frames);
AnimBlendFrameData *RpAnimBlendClumpFindFrame(RpClump *clump, const char *name);
AnimBlendFrameData *RpAnimBlendClumpFindBone(RpClump *clump, uint tag);

struct RFrame {
	CQuaternion rot;
	float time;
};

struct RTFrame {
	CQuaternion rot;
	float time;
	CVector pos;
};

class CLink_CAnimBlendHierarchy {
public:
	CAnimBlendHierarchy *item;
	CLink_CAnimBlendHierarchy *prev;
	CLink_CAnimBlendHierarchy *next;

	void Insert(CLink_CAnimBlendHierarchy*);
	void Remove(void);
};

class CLinkList_CAnimBlendHierarchy {
public:
	CLink_CAnimBlendHierarchy head, tail;
	CLink_CAnimBlendHierarchy freeHead, freeTail;
	CLink_CAnimBlendHierarchy *links;

	void Init(int n);
	void Shutdown(void);
	CLink_CAnimBlendHierarchy *Insert(CAnimBlendHierarchy**);
	void Remove(CLink_CAnimBlendHierarchy*);
};

#define GETFRAME(seq, i) ((char*)(seq)->keyFrames + (((seq)->flag & 2) ? sizeof(RTFrame) : sizeof(RFrame))*(i))
#define GETCFRAME(seq, i) ((char*)(seq)->keyFramesCompressed + (((seq)->flag & 2) ? sizeof(RTFrame) : sizeof(RFrame))*(i))

// complete
class CAnimBlendSequence
{
public:
	void *vtable;
	int flag;
	char name[24];
	int numFrames;
	short boneTag;
	short pad;
	void *keyFrames;
	void *keyFramesCompressed;

	void RemoveQuaternionFlips(void);
	void SetNumFrames(int numFrames, char TS, char special);
	void SetBoneTag(int tag);
	void SetName(const char *name);
	CAnimBlendSequence(void);
	~CAnimBlendSequence(void);
	void ctor(void);
	void dtor(void);
	void dtor2(char flag);
};

// complete
class CAnimBlendHierarchy
{
public:
	char name[24];
	CAnimBlendSequence *blendSequences;
	short numSequences;
	char loadSpecial;		// special? who came up with this name? me? should be named compressed probably
	char compressed;		// really compressed?
	float totalLength;
	CLink_CAnimBlendHierarchy *linkPtr;

	void RemoveUncompressedData(void);
	void Uncompress(void);
	void RemoveQuaternionFlips(void);
	void CalcTotalTimeCompressed(void);
	void CalcTotalTime(void);
	void SetName(const char *name);
	void Shutdown(void);
	~CAnimBlendHierarchy(void);
	CAnimBlendHierarchy(void);
	void dtor(void);
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

// complete
class CAnimBlendNode
{
public:
	float theta0;
	float theta1;
	int frame0;
	int frame1;
	float time;
	CAnimBlendSequence *sequence;		// !!
	CAnimBlendAssociation *blendAssoc;	// reference to owner

	void CalcDeltasCompressed(void);
	void SetupKeyFrameCompressed(void);
	void FindKeyFrame(float time);
	void GetEndTranslation(CVector *vec, float f);
	void GetCurrentTranslation(CVector *vec, float f);
	void CalcDeltas(void);
	bool NextKeyFrame(void);
	bool Update(CVector &vec, CQuaternion &quat, float f);
	void Init(void);
};

// complete
class CAnimBlendAssociation
{
public:
	enum Flags {
		NoRotation = 2,
		NoTranslation = 4,
		Flag8 = 8,
		Partial = 0x10,
		Movement = 0x20
	};
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
	void (*callback)(CAnimBlendAssociation*, void*);
	void *callbackArg;

	void SetFinishCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg);
	void SetDeleteCallback(void (*callback)(CAnimBlendAssociation*, void*), void *arg);
	void SetBlend(float blendAmount, float blendDelta);
	void Start(float time);
	CAnimBlendNode *GetNode(int i);
	void SyncAnimation(CAnimBlendAssociation *anim);
	void SetCurrentTime(float time);
	void Init(CAnimBlendAssociation&);
	void Init(RpClump *clump, CAnimBlendHierarchy *anim);
	CAnimBlendAssociation(void);
	CAnimBlendAssociation(CAnimBlendAssociation&);
	~CAnimBlendAssociation(void);
	bool UpdateBlend(float f);
	void UpdateTime(float f1, float f2);
	void ctor(void);
	void dtor(void);
	void dtor2(char flag);
};

// complete
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
	int updateFlag;
	RwV3d pos;
	union {
		RwFrame *frame;
		RpHAnimKeyFrame *hanimframe;
	};
	int nodeID;
};

// complete
class CAnimBlendClumpData
{
public:
	void *nextAssoc;			// pointer to CAnimBlendAssociation::next
	void *prevAssoc;
	int numFrames;
	CVector *pedPosition;
	AnimBlendFrameData *frames;

	void ForAllFrames(void (*cb)(AnimBlendFrameData*, void*), void *arg);
	void SetNumberOfBones(int n);
	~CAnimBlendClumpData(void);
	CAnimBlendClumpData(void);
	void ctor(void);
	void dtor(void);
};

struct AnimAssocDefinition
{
	char *name;
	char *blockName;
	int modelIndex;
	int animCount;
	char **animNames;
	struct {
		int animId;
		int flags;
	} *animInfoList;
};

// complete
class CAnimManager
{
public:
	static int GetNumRefsToAnimBlock(int i);
	static void RemoveAnimBlockRefWithoutDelete(int i);
	static void RemoveAnimBlockRef(int i);
	static void AddAnimBlockRef(int i);
	static void RemoveAnimBlock(int i);
	static void RemoveLastAnimFile(void);
	static void LoadAnimFile(RwStream *stream, bool a2, const char (*a3)[32]);
	static int RegisterAnimBlock(const char *name);
	static void CreateAnimAssocGroups(void);
	static void LoadAnimFiles(void);
	static CAnimBlendAssociation *BlendAnimation(RpClump *clump, int groupId, int animId, float speed);
	static CAnimBlendAssociation *AddAnimation(RpClump *clump, int groupId, int animId);
	static CAnimBlendAssociation *GetAnimAssociation(int groupId, const char *name);
	static CAnimBlendAssociation *GetAnimAssociation(int groupId, int animId);
	static const char *GetAnimGroupName(int i);
	static CAnimBlendHierarchy *GetAnimation(const char *name, CAnimBlock *animBlock);
	static int GetAnimationBlockIndex(const char *name);
	static CAnimBlock *GetAnimationBlock(const char *name);
	static void RemoveFromUncompressedCache(CAnimBlendHierarchy*);
	static void UncompressAnimation(CAnimBlendHierarchy *hier);
	static void Shutdown(void);
	static void Initialise(void);

	static int &ms_numAnimations;
	static CAnimBlendHierarchy *ms_aAnimations;
	static int &ms_numAnimBlocks;
	static CAnimBlock *ms_aAnimBlocks;
	static CAnimBlendAssocGroup *&ms_aAnimAssocGroups;
	static AnimAssocDefinition *ms_aAnimAssocDefinitions;
	static CLinkList_CAnimBlendHierarchy *ms_animCache;
};
