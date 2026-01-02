#include "common.h"


#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "rwcore.h"

#include "skeleton.h"
#include "platform.h"
#include "main.h"
#include "MemoryHeap.h"
#include "crossplatform.h"

static RwBool               DefaultVideoMode = TRUE;

RsGlobalType                RsGlobal;

#ifdef _WIN32
RwUInt32    
#else
double
#endif
RsTimer(void)
{
	return psTimer();
}


/*
 *****************************************************************************
 */
void
RsCameraShowRaster(RwCamera * camera)
{
	psCameraShowRaster(camera);

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsCameraBeginUpdate(RwCamera * camera)
{
	return psCameraBeginUpdate(camera);
}

/*
 *****************************************************************************
 */
RwImage*
RsGrabScreen(RwCamera *camera)
{
	return psGrabScreen(camera);
}

/*
 *****************************************************************************
 */
RwBool
RsRegisterImageLoader(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 */
static              RwBool
RsSetDebug(void)
{
	return TRUE;
}

/*
 *****************************************************************************
 */
void
RsMouseSetPos(RwV2d * pos)
{
	psMouseSetPos(pos);

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsSelectDevice(void)
{
	return psSelectDevice();
}

/*
 *****************************************************************************
 */
RwBool
RsInputDeviceAttach(RsInputDeviceType inputDevice,
					RsInputEventHandler inputEventHandler)
{
	switch (inputDevice)
	{
		case rsKEYBOARD:
			{
				RsGlobal.keyboard.inputEventHandler = inputEventHandler;
				RsGlobal.keyboard.used = TRUE;
				break;
			}
		case rsMOUSE:
			{
				RsGlobal.mouse.inputEventHandler = inputEventHandler;
				RsGlobal.mouse.used = TRUE;
				break;
			}
		case rsPAD:
			{
				RsGlobal.pad.inputEventHandler = inputEventHandler;
				RsGlobal.pad.used = TRUE;
				break;
			}
		default:
			{
				return FALSE;
			}
	}

	return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
rsCommandLine(RwChar *arg)
{
	RsEventHandler(rsFILELOAD, arg);

	return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
rsPreInitCommandLine(RwChar *arg)
{
	if( !strcmp(arg, RWSTRING("-vms")) )
	{
		DefaultVideoMode = FALSE;

		return TRUE;
	}
#ifndef MASTER
	if (!strcmp(arg, RWSTRING("-animviewer")))
	{
		gbModelViewer = TRUE;

		return TRUE;
	}
#endif
	return FALSE;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsKeyboardEventHandler(RsEvent event, void *param)
{
	if (RsGlobal.keyboard.used)
	{
		return RsGlobal.keyboard.inputEventHandler(event, param);
	}

	return rsEVENTNOTPROCESSED;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsPadEventHandler(RsEvent event, void *param)
{
	if (RsGlobal.pad.used)
	{
		return RsGlobal.pad.inputEventHandler(event, param);
	}

	return rsEVENTNOTPROCESSED;
}

/*
 *****************************************************************************
 */
RsEventStatus
RsEventHandler(RsEvent event, void *param)
{
	RsEventStatus       result;
	RsEventStatus       es;
  
	/*
	 * Give the application an opportunity to override any events...
	 */
	es = AppEventHandler(event, param);

	/*
	 * We never allow the app to replace the quit behaviour,
	 * only to intercept...
	 */
	if (event == rsQUITAPP)
	{
		/*
		 * Set the flag which causes the event loop to exit...
		 */
		RsGlobal.quit = TRUE;
	}

	if (es == rsEVENTNOTPROCESSED)
	{
		switch (event)
		{
			case rsSELECTDEVICE:
				result =
					(RsSelectDevice()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsCOMMANDLINE:
				result = (rsCommandLine((RwChar *) param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;
			case rsPREINITCOMMANDLINE:
				result = (rsPreInitCommandLine((RwChar *) param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;
			case rsINITDEBUG:
				result =
					(RsSetDebug()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsREGISTERIMAGELOADER:
				result = (RsRegisterImageLoader()?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsRWTERMINATE:
				RsRwTerminate();
				result = (rsEVENTPROCESSED);
				break;

			case rsRWINITIALIZE:
				result = (RsRwInitialize(param) ?
						  rsEVENTPROCESSED : rsEVENTERROR);
				break;

			case rsTERMINATE:
				RsTerminate();
				result = (rsEVENTPROCESSED);
				break;

			case rsINITIALIZE:
				result =
					(RsInitialize()? rsEVENTPROCESSED : rsEVENTERROR);
				break;

			default:
				result = (es);
				break;

		}
	}
	else
	{
		result = (es);
	}

	return result;
}

/*
 *****************************************************************************
 */
void
RsRwTerminate(void)
{
	/* Close RenderWare */

	RwEngineStop();
	RwEngineClose();
	RwEngineTerm();

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsRwInitialize(void *displayID)
{
	RwEngineOpenParams  openParams;

	FILE *log = NULL; // fopen("/media/internal/.gta3/debug.log", "a");
	if (log) { fprintf(log, "[RsRwInitialize] Starting RenderWare initialization\n"); fflush(log); }

	PUSH_MEMID(MEMID_RENDER);	// NB: not popped on failed return

	/*
	 * Start RenderWare...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling RwEngineInit...\n"); fflush(log); }
	if (!RwEngineInit(psGetMemoryFunctions(), 0, rsRESOURCESDEFAULTARENASIZE))
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: RwEngineInit failed\n"); fclose(log); }
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] RwEngineInit succeeded\n"); fflush(log); }

	/*
	 * Install any platform specific file systems...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling psInstallFileSystem...\n"); fflush(log); }
	psInstallFileSystem();
	if (log) { fprintf(log, "[RsRwInitialize] psInstallFileSystem succeeded\n"); fflush(log); }

	/*
	 * Initialize debug message handling...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling RsEventHandler(rsINITDEBUG)...\n"); fflush(log); }
	RsEventHandler(rsINITDEBUG, nil);
	if (log) { fprintf(log, "[RsRwInitialize] rsINITDEBUG succeeded\n"); fflush(log); }

	/*
	 * Attach all plugins...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling RsEventHandler(rsPLUGINATTACH)...\n"); fflush(log); }
	if (RsEventHandler(rsPLUGINATTACH, nil) == rsEVENTERROR)
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: rsPLUGINATTACH failed\n"); fclose(log); }
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] rsPLUGINATTACH succeeded\n"); fflush(log); }

	/*
	 * Attach input devices...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling RsEventHandler(rsINPUTDEVICEATTACH)...\n"); fflush(log); }
	if (RsEventHandler(rsINPUTDEVICEATTACH, nil) == rsEVENTERROR)
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: rsINPUTDEVICEATTACH failed\n"); fclose(log); }
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] rsINPUTDEVICEATTACH succeeded\n"); fflush(log); }

	openParams.displayID = displayID;

	if (log) { fprintf(log, "[RsRwInitialize] Calling RwEngineOpen...\n"); fflush(log); }
	if (!RwEngineOpen(&openParams))
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: RwEngineOpen failed\n"); fclose(log); }
		RwEngineTerm();
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] RwEngineOpen succeeded\n"); fflush(log); }

#ifdef LIBRW
	// WEBOS/UNIX: Override LibRW's file functions to use case-insensitive file opening
	// LibRW uses standard fopen() which is case-sensitive on Unix systems
	// We need fcaseopen() for case-insensitive access to game data files
	#ifndef _WIN32
	if (log) { fprintf(log, "[RsRwInitialize] Overriding LibRW file functions with case-insensitive versions\n"); fflush(log); }

	// Override rwfopen to use fcaseopen (case-insensitive)
	// fcaseopen is defined in crossplatform.h as a macro to _fcaseopen
	rw::engine->filefuncs.rwfopen = (void *(*)(const char*, const char*))fcaseopen;
	// Keep other functions as-is (they work with FILE*)
	// rwfclose, rwfseek, rwftell, rwfread, rwfwrite remain standard functions

	if (log) { fprintf(log, "[RsRwInitialize] File function override complete\n"); fflush(log); }
	#endif
#endif

	if (log) { fprintf(log, "[RsRwInitialize] Calling RsEventHandler(rsSELECTDEVICE)...\n"); fflush(log); }
	if (RsEventHandler(rsSELECTDEVICE, displayID) == rsEVENTERROR)
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: rsSELECTDEVICE failed\n"); fclose(log); }
		RwEngineClose();
		RwEngineTerm();
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] rsSELECTDEVICE succeeded\n"); fflush(log); }

	if (log) { fprintf(log, "[RsRwInitialize] Calling RwEngineStart...\n"); fflush(log); }
	if (!RwEngineStart())
	{
		if (log) { fprintf(log, "[RsRwInitialize] ERROR: RwEngineStart failed\n"); fclose(log); }
		RwEngineClose();
		RwEngineTerm();
		return (FALSE);
	}
	if (log) { fprintf(log, "[RsRwInitialize] RwEngineStart succeeded\n"); fflush(log); }

	/*
	 * Register loaders for an image with a particular file extension...
	 */
	if (log) { fprintf(log, "[RsRwInitialize] Calling RsEventHandler(rsREGISTERIMAGELOADER)...\n"); fflush(log); }
	RsEventHandler(rsREGISTERIMAGELOADER, nil);
	if (log) { fprintf(log, "[RsRwInitialize] rsREGISTERIMAGELOADER succeeded\n"); fflush(log); }

	if (log) { fprintf(log, "[RsRwInitialize] Calling psNativeTextureSupport...\n"); fflush(log); }
	psNativeTextureSupport();
	if (log) { fprintf(log, "[RsRwInitialize] psNativeTextureSupport succeeded\n"); fflush(log); }

	if (log) { fprintf(log, "[RsRwInitialize] Setting texture mipmapping...\n"); fflush(log); }
	RwTextureSetMipmapping(FALSE);
	RwTextureSetAutoMipmapping(FALSE);
	if (log) { fprintf(log, "[RsRwInitialize] Texture mipmapping set\n"); fflush(log); }

	POP_MEMID();

	if (log) { fprintf(log, "[RsRwInitialize] RenderWare initialization complete!\n"); fclose(log); }
	return TRUE;
}

/*
 *****************************************************************************
 */
void
RsTerminate(void)
{
	psTerminate();

	return;
}

/*
 *****************************************************************************
 */
RwBool
RsInitialize(void)
{
	/*
	 * Initialize Platform independent data...
	 */
	RwBool              result;

	RsGlobal.appName = RWSTRING("GTA3");
	RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
	RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
	RsGlobal.width = DEFAULT_SCREEN_WIDTH;
	RsGlobal.height = DEFAULT_SCREEN_HEIGHT;
	
	RsGlobal.maxFPS = 30;
	 
	RsGlobal.quit = FALSE;

	/* setup the keyboard */
	RsGlobal.keyboard.inputDeviceType = rsKEYBOARD;
	RsGlobal.keyboard.inputEventHandler = nil;
	RsGlobal.keyboard.used = FALSE;

	/* setup the mouse */
	RsGlobal.mouse.inputDeviceType = rsMOUSE;
	RsGlobal.mouse.inputEventHandler = nil;
	RsGlobal.mouse.used = FALSE;

	/* setup the pad */
	RsGlobal.pad.inputDeviceType = rsPAD;
	RsGlobal.pad.inputEventHandler = nil;
	RsGlobal.pad.used = FALSE;

	result = psInitialize();

	return result;
}
