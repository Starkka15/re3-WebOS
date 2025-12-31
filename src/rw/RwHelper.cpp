#define WITHD3D
#include "common.h"
#include <rpskin.h>

#include "RwHelper.h"
#include "Timecycle.h"
#include "skeleton.h"
#include "Debug.h"
#include "MBlur.h"
#if !defined(FINAL) || defined(DEBUGMENU)
#include "rtcharse.h"
#endif
#ifndef FINAL
RtCharset *debugCharset;
bool bDebugRenderGroups;
#endif

#ifdef PS2_ALPHA_TEST
bool gPS2alphaTest = true;
#else
bool gPS2alphaTest = false;
#endif
bool gBackfaceCulling = true;

#if !defined(FINAL) || defined(DEBUGMENU)
static bool charsetOpen;
void OpenCharsetSafe()
{
	if(!charsetOpen)
		RtCharsetOpen();
	charsetOpen = true;
}
#endif

void CreateDebugFont()
{
#ifndef FINAL
	RwRGBA color = { 255, 255, 128, 255 };
	RwRGBA colorbg = { 0, 0, 0, 0 };
	OpenCharsetSafe();
	debugCharset = RtCharsetCreate(&color, &colorbg);
#endif
}

void DestroyDebugFont()
{
#ifndef FINAL
	RtCharsetDestroy(debugCharset);
	RtCharsetClose();
	charsetOpen = false;
#endif
}

void ObrsPrintfString(const char *str, short x, short y)
{
#ifndef FINAL
	RtCharsetPrintBuffered(debugCharset, str, x*8, y*16, true);
#endif
}

void FlushObrsPrintfs()
{
#ifndef FINAL
	RtCharsetBufferFlush();
#endif
}

void
DefinedState(void)
{
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	// Clear any accumulated errors before we start
	while(glGetError() != GL_NO_ERROR);
	static int defStateCallCount = 0;
	defStateCallCount++;
	FILE* defLog = NULL /* logging disabled */;
	if(defLog) { fprintf(defLog, "DefinedState() call #%d START\n", defStateCallCount); fclose(defLog); }
#endif

	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after TEXTUREADDRESS: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)TRUE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after TEXTUREPERSPECTIVE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after ZWRITEENABLE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after SHADEMODE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after TEXTUREFILTER: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after VERTEXALPHAENABLE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after SRCBLEND: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after DESTBLEND: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEALPHAPRIMITIVEBUFFER, (void*)FALSE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after ALPHAPRIMITIVEBUFFER: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void*)RWRGBALONG(0, 0, 0, 255));
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after BORDERCOLOR: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after FOGENABLE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEFOGCOLOR,
		(void*)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after FOGCOLOR: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)rwFOGTYPELINEAR);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after FOGTYPE: 0x%x\n", e); fclose(f); } } }
#endif
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after CULLMODE: 0x%x\n", e); fclose(f); } } }
#endif

#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAGREATEREQUAL);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after ALPHATESTFUNC: 0x%x\n", e); fclose(f); } } }
#endif

	rw::SetRenderState(rw::GSALPHATEST, gPS2alphaTest);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after GSALPHATEST: 0x%x\n", e); fclose(f); } } }
#endif
#else
	// D3D stuff
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
#endif
	SetAlphaRef(2);
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	{ GLenum e = glGetError(); if(e) { FILE* f = NULL /* logging disabled */; if(f) { fprintf(f, "  GL ERROR after SetAlphaRef: 0x%x\n", e); fclose(f); } } }
	defLog = NULL /* logging disabled */;
	if(defLog) { fprintf(defLog, "DefinedState() call #%d END\n", defStateCallCount); fclose(defLog); }
#endif
}

void
SetAlphaRef(int ref)
{
#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTREF, ref+1);
#else
	RwD3D8SetRenderState(D3DRS_ALPHAREF, ref);
#endif
}

void
SetCullMode(uint32 mode)
{
	if(gBackfaceCulling)
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)mode);
	else
		RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
}

#ifndef FINAL
void
PushRendergroup(const char *name)
{
	if(!bDebugRenderGroups)
		return;
#if defined(RW_OPENGL)
	if(GLAD_GL_KHR_debug)
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
#elif defined(RW_D3D9)
	static WCHAR tmp[256];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name, -1, tmp, sizeof(tmp));
	D3DPERF_BeginEvent(0xFFFFFFFF, tmp);
#endif
}

