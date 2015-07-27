#include "vc_anim.h"

void
FrameUpdateCallBackSkinnedWith3dVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes)
{
	CVector vec, pos;
	CQuaternion q, rot;
	float totalBlendAmount = 0.0f;
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float curx = 0.0f, cury = 0.0f, curz = 0.0f;
	float endx = 0.0f, endy = 0.0f, endz = 0.0f;
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
			curx += vec.x;
			cury += vec.y;
			curz += vec.z;
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
					x += vec.x;
					y += vec.y;
					z += vec.z;
					looped |= nodelooped;
					if(nodelooped){
						(*node)->GetEndTranslation(&vec, 1.0f-totalBlendAmount);
						endx += vec.x;
						endy += vec.y;
						endz += vec.z;
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
		pAnimClumpToUpdate->d[2] = z - curz;
		if(looped){
			pAnimClumpToUpdate->d[0] += endx;
			pAnimClumpToUpdate->d[1] += endy;
			pAnimClumpToUpdate->d[2] += endz;
		}
		frameData[6] = pos.x - x + frame->pos.x;
		frameData[7] = pos.y - y + frame->pos.y;
		frameData[8] = pos.z - z + frame->pos.z;
	}	
}

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
FrameUpdateCallBackWith3dVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes)
{
	CVector vec, pos;
	CQuaternion q, rot;
	float totalBlendAmount = 0.0f;
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float curx = 0.0f, cury = 0.0f, curz = 0.0f;
	float endx = 0.0f, endy = 0.0f, endz = 0.0f;
	rot.x = rot.y = rot.z = rot.w = 0.0f;
	pos.x = pos.y = pos.z = 0.0f;
	bool looped = false;
	RwMatrix *mat = &frame->frame->modelling;
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
			curx += vec.x;
			cury += vec.y;
			curz += vec.z;
		}
		node++;
	}while(*node);

	node = nodes+1;
	do{
		if((*node)->sequence){
			bool nodelooped = (*node)->Update(vec, q, 1.0f-totalBlendAmount);
			rot.x += q.x;
			rot.y += q.y;
			rot.z += q.z;
			rot.w += q.w;
			if((*node)->sequence->flag & 2){
				pos.x += vec.x;
				pos.y += vec.y;
				pos.z += vec.z;
				if((*node)->blendAssoc->flags & 0x40){
					x += vec.x;
					y += vec.y;
					z += vec.z;
					looped |= nodelooped;
					if(nodelooped){
						(*node)->GetEndTranslation(&vec, 1.0f-totalBlendAmount);
						endx += vec.x;
						endy += vec.y;
						endz += vec.z;
					}
				}
			}
		}
		++*node;
		node++;
	}while(*node);

	if(!(frame->flag & 2)){
		mat->at.z = mat->up.y = mat->right.x = 1.0f;
		mat->up.x = mat->right.z = mat->right.y = 0.0f;
		mat->at.y = mat->at.x = mat->up.z = 0.0f;
		mat->pos.z = mat->pos.y = mat->pos.x = 0.0f;
		mat->flags |= 0x20003;

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
		rot.Get(mat);
	}

	if(!(frame->flag & 4)){
		pAnimClumpToUpdate->d[0] = x - curx;
		pAnimClumpToUpdate->d[1] = y - cury;
		pAnimClumpToUpdate->d[2] = z - curz;
		if(looped){
			pAnimClumpToUpdate->d[0] += endx;
			pAnimClumpToUpdate->d[1] += endy;
			pAnimClumpToUpdate->d[2] += endz;
		}
		mat->pos.x = pos.x - x + frame->pos.x;
		mat->pos.y = pos.y - y + frame->pos.y;
		mat->pos.z = pos.z - z + frame->pos.z;
	}
	RwMatrixUpdate(mat);
}

