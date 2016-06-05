/*
Copyright (c) 2015-2016 Steven Stanfield

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
*/
#include "loWcore.h"
#include "GL/glew.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include <time.h>
#include <sys/time.h>
#include <GL/glx.h>


typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

typedef struct {
	Display              *dpy;
	Window               root;
	XVisualInfo          *vi;
	Colormap             cmap;
	XSetWindowAttributes swa;
	Window               win;
	GLXContext           glc;
	XWindowAttributes    gwa;
	XWindowAttributes    rootAttr;
	unsigned long        valuemask;
	loWBool              fullScreen;
	unsigned int         preFullscreenWidth;
	unsigned int         preFullscreenHeight;
	loWCallbacks         *callbacks;
	loWBool              looping;
	char                 *title;
	unsigned long        metaKeys;
	Atom                 wmProtocols;
	Atom                 wmDeleteWindow;
} CTX;

static loWCallbacks NULLCallbacks;

static GLboolean chooseFBConfig(Display *dpy, GLXFBConfig *result)
{
	GLXFBConfig* nativeConfigs;
	int i, nativeCount, usableCount;
	const char* vendor;
	GLboolean trustWindowBit = GL_TRUE;

	vendor = glXGetClientString(dpy, GLX_VENDOR);
	if (strcmp(vendor, "Chromium") == 0) {
		// HACK: This is a (hopefully temporary) workaround for Chromium
		// (VirtualBox GL) not setting the window bit on any GLXFBConfigs
		trustWindowBit = GL_FALSE;
	}

	nativeConfigs = glXGetFBConfigs(dpy,
									0, //_glfw.x11.screen,
									&nativeCount);

	if (!nativeCount) {
		printf("GLX: No GLXFBConfigs returned\n");
		return GL_FALSE;
	}

	usableCount = 0;

	for (i = 0; i < nativeCount; i++) {
		const GLXFBConfig n = nativeConfigs[i];
		int doubleBuffer;
		int visualId;
		int renderType;
		int drawableType;
		glXGetFBConfigAttrib(dpy, n, GLX_DOUBLEBUFFER, &doubleBuffer);
		glXGetFBConfigAttrib(dpy, n, GLX_VISUAL_ID, &visualId);
		glXGetFBConfigAttrib(dpy, n, GLX_RENDER_TYPE, &renderType);
		glXGetFBConfigAttrib(dpy, n, GLX_DRAWABLE_TYPE, &drawableType);

		if (!doubleBuffer || !visualId) {
			// Only consider double-buffered GLXFBConfigs with associated visuals
			continue;
		}

		if (!(renderType & GLX_RGBA_BIT)) {
			// Only consider RGBA GLXFBConfigs
			continue;
		}

		if (!(drawableType & GLX_WINDOW_BIT)) {
			if (trustWindowBit) {
				// Only consider window GLXFBConfigs
				continue;
			}
		}
		*result = n;
		usableCount++;
	}
	printf("Usable Count %d\n", usableCount);

	XFree(nativeConfigs);

	return usableCount ? GL_TRUE : GL_FALSE;
}

static KeySym CodeToSym(Display *dpy, unsigned int code) {
	int keysyms_per_keycode_return;
	KeySym ret;
	KeySym *keysym = XGetKeyboardMapping(dpy,
										 code,
										 1,
										 &keysyms_per_keycode_return);
	ret = keysym[0];
	XFree(keysym);
	return ret;
}

static void metaPress(CTX *ctx, KeySym keysym) {
	switch (keysym) {
	case XK_Shift_L: ctx->metaKeys |= loW_Shift_L; break;
	case XK_Shift_R: ctx->metaKeys |= loW_Shift_R; break;
	case XK_Control_L: ctx->metaKeys |= loW_Control_L; break;
	case XK_Control_R: ctx->metaKeys |= loW_Control_R; break;
	case XK_Caps_Lock: ctx->metaKeys |= loW_Caps_Lock; break;
	case XK_Shift_Lock: ctx->metaKeys |= loW_Shift_Lock; break;
	case XK_Meta_L: ctx->metaKeys |= loW_Meta_L; break;
	case XK_Meta_R: ctx->metaKeys |= loW_Meta_R; break;
	case XK_Alt_L: ctx->metaKeys |= loW_Alt_L; break;
	case XK_Alt_R: ctx->metaKeys |= loW_Alt_R; break;
	case XK_Super_L: ctx->metaKeys |= loW_Super_L; break;
	case XK_Super_R: ctx->metaKeys |= loW_Super_R; break;
	case XK_Hyper_L: ctx->metaKeys |= loW_Hyper_L; break;
	case XK_Hyper_R: ctx->metaKeys |= loW_Hyper_R; break;
	}
}