void
PopRendergroup(void)
{
	if(!bDebugRenderGroups)
		return;
#if defined(RW_OPENGL)
	if(GLAD_GL_KHR_debug)
		glPopDebugGroup();
#elif defined(RW_D3D9)
	D3DPERF_EndEvent();
#endif
}
#endif

RwFrame*
GetFirstFrameCallback(RwFrame *child, void *data)
{
	*(RwFrame**)data = child;
	return nil;
}

RwFrame*
GetFirstChild(RwFrame *frame)
{
	RwFrame *child;

	child = nil;
	RwFrameForAllChildren(frame, GetFirstFrameCallback, &child);
	return child;
}

RwObject*
GetFirstObjectCallback(RwObject *object, void *data)
{
	*(RwObject**)data = object;
	return nil;
}

RwObject*
GetFirstObject(RwFrame *frame)
{
	RwObject *obj;

	obj = nil;
	RwFrameForAllObjects(frame, GetFirstObjectCallback, &obj);
	return obj;
}

RpAtomic*
GetFirstAtomicCallback(RpAtomic *atm, void *data)
{
	*(RpAtomic**)data = atm;
	return nil;
}

RpAtomic*
GetFirstAtomic(RpClump *clump)
{
	RpAtomic *atm;

	atm = nil;
	RpClumpForAllAtomics(clump, GetFirstAtomicCallback, &atm);
	return atm;
}

RwTexture*
GetFirstTextureCallback(RwTexture *tex, void *data)
{
	*(RwTexture**)data = tex;
	return nil;
}

RwTexture*
GetFirstTexture(RwTexDictionary *txd)
{
	RwTexture *tex;

	tex = nil;
	RwTexDictionaryForAllTextures(txd, GetFirstTextureCallback, &tex);
	return tex;
}

#ifdef PED_SKIN
static RpAtomic*
isSkinnedCb(RpAtomic *atomic, void *data)
{
	RpAtomic **pAtomic = (RpAtomic**)data;
	if(*pAtomic)
		return nil;	// already found one
	if(RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)))
		*pAtomic = atomic;	// we could just return nil here directly...
	return atomic;
}

RpAtomic*
IsClumpSkinned(RpClump *clump)
{
	RpAtomic *atomic = nil;
	RpClumpForAllAtomics(clump, isSkinnedCb, &atomic);
	return atomic;
}

static RpAtomic*
GetAnimHierarchyCallback(RpAtomic *atomic, void *data)
{
	*(RpHAnimHierarchy**)data = RpSkinAtomicGetHAnimHierarchy(atomic);
	return nil;
}

RpHAnimHierarchy*
GetAnimHierarchyFromSkinClump(RpClump *clump)
{
	RpHAnimHierarchy *hier = nil;
	RpClumpForAllAtomics(clump, GetAnimHierarchyCallback, &hier);
	return hier;
}

static RwFrame*
GetAnimHierarchyFromClumpCB(RwFrame *frame, void *data)
{
	RpHAnimHierarchy *hier = RpHAnimFrameGetHierarchy(frame);
	if(hier){
		*(RpHAnimHierarchy**)data = hier;
		return nil;
	}
	RwFrameForAllChildren(frame, GetAnimHierarchyFromClumpCB, data);
	return frame;
}

RpHAnimHierarchy*
GetAnimHierarchyFromClump(RpClump *clump)
{
	RpHAnimHierarchy *hier = nil;
	RwFrameForAllChildren(RpClumpGetFrame(clump), GetAnimHierarchyFromClumpCB, &hier);
	return hier;
}

RwFrame*
GetHierarchyFromChildNodesCB(RwFrame *frame, void *data)
{
	RpHAnimHierarchy **pHier = (RpHAnimHierarchy**)data;
	RpHAnimHierarchy *hier = RpHAnimFrameGetHierarchy(frame);
	if(hier == nil)
		RwFrameForAllChildren(frame, GetHierarchyFromChildNodesCB, &hier);
	*pHier = hier;
	return nil;
}

void
SkinGetBonePositionsToTable(RpClump *clump, RwV3d *boneTable)
{
	int i, parent;
	RpAtomic *atomic;
	RpSkin *skin;
	RpHAnimHierarchy *hier;
	int numBones;
	RwMatrix m, invmat;
	int stack[32];
	int sp;

	if(boneTable == nil)
		return;

//	atomic = GetFirstAtomic(clump);		// mobile, also VC
	atomic = IsClumpSkinned(clump);		// xbox, seems safer
	assert(atomic);
	skin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic));
	assert(skin);
	hier = GetAnimHierarchyFromSkinClump(clump);
	assert(hier);
	boneTable[0].x = 0.0f;
	boneTable[0].y = 0.0f;
	boneTable[0].z = 0.0f;
	numBones = RpSkinGetNumBones(skin);
	parent = 0;
	sp = 0;
