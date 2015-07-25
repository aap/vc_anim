#include "vc_anim.h"

WRAPPER void CLink_CAnimBlendHierarchy::Insert(CLink_CAnimBlendHierarchy*) { EAXJMP(0x405D10); }
WRAPPER void CLink_CAnimBlendHierarchy::Remove(void) { EAXJMP(0x405D30); }

WRAPPER void CLinkList_CAnimBlendHierarchy::Init(int n) { EAXJMP(0x405C20); }
WRAPPER void CLinkList_CAnimBlendHierarchy::Shutdown(void) { EAXJMP(0x405C90); }
WRAPPER CLink_CAnimBlendHierarchy *CLinkList_CAnimBlendHierarchy::Insert(CAnimBlendHierarchy**) { EAXJMP(0x405CB0); }
WRAPPER void CLinkList_CAnimBlendHierarchy::Remove(CLink_CAnimBlendHierarchy*) { EAXJMP(0x405CF0); }

void
CAnimBlendHierarchy::RemoveUncompressedData(void)
{
	this->loadSpecial = 1;
}

void
CAnimBlendHierarchy::Uncompress(void)
{
	this->loadSpecial = 0;
	if(this->totalLength == 0.0f){
		for(int i = 0; i < this->numSequences; i++)
			this->blendSequences[i].RemoveQuaternionFlips();
		this->CalcTotalTime();
	}
}

void
CAnimBlendHierarchy::RemoveQuaternionFlips(void)
{
	for(int i = 0; i < this->numSequences; i++)
		this->blendSequences[i].RemoveQuaternionFlips();
}

WRAPPER void
CAnimBlendHierarchy::CalcTotalTimeCompressed(void)
{
	this->totalLength = 0.0f;
	for(int i = 0; i < this->numSequences; i++){
		CAnimBlendSequence *seq = &this->blendSequences[i];
		RFrame *last = (RFrame*)GETCFRAME(seq, seq->numFrames-1);
		if(last->time/60.f > this->totalLength)
			this->totalLength = last->time/60.f;
		for(int j = seq->numFrames-1; j > 0; j--){
			RFrame *f1 = (RFrame*)GETCFRAME(seq, j);
			RFrame *f2 = (RFrame*)GETCFRAME(seq, j-1);
			f1->time -= f2->time;
		}
	}
}

void
CAnimBlendHierarchy::CalcTotalTime(void)
{
	this->totalLength = 0.0f;
	for(int i = 0; i < this->numSequences; i++){
		CAnimBlendSequence *seq = &this->blendSequences[i];
		RFrame *last = (RFrame*)GETFRAME(seq, seq->numFrames-1);
		if(last->time > this->totalLength)
			this->totalLength = last->time;
		for(int j = seq->numFrames-1; j > 0; j--){
			RFrame *f1 = (RFrame*)GETFRAME(seq, j);
			RFrame *f2 = (RFrame*)GETFRAME(seq, j-1);
			f1->time -= f2->time;
		}
	}
}

void
CAnimBlendHierarchy::SetName(const char *name)
{
	strncpy(this->name, name, 24);
}

void
CAnimBlendHierarchy::Shutdown(void)
{
	CAnimManager::RemoveFromUncompressedCache(this);
	if(this->blendSequences)
		destroy_array(this->blendSequences, &CAnimBlendSequence::dtor);
	this->blendSequences = NULL;
	this->numSequences = 0;
	this->totalLength = 0.0f;
	this->loadSpecial = 0;
}

CAnimBlendHierarchy::~CAnimBlendHierarchy(void) { dtor(); }

void
CAnimBlendHierarchy::dtor(void)
{
	CAnimManager::RemoveFromUncompressedCache(this);
	if(this->blendSequences)
		destroy_array(this->blendSequences, &CAnimBlendSequence::dtor);
	this->blendSequences = 0;
	this->numSequences = 0;
	this->totalLength = 0.0f;
}

CAnimBlendHierarchy::CAnimBlendHierarchy(void)
{
	this->blendSequences = 0;
	this->numSequences = 0;
	this->loadSpecial = 0;
	this->compressed = 0;
	this->totalLength = 0.0f;
	this->linkPtr = NULL;
}