static void metaRelease(CTX *ctx, KeySym keysym) {
	switch (keysym) {
	case XK_Shift_L: ctx->metaKeys &= ~loW_Shift_L; break;
	case XK_Shift_R: ctx->metaKeys &= ~loW_Shift_R; break;
	case XK_Control_L: ctx->metaKeys &= ~loW_Control_L; break;
	case XK_Control_R: ctx->metaKeys &= ~loW_Control_R; break;
	case XK_Caps_Lock: ctx->metaKeys &= ~loW_Caps_Lock; break;
	case XK_Shift_Lock: ctx->metaKeys &= ~loW_Shift_Lock; break;
	case XK_Meta_L: ctx->metaKeys &= ~loW_Meta_L; break;
	case XK_Meta_R: ctx->metaKeys &= ~loW_Meta_R; break;
	case XK_Alt_L: ctx->metaKeys &= ~loW_Alt_L; break;
	case XK_Alt_R: ctx->metaKeys &= ~loW_Alt_R; break;
	case XK_Super_L: ctx->metaKeys &= ~loW_Super_L; break;
	case XK_Super_R: ctx->metaKeys &= ~loW_Super_R; break;
	case XK_Hyper_L: ctx->metaKeys &= ~loW_Hyper_L; break;
	case XK_Hyper_R: ctx->metaKeys &= ~loW_Hyper_R; break;
	}
}

loWContext loWinit(loWCallbacks *c, int width, int height, loWBool startFullscreen) {
	CTX *ctx;
	ctx = malloc(sizeof(CTX));
	memset(ctx, 0, sizeof(CTX));
	memset(&NULLCallbacks, 0, sizeof(NULLCallbacks));

	ctx->callbacks = c==NULL?&NULLCallbacks:c;
	GLint att[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		None
	};


	ctx->dpy = XOpenDisplay(NULL);

	if(ctx->dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		free(ctx);
		return NULL;
	}

	ctx->root = DefaultRootWindow(ctx->dpy);

	ctx->vi = glXChooseVisual(ctx->dpy, 0, att);

	if(ctx->vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		free(ctx);
		return NULL;
	}
	else {
		printf("\n\tvisual %p selected\n", (void *)ctx->vi->visualid); /* %p creates hexadecimal output like in glxinfo */
	}


	ctx->cmap = XCreateColormap(ctx->dpy, ctx->root, ctx->vi->visual, AllocNone);

	ctx->swa.colormap = ctx->cmap;
	ctx->swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
						  ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	XGetWindowAttributes(ctx->dpy, ctx->root, &ctx->rootAttr);

	ctx->valuemask = CWColormap | CWEventMask;
	printf("w: %d, h: %d\n", ctx->rootAttr.width, ctx->rootAttr.height);

	if (startFullscreen) {
		ctx->preFullscreenWidth = ctx->rootAttr.width;
		ctx->preFullscreenHeight = ctx->rootAttr.height;
		ctx->swa.override_redirect = True;
		ctx->valuemask |= CWOverrideRedirect;
		width = ctx->rootAttr.width;
		height = ctx->rootAttr.height;
		ctx->fullScreen = loW_true;
	}
	ctx->win = XCreateWindow(ctx->dpy, ctx->root, 0, 0, width, height, 0,
							 ctx->vi->depth, InputOutput, ctx->vi->visual,
							 ctx->valuemask, &ctx->swa);

	XMapWindow(ctx->dpy, ctx->win);
	XStoreName(ctx->dpy, ctx->win, "");
	XGetWindowAttributes(ctx->dpy, ctx->win, &ctx->gwa);
	ctx->wmProtocols = XInternAtom(ctx->dpy, "WM_PROTOCOLS", loW_false);
	ctx->wmDeleteWindow = XInternAtom(ctx->dpy, "WM_DELETE_WINDOW", loW_false);
	XSetWMProtocols(ctx->dpy, ctx->win, &ctx->wmDeleteWindow, 1);

	// NOTE: It is not necessary to create or make current to a context before
	// calling glXGetProcAddressARB
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
			glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );


	if (glXCreateContextAttribsARB)
	{
		int nbConfigs = 0;
		GLXFBConfig config;
		if (chooseFBConfig(ctx->dpy, &config))
		{
			int flags = 0;
//			flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
//			flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

			// Create the context
			int attributes[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 3,
				GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
				GLX_CONTEXT_FLAGS_ARB, flags,
				None, None
			};
			ctx->glc = glXCreateContextAttribsARB(ctx->dpy, config, 0, 1, attributes);
		}

		glXMakeCurrent(ctx->dpy, ctx->win, ctx->glc);
	}

	glEnable(GL_DEPTH_TEST);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("GLEW Error: %s\n", glewGetErrorString(err));
		free(ctx);
		return NULL;
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	if (ctx->fullScreen)
		XGrabKeyboard(ctx->dpy, ctx->win, 0, GrabModeAsync, GrabModeAsync, CurrentTime);
	return (void *)ctx;
}

