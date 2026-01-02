#include "common.h"
#include <ctype.h>

#include "templates.h"
#include "General.h"
#include "Streaming.h"
#include "RwHelper.h"
#include "TxdStore.h"
#include "FileMgr.h"

CPool<TxdDef,TxdDef> *CTxdStore::ms_pTxdPool;
RwTexDictionary *CTxdStore::ms_pStoredTxd;

void
CTxdStore::Initialise(void)
{
	if(ms_pTxdPool == nil)
		ms_pTxdPool = new CPool<TxdDef,TxdDef>(TXDSTORESIZE);
}

void
CTxdStore::Shutdown(void)
{
	if(ms_pTxdPool)
		delete ms_pTxdPool;
}

void
CTxdStore::GameShutdown(void)
{
	int i;

	for(i = 0; i < TXDSTORESIZE; i++){
		TxdDef *def = GetSlot(i);
		if(def && GetNumRefs(i) == 0)
			RemoveTxdSlot(i);
	}
}

int
CTxdStore::AddTxdSlot(const char *name)
{
	TxdDef *def = ms_pTxdPool->New();
	assert(def);
	def->texDict = nil;
	def->refCount = 0;
	strcpy(def->name, name);
	return ms_pTxdPool->GetJustIndex(def);
}

void
CTxdStore::RemoveTxdSlot(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	ms_pTxdPool->Delete(def);
}

int
CTxdStore::FindTxdSlot(const char *name)
{
	int size = ms_pTxdPool->GetSize();
	for(int i = 0; i < size; i++){
		TxdDef *def = GetSlot(i);
		if(def && !CGeneral::faststricmp(def->name, name))
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	return GetSlot(slot)->name;
}

void
CTxdStore::PushCurrentTxd(void)
{
	ms_pStoredTxd = RwTexDictionaryGetCurrent();
}

void
CTxdStore::PopCurrentTxd(void)
{
	RwTexDictionarySetCurrent(ms_pStoredTxd);
	ms_pStoredTxd = nil;
}

void
CTxdStore::SetCurrentTxd(int slot)
{
	RwTexDictionarySetCurrent(GetSlot(slot)->texDict);
}

void
CTxdStore::Create(int slot)
{
	GetSlot(slot)->texDict = RwTexDictionaryCreate();
}

int
CTxdStore::GetNumRefs(int slot)
{
	return GetSlot(slot)->refCount;
}

void
CTxdStore::AddRef(int slot)
{
	GetSlot(slot)->refCount++;
}

void
CTxdStore::RemoveRef(int slot)
{
	if(--GetSlot(slot)->refCount <= 0)
		CStreaming::RemoveTxd(slot);
}

void
CTxdStore::RemoveRefWithoutDelete(int slot)
{
	GetSlot(slot)->refCount--;
}

bool
CTxdStore::LoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);

#ifdef WEBOS_TOUCHPAD
	FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
	if (log) { fprintf(log, "LoadTxd(stream): ENTRY slot=%d, stream=%p\n", slot, stream); fflush(log); fclose(log); }
#endif

	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
#ifdef WEBOS_TOUCHPAD
		log = NULL /* logging disabled */;
		if (log) { fprintf(log, "LoadTxd(stream): Found TEXDICTIONARY chunk, calling RwTexDictionaryGtaStreamRead\n"); fflush(log); fclose(log); }
#endif
		def->texDict = RwTexDictionaryGtaStreamRead(stream);
#ifdef WEBOS_TOUCHPAD
		log = NULL /* logging disabled */;
		if (log) { fprintf(log, "LoadTxd(stream): RwTexDictionaryGtaStreamRead returned %p\n", def->texDict); fflush(log); fclose(log); }
#endif
		return def->texDict != nil;
	}
#ifdef WEBOS_TOUCHPAD
	log = NULL /* logging disabled */;
	if (log) { fprintf(log, "LoadTxd(stream): RwStreamFindChunk failed\n"); fflush(log); fclose(log); }
#endif
	printf("Failed to load TXD\n");
	return false;
}

bool
CTxdStore::LoadTxd(int slot, const char *filename)
{
	RwStream *stream;
	bool ret;

	ret = false;
#ifdef WEBOS_TOUCHPAD
	FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
	if (log) { fprintf(log, "LoadTxd: Attempting to load '%s'\n", filename); fflush(log); }
#endif
	_rwD3D8TexDictionaryEnableRasterFormatConversion(true);
#ifdef WEBOS_TOUCHPAD
	// WEBOS_TOUCHPAD: Build full path and try case-insensitive file loading
	char fullPath[512];
	char altFilename[256];

	// Get current directory from CFileMgr
	snprintf(fullPath, sizeof(fullPath), "%s%s", CFileMgr::GetDirName(), filename);

	if (log) { fprintf(log, "LoadTxd: Trying full path: '%s'\n", fullPath); fflush(log); }

	// 1. Try original filename first
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fullPath);
	if (stream == nil) {
		// 2. Try uppercase version (e.g., "splash1.txd" -> "SPLASH1.TXD")
		snprintf(fullPath, sizeof(fullPath), "%s%s", CFileMgr::GetDirName(), filename);
		// Find the filename part and convert to uppercase
		char *filenameStart = strrchr(fullPath, '/');
		if (!filenameStart) filenameStart = fullPath;
		else filenameStart++;

		for (char *p = filenameStart; *p; p++) {
			*p = toupper(*p);
		}

		if (log) { fprintf(log, "LoadTxd: Original case failed, trying uppercase: '%s'\n", fullPath); fflush(log); }
		stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fullPath);

		if (stream == nil) {
			// 3. Try lowercase version
			for (char *p = filenameStart; *p; p++) {
				*p = tolower(*p);
			}

			if (log) { fprintf(log, "LoadTxd: Uppercase failed, trying lowercase: '%s'\n", fullPath); fflush(log); }
			stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fullPath);

			if (stream == nil) {
				if (log) { fprintf(log, "LoadTxd: All case variations failed, file not found\n"); fflush(log); fclose(log); }
				return false;
			}
		}
	}
	if (log) { fprintf(log, "LoadTxd: RwStreamOpen succeeded, about to parse TXD...\n"); fflush(log); }
#else
	do
		stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	while(stream == nil);
#endif
	ret = LoadTxd(slot, stream);
#ifdef WEBOS_TOUCHPAD
	if (log) { fprintf(log, "LoadTxd: Parse completed, ret=%d\n", ret); fflush(log); fclose(log); }
#endif
	RwStreamClose(stream, nil);
	return ret;
}

bool
CTxdStore::StartLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
		def->texDict = RwTexDictionaryGtaStreamRead1(stream);
		return def->texDict != nil;
	}else{
		printf("Failed to load TXD\n");
		return false;
	}
}

bool
CTxdStore::FinishLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	def->texDict = RwTexDictionaryGtaStreamRead2(stream, def->texDict);
	return def->texDict != nil;
}

void
CTxdStore::RemoveTxd(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	def->texDict = nil;
}