#ifdef FIX_BUGS
	stack[0] = 0;	// i think this is ok
#endif
	for(i = 1; i < numBones; i++){
		RwMatrixCopy(&m, &RpSkinGetSkinToBoneMatrices(skin)[i]);
		RwMatrixInvert(&invmat, &m);
		const RwMatrix *x = RpSkinGetSkinToBoneMatrices(skin);
		RwV3dTransformPoints(&boneTable[i], &invmat.pos, 1, &x[parent]);
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[++sp] = parent;
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPOPPARENTMATRIX)
			parent = stack[sp--];
		else
			parent = i;

		//assert(parent >= 0 && parent < numBones);
	}
}

RpHAnimAnimation*
HAnimAnimationCreateForHierarchy(RpHAnimHierarchy *hier)
{
	int i;
#if defined FIX_BUGS || defined LIBRW
	int numNodes = hier->numNodes*2;	// you're supposed to have at least two KFs per node
#else
	int numNodes = hier->numNodes;
#endif
	RpHAnimAnimation *anim = RpHAnimAnimationCreate(rpHANIMSTDKEYFRAMETYPEID, numNodes, 0, 0.0f);
	if(anim == nil)
		return nil;
	RpHAnimStdKeyFrame *frame;
	for(i = 0; i < numNodes; i++){
		frame = (RpHAnimStdKeyFrame*)HANIMFRAME(anim, i);	// games uses struct size here, not safe
		frame->q.real = 1.0f;
		frame->q.imag.x = frame->q.imag.y = frame->q.imag.z = 0.0f;
		frame->t.x = frame->t.y = frame->t.z = 0.0f;
#if defined FIX_BUGS || defined LIBRW
		// times are subtracted and divided giving NaNs
		// so they can't both be 0
		frame->time = i/hier->numNodes;
#else
		frame->time = 0.0f;
#endif
		frame->prevFrame = nil;
	}
	return anim;
}

void
RenderSkeleton(RpHAnimHierarchy *hier)
{
	int i;
	int sp;
	int stack[32];
	int par;
	CVector p1, p2;
	int numNodes = hier->numNodes;
	RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
	p1 = mats[0].pos;

	par = 0;
	sp = 0;
	stack[sp++] = par;
	for(i = 1; i < numNodes; i++){
		p1 = mats[par].pos;
		p2 = mats[i].pos;
		CDebug::AddLine(p1, p2, 0xFFFFFFFF, 0xFFFFFFFF);
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPUSHPARENTMATRIX)
			stack[sp++] = par;
		par = i;
		if(HIERNODEINFO(hier)[i].flags & rpHANIMPOPPARENTMATRIX)
			par = stack[--sp];
	}
}
#endif