void
FrameUpdateCallBackWithVelocityExtraction(AnimBlendFrameData *frame, CAnimBlendNode **nodes)
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
	RwMatrix *mat = &frame->frame->modelling;
	CAnimBlendNode **node;

	if(*nodes)
		for(node = nodes+1; *node; node++)
			if((*node)->sequence && ((*node)->blendAssoc->flags & CAnimBlendAssociation::Partial))
				totalBlendAmount += (*node)->blendAssoc->blendAmount;

	for(node = nodes+1; *node; node++)
		if((*node)->sequence && (*node)->sequence->flag & 2 && (*node)->blendAssoc->flags & 0x40){
			(*node)->GetCurrentTranslation(&vec, 1.0f-totalBlendAmount);
			cury += vec.y;
			if((*node)->blendAssoc->flags & 0x80)
				curx += vec.x;
		}

	for(node = nodes+1; *node; node++){
		if((*node)->sequence){
			bool nodelooped = (*node)->Update(vec, q, 1.0f-totalBlendAmount);
			rot.x += q.x;
			rot.y += q.y;
			rot.z += q.z;
			rot.w += q.w;
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
	}

	if(!(frame->flag & 2)){
		mat->at.z = mat->up.y = mat->right.x = 1.0f;
		mat->up.x = mat->right.z = mat->right.y = 0.0f;
		mat->at.y = mat->at.x = mat->up.z = 0.0f;
		mat->pos.z = mat->pos.y = mat->pos.x = 0.0f;
		mat->flags |= 0x20003;

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
		rot.Get(mat);
	}

	if(!(frame->flag & 4)){
		pAnimClumpToUpdate->d[0] = x - curx;
		pAnimClumpToUpdate->d[1] = y - cury;
		if(looped){
			pAnimClumpToUpdate->d[0] += endx;
			pAnimClumpToUpdate->d[1] += endy;
		}
		mat->pos.x = pos.x - x;
		mat->pos.y = pos.y - y;
		mat->pos.z = pos.z;
	}
	RwMatrixUpdate(mat);
}

void
FrameUpdateCallBackNonSkinned(AnimBlendFrameData *frame, void *arg)
{
	CAnimBlendNode **nodes = (CAnimBlendNode**)arg;
	CVector vec, pos;
	CQuaternion q, rot;
	float totalBlendAmount = 0.0f;
	rot.x = rot.y = rot.z = rot.w = 0.0f;
	pos.x = pos.y = pos.z = 0.0f;
	CAnimBlendNode **node;
	RwMatrix *mat = &frame->frame->modelling;

	if (frame->flag & 8 && pAnimClumpToUpdate->d){
		if(frame->flag & 0x10)
			FrameUpdateCallBackWith3dVelocityExtraction(frame, nodes);
		else
			FrameUpdateCallBackWithVelocityExtraction(frame, nodes);
		return;
	}

	if(*nodes)
		for(node = nodes+1; *node; node++)
			if((*node)->sequence && ((*node)->blendAssoc->flags & CAnimBlendAssociation::Partial))
				totalBlendAmount += (*node)->blendAssoc->blendAmount;

	for(node = nodes+1; *node; node++){
		if((*node)->sequence){
			(*node)->Update(vec, q, 1.0f-totalBlendAmount);
			rot.x += q.x;
			rot.y += q.y;
			rot.z += q.z;
			rot.w += q.w;
			if((*node)->sequence->flag & 2){
				pos.x += vec.x;
				pos.y += vec.y;
				pos.z += vec.z;
			}
		}
		++*node;
	}

	if(!(frame->flag & 2)){
		mat->at.z = mat->up.y = mat->right.x = 1.0f;
		mat->up.x = mat->right.z = mat->right.y = 0.0f;
		mat->at.y = mat->at.x = mat->up.z = 0.0f;
		mat->pos.z = mat->pos.y = mat->pos.x = 0.0f;
		mat->flags |= 0x20003;

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
		rot.Get(mat);
	}

	if(!(frame->flag & 4)){
		mat->pos.x = pos.x + frame->pos.x;
		mat->pos.y = pos.y + frame->pos.y;
		mat->pos.z = pos.z + frame->pos.z;
	}
	RwMatrixUpdate(mat);
}

void
FrameUpdateCallbackNoRender(AnimBlendFrameData *frame, void *arg)
{
	if(frame->flag & 8 && pAnimClumpToUpdate->d)
		FrameUpdateCallBackSkinnedWithVelocityExtraction(frame, (CAnimBlendNode**)arg);
}