void loWsetCallbacks(loWContext context, loWCallbacks *c) {
	CTX *ctx = (CTX *)context;
	ctx->callbacks = c==NULL?&NULLCallbacks:c;
}

void loWshutdown(loWContext context) {
	CTX *ctx = (CTX *)context;
	ctx->looping = loW_false;
}

void loWloop(loWContext context) {
	CTX *ctx = (CTX *)context;
	XEvent xev;

	if (ctx->callbacks->init) ctx->callbacks->init(ctx);
	ctx->looping = loW_true;

	while(ctx->looping) {
		if (XPending(ctx->dpy)) {
			XNextEvent(ctx->dpy, &xev);

			KeySym keysym;
			switch (xev.type) {
			case Expose:
				XGetWindowAttributes(ctx->dpy, ctx->win, &ctx->gwa);
				glViewport(0, 0, ctx->gwa.width, ctx->gwa.height);
				if (ctx->callbacks->expose) ctx->callbacks->expose(ctx,
				                                                   ctx->gwa.width,
				                                                   ctx->gwa.height);
				break;
			case KeyPress:
				keysym = CodeToSym(ctx->dpy, xev.xkey.keycode);
				metaPress(ctx, keysym);
				if (ctx->callbacks->keyPress) ctx->callbacks->keyPress(ctx, keysym, ctx->metaKeys);
				break;
			case KeyRelease:
				keysym = CodeToSym(ctx->dpy, xev.xkey.keycode);
				metaRelease(ctx, keysym);
				if (ctx->callbacks->keyRelease) ctx->callbacks->keyRelease(ctx, keysym, ctx->metaKeys);
				break;
			case ButtonPress:
				if (ctx->callbacks->buttonPress)
					ctx->callbacks->buttonPress(ctx, xev.xbutton.button,
					                            xev.xbutton.x, xev.xbutton.y, ctx->metaKeys);
				break;
			case ButtonRelease:
				if (ctx->callbacks->buttonRelease)
					ctx->callbacks->buttonRelease(ctx, xev.xbutton.button,
					                              xev.xbutton.x, xev.xbutton.y, ctx->metaKeys);
				break;
			case MotionNotify:
				if (ctx->callbacks->mouseMove)
					ctx->callbacks->mouseMove(ctx, xev.xbutton.x, xev.xbutton.y,
					                            ctx->metaKeys);
				break;
			case ClientMessage:
				if (xev.xclient.message_type == ctx->wmProtocols &&
					xev.xclient.data.l[0] == ctx->wmDeleteWindow)
					loWshutdown(ctx);
				break;
			}
		} else {
			if (ctx->callbacks->idle) ctx->callbacks->idle(ctx);
		}
	}
	glXMakeCurrent(ctx->dpy, None, NULL);
	glXDestroyContext(ctx->dpy, ctx->glc);
	XDestroyWindow(ctx->dpy, ctx->win);
	XCloseDisplay(ctx->dpy);
	ctx->callbacks->shutdown(ctx);
	if (ctx->title) free(ctx->title);
	free(ctx);
}