void
CameraSize(RwCamera * camera, RwRect * rect,
		   RwReal viewWindow, RwReal aspectRatio)
{
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
	FILE *log = fopen("/media/internal/.gta3/debug.log", "a");
	if (log) { fprintf(log, "CameraSize: Called with camera=%p, rect=%p\n", camera, rect); fflush(log); fclose(log); }
#endif

	if (camera)
	{
		RwVideoMode         videoMode;
		RwRect              r;
		RwRect              origSize = { 0, 0, 0, 0 };	// FIX just to make the compier happy
		RwV2d               vw;

#ifdef WEBOS_TOUCHPAD
		// FIX: RwEngineGetVideoModeInfo returns garbage on webOS, use RsGlobal instead
		videoMode.width = RsGlobal.width;
		videoMode.height = RsGlobal.height;
		videoMode.flags = 0;
#else
		RwEngineGetVideoModeInfo(&videoMode,
								 RwEngineGetCurrentVideoMode());
#endif

#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		log = NULL /* logging disabled */;
		if (log) {
			fprintf(log, "CameraSize: VideoMode: %dx%d, flags=0x%x\n", videoMode.width, videoMode.height, videoMode.flags);
			fprintf(log, "CameraSize: About to call RwCameraGetRaster...\n");
			fflush(log);
			fclose(log);
		}
#endif

#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		RwRaster *camRaster = RwCameraGetRaster(camera);
		log = NULL /* logging disabled */;
		if (log) {
			fprintf(log, "CameraSize: RwCameraGetRaster returned: %p\n", camRaster);
			fflush(log);
			fclose(log);
		}
		if (!camRaster) {
			log = NULL /* logging disabled */;
			if (log) {
				fprintf(log, "CameraSize: ERROR - Camera raster is NULL!\n");
				fflush(log);
				fclose(log);
			}
			return;
		}
		origSize.w  = RwRasterGetWidth(camRaster);
#else
		origSize.w  = RwRasterGetWidth(RwCameraGetRaster(camera));
#endif

#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		log = NULL /* logging disabled */;
		if (log) {
			fprintf(log, "CameraSize: Got raster width: %d\n", origSize.w);
			fprintf(log, "CameraSize: About to get raster height...\n");
			fflush(log);
			fclose(log);
		}
		origSize.h = RwRasterGetHeight(camRaster);
#else
		origSize.h = RwRasterGetHeight(RwCameraGetRaster(camera));
#endif

#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		log = NULL /* logging disabled */;
		if (log) { fprintf(log, "CameraSize: Got raster height: %d\n", origSize.h); fflush(log); fclose(log); }
#endif

		if (!rect)
		{
#ifdef WEBOS_TOUCHPAD
			// webOS: Video mode not properly tracked in NULL platform
			// Use RsGlobal dimensions directly
			r.x = r.y = 0;
			r.w = RsGlobal.width;
			r.h = RsGlobal.height;
			rect = &r;
#else
			if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
			{
				/* For full screen applications, resizing the camera just doesn't
				 * make sense, use the video mode size.
				 */

				r.x = r.y = 0;
				r.w = videoMode.width;
				r.h = videoMode.height;
				rect = &r;
			}
			else
			{
				/*
				rect not specified - reuse current values
				*/
				r.w = RwRasterGetWidth(RwCameraGetRaster(camera));
				r.h = RwRasterGetHeight(RwCameraGetRaster(camera));
				r.x = r.y = 0;
				rect = &r;
			}
#endif
		}

#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		log = NULL /* logging disabled */;
		if (log) {
			fprintf(log, "CameraSize: About to check if resize needed, rect=%p\n", rect);
			fflush(log);
			fclose(log);
		}
		if (!rect) {
			log = NULL /* logging disabled */;
			if (log) {
				fprintf(log, "CameraSize: ERROR - rect is NULL after rect check!\n");
				fflush(log);
				fclose(log);
			}
			return;
		}
		log = NULL /* logging disabled */;
		if (log) {
			fprintf(log, "CameraSize: rect->w=%d, rect->h=%d, origSize.w=%d, origSize.h=%d\n",
				rect->w, rect->h, origSize.w, origSize.h);
			fflush(log);
			fclose(log);
		}
#endif

		if (( origSize.w != rect->w ) || ( origSize.h != rect->h ))
		{
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
			log = NULL /* logging disabled */;
			if (log) {
				fprintf(log, "CameraSize: Resize needed, destroying old rasters...\n");
				fflush(log);
				fclose(log);
			}
#endif
			RwRaster           *raster;
			RwRaster           *zRaster;

			// BUG: game just changes camera raster's sizes, but this is a hack
#if defined FIX_BUGS || defined LIBRW
			/*
			 * Destroy rasters...
			 */

			raster = RwCameraGetRaster(camera);
			if( raster )
			{
				RwRasterDestroy(raster);
				camera->frameBuffer = nil;
			}

			zRaster = RwCameraGetZRaster(camera);
			if( zRaster )
			{
				RwRasterDestroy(zRaster);
				camera->zBuffer = nil;
			}

			/*
			 * Create new rasters...
			 */

			raster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERA);
			zRaster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

			if( raster && zRaster )
			{
				RwCameraSetRaster(camera, raster);
				RwCameraSetZRaster(camera, zRaster);
			}
			else
			{
				if( raster )
				{
					RwRasterDestroy(raster);
				}

				if( zRaster )
				{
					RwRasterDestroy(zRaster);
				}

				rect->x = origSize.x;
				rect->y = origSize.y;
				rect->w = origSize.w;
				rect->h = origSize.h;

				/*
				 * Use default values...
				 */
				raster =
					RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERA);

				zRaster =
					RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

				RwCameraSetRaster(camera, raster);
				RwCameraSetZRaster(camera, zRaster);
			}
#else
			raster = RwCameraGetRaster(camera);
			zRaster = RwCameraGetZRaster(camera);

			raster->width = zRaster->width = rect->w;
			raster->height = zRaster->height = rect->h;
#endif
#ifdef FIX_BUGS
			if(CMBlur::BlurOn){
				CMBlur::MotionBlurClose();
				CMBlur::MotionBlurOpen(camera);
			}
#endif
		}

		/* Figure out the view window */
		if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
		{
			/* derive ratio from aspect ratio */
			vw.x = viewWindow;
			vw.y = viewWindow / aspectRatio;
		}
		else
		{
			/* derive from pixel ratios */
			if (rect->w > rect->h)
			{
				vw.x = viewWindow;
				vw.y = (rect->h * viewWindow) / rect->w;
			}
			else
			{
				vw.x = (rect->w * viewWindow) / rect->h;
				vw.y = viewWindow;
			}
		}

		RwCameraSetViewWindow(camera, &vw);

		RsGlobal.width  = rect->w;
		RsGlobal.height = rect->h;
	}

	return;
}

void
CameraDestroy(RwCamera *camera)
{
	RwRaster    *raster, *tmpRaster;
	RwFrame     *frame;

	if (camera)
	{
		frame = RwCameraGetFrame(camera);
		if (frame)
		{
			RwFrameDestroy(frame);
		}

		raster = RwCameraGetRaster(camera);
		if (raster)
		{
			tmpRaster = RwRasterGetParent(raster);

			RwRasterDestroy(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwRasterDestroy(tmpRaster);
			}
		}

		raster = RwCameraGetZRaster(camera);
		if (raster)
		{
			tmpRaster = RwRasterGetParent(raster);

			RwRasterDestroy(raster);

			if ((tmpRaster != nil) && (tmpRaster != raster))
			{
				RwRasterDestroy(tmpRaster);
			}
		}

		RwCameraDestroy(camera);
	}

	return;
}

RwCamera           *
CameraCreate(RwInt32 width, RwInt32 height, RwBool zBuffer)
{
	RwCamera           *camera;
	FILE *log = fopen("/media/internal/.gta3/debug.log", "a");

	if (log) { fprintf(log, "CameraCreate: Creating camera...\n"); fflush(log); }
	camera = RwCameraCreate();
	if (log) { fprintf(log, "CameraCreate: RwCameraCreate() returned: %p\n", camera); fflush(log); }

	if (camera)
	{
		if (log) { fprintf(log, "CameraCreate: Creating frame...\n"); fflush(log); }
		RwFrame *frame = RwFrameCreate();
		if (log) { fprintf(log, "CameraCreate: RwFrameCreate() returned: %p\n", frame); fflush(log); }
		RwCameraSetFrame(camera, frame);

		if (log) { fprintf(log, "CameraCreate: Creating camera raster...\n"); fflush(log); }
		RwRaster *raster = RwRasterCreate(width, height, 0, rwRASTERTYPECAMERA);
		if (log) { fprintf(log, "CameraCreate: RwRasterCreate(CAMERA) returned: %p\n", raster); fflush(log); }
		RwCameraSetRaster(camera, raster);

		if (zBuffer)
		{
			if (log) { fprintf(log, "CameraCreate: Creating zbuffer raster...\n"); fflush(log); }
			RwRaster *zraster = RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER);
			if (log) { fprintf(log, "CameraCreate: RwRasterCreate(ZBUFFER) returned: %p\n", zraster); fflush(log); }
			RwCameraSetZRaster(camera, zraster);
		}

		/* now check that everything is valid */
#ifdef WEBOS_VERBOSE_DEBUG_DISABLED
		// WebOS uses NULL platform - rasters don't have parents, skip parent check
		if (log) { fprintf(log, "CameraCreate: Validating (webOS)...\n"); fflush(log); }
		if (log) { fprintf(log, "  Frame: %p\n", RwCameraGetFrame(camera)); fflush(log); }
		if (log) { fprintf(log, "  Raster: %p\n", RwCameraGetRaster(camera)); fflush(log); }
		if (zBuffer && log) { fprintf(log, "  ZRaster: %p\n", RwCameraGetZRaster(camera)); fflush(log); }

		if (RwCameraGetFrame(camera) &&
			RwCameraGetRaster(camera) &&
			(!zBuffer || RwCameraGetZRaster(camera)))
		{
			/* everything OK */
			if (log) { fprintf(log, "CameraCreate: Validation passed!\n"); fclose(log); }
			return (camera);
		}
		if (log) { fprintf(log, "CameraCreate: Validation FAILED!\n"); fflush(log); }
#else
		if (RwCameraGetFrame(camera) &&
			RwCameraGetRaster(camera) &&
			RwRasterGetParent(RwCameraGetRaster(camera)) &&
			(!zBuffer || (RwCameraGetZRaster(camera) &&
						  RwRasterGetParent(RwCameraGetZRaster
											(camera)))))
		{
			/* everything OK */
			return (camera);
		}
#endif
	}
	else
	{
		if (log) { fprintf(log, "CameraCreate: RwCameraCreate() returned NULL!\n"); fflush(log); }
	}

	/* if we're here then an error must have occurred so clean up */
	if (log) { fprintf(log, "CameraCreate: Destroying camera and returning NULL\n"); fclose(log); }

	CameraDestroy(camera);
	return (nil);
}