void loWswapBuffers(loWContext context) {
	CTX *ctx = (CTX *)context;
	glXSwapBuffers(ctx->dpy, ctx->win);
}

void loWtoggleFullscreen(loWContext context) {
	CTX *ctx = (CTX *)context;
	int width;
	int height;
	if (ctx->fullScreen) {
		ctx->swa.override_redirect = False;
		ctx->valuemask &= ~CWOverrideRedirect;
		width = ctx->preFullscreenWidth;
		height = ctx->preFullscreenHeight;
		XUngrabKeyboard(ctx->dpy, CurrentTime);
		ctx->fullScreen = loW_false;
	} else {
		ctx->preFullscreenWidth = ctx->gwa.width;
		ctx->preFullscreenHeight = ctx->gwa.height;
		ctx->swa.override_redirect = True;
		ctx->valuemask |= CWOverrideRedirect;
		width = ctx->rootAttr.width;
		height = ctx->rootAttr.height;
		ctx->fullScreen = loW_true;
	}
	XDestroyWindow(ctx->dpy, ctx->win);
	ctx->win = XCreateWindow(ctx->dpy, ctx->root, 0, 0,
						width, height, 0,
						ctx->vi->depth, InputOutput, ctx->vi->visual,
						ctx->valuemask, &ctx->swa);
	XMapWindow(ctx->dpy, ctx->win);
	XStoreName(ctx->dpy, ctx->win, ctx->title?ctx->title:"");
	XSetWMProtocols(ctx->dpy, ctx->win, &ctx->wmDeleteWindow, 1);
	glXMakeCurrent(ctx->dpy, ctx->win, ctx->glc);
	if (ctx->fullScreen) XGrabKeyboard(ctx->dpy, ctx->win, 0, GrabModeAsync, GrabModeAsync, CurrentTime);
}

loWBool loWisFullscreen(loWContext context) {
	CTX *ctx = (CTX *)context;
	return ctx->fullScreen;
}

void loWsetWindowTitle(loWContext context, const char *title) {
	CTX *ctx = (CTX *)context;
	if (ctx->title) free(ctx->title);
	if (title) {
		int len;
		for (len = 0; len < 512 && title[len]; len++);
		ctx->title = (char *)malloc(len+1);
		memset(ctx->title, 0, len+1);
		strncpy(ctx->title, title, len);
		XStoreName(ctx->dpy, ctx->win, ctx->title);
	} else {
		ctx->title = NULL;
		XStoreName(ctx->dpy, ctx->win, "");
	}
}

unsigned long loWgetMetaKeys(loWContext context) {
	CTX *ctx = (CTX *)context;
	return ctx->metaKeys;
}

void loWgetWidthHeight(loWContext context, int *width, int *height) {
	CTX *ctx = (CTX *)context;
	*width = ctx->gwa.width;
	*height = ctx->gwa.height;
}

void loWconvertWinToGL(loWContext context, const int winx, const int winy,
                    float *glx, float *gly) {
	CTX *ctx = (CTX *)context;
	int hw = ctx->gwa.width / 2;
	int hh = ctx->gwa.height / 2;
	*glx = (float)(winx-hw)/(float)hw;
	*gly = -(float)(winy-hh)/(float)hh;
}

double loWgetSeconds()
{
	// http://linux/die/netman3/clock_gettime
//#if defined(CLOCK_MONOTONIC) // If we can use clock_gettime, which has nanosecond precision...
//	if(MonotonicClockAvailable)
//	{
//		struct timespec ts;
//		clock_gettime(CLOCK_MONOTONIC_RAW, &ts); // Better to use CLOCK_MONOTONIC than CLOCK_REALTIME.
//		return (double)ts.tv_sec + ((double)ts.tv_nsec / 1E9);
//	}
//#endif

	// We cannot use rdtsc because its frequency changes at runtime.
	struct timeval tv;
	gettimeofday(&tv, 0);

	return (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);
}

void loWnanoSleep(unsigned long nsec) {
	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = nsec;
	nanosleep(&tv, NULL);
}