#ifdef LIBRW
#include <rpmatfx.h>
#include "VehicleModelInfo.h"

int32
findPlatform(rw::Atomic *a)
{
	rw::Geometry *g = a->geometry;
	if(g->instData)
		return g->instData->platform;
	return 0;
}

// in CVehicleModelInfo in VC
static RpMaterial*
GetMatFXEffectMaterialCB(RpMaterial *material, void *data)
{
	if(RpMatFXMaterialGetEffects(material) == rpMATFXEFFECTNULL)
		return material;
	*(int*)data = RpMatFXMaterialGetEffects(material);
	return nil;
}

// Game doesn't read atomic extensions so we never get any other than the default pipe,
// but we need it for uninstancing
void
attachPipe(rw::Atomic *atomic)
{
	if(RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)))
		atomic->pipeline = rw::skinGlobals.pipelines[rw::platform];
	else{
		int fx = rpMATFXEFFECTNULL;
		RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic), GetMatFXEffectMaterialCB, &fx);
		if(fx != rpMATFXEFFECTNULL)
			RpMatFXAtomicEnableEffects(atomic);
	}
}

// Attach pipes for the platform we have native data for so we can uninstance
void
switchPipes(rw::Atomic *a, int32 platform)
{
	if(a->pipeline && a->pipeline->platform != platform){
		uint32 plgid = a->pipeline->pluginID;
		switch(plgid){
		// assume default pipe won't be attached explicitly
		case rw::ID_SKIN:
			a->pipeline = rw::skinGlobals.pipelines[platform];
			break;
		case rw::ID_MATFX:
			a->pipeline = rw::matFXGlobals.pipelines[platform];
			break;
		}
	}
}

RpAtomic*
ConvertPlatformAtomic(RpAtomic *atomic, void *data)
{
	int32 driver = rw::platform;
	int32 platform = findPlatform(atomic);
	if(platform != 0 && platform != driver){
		attachPipe(atomic);	// kludge
		rw::ObjPipeline *origPipe = atomic->pipeline;
		rw::platform = platform;
		switchPipes(atomic, rw::platform);
		if(atomic->geometry->flags & rw::Geometry::NATIVE)
			atomic->uninstance();
		// no ADC in this game
		//rw::ps2::unconvertADC(atomic->geometry);
		rw::platform = driver;
		atomic->pipeline = origPipe;
	}
	return atomic;
}
#endif

#if defined(FIX_BUGS) && defined(GTA_PC)
RwUInt32 saved_alphafunc, saved_alpharef;

void
SetAlphaTest(RwUInt32 alpharef)
{
#ifdef LIBRW
	saved_alphafunc = rw::GetRenderState(rw::ALPHATESTFUNC);
	saved_alpharef = rw::GetRenderState(rw::ALPHATESTREF);

	rw::SetRenderState(rw::ALPHATESTFUNC, rw::ALPHAGREATEREQUAL);
	rw::SetRenderState(rw::ALPHATESTREF, 0);
#else
	RwD3D8GetRenderState(D3DRS_ALPHAFUNC, &saved_alphafunc);
	RwD3D8GetRenderState(D3DRS_ALPHAREF, &saved_alpharef);
	
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	RwD3D8SetRenderState(D3DRS_ALPHAREF, alpharef);
#endif
}

void
RestoreAlphaTest()
{
#ifdef LIBRW
	rw::SetRenderState(rw::ALPHATESTFUNC, saved_alphafunc);
	rw::SetRenderState(rw::ALPHATESTREF, saved_alpharef);
#else
	RwD3D8SetRenderState(D3DRS_ALPHAFUNC, saved_alphafunc);
	RwD3D8SetRenderState(D3DRS_ALPHAREF, saved_alpharef);
#endif
}
#endif
