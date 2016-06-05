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
#ifndef LOWCORE_H
#define LOWCORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char loWBool;
#define loW_true  1
#define loW_false 0

// Bitflags for various meta keys.
#define loW_Shift_L                       (1L<<0)  /* Left shift */
#define loW_Shift_R                       (1L<<1)  /* Right shift */
#define loW_Shift                         (loW_Shift_L|loW_Shift_R)
#define loW_Control_L                     (1L<<2)  /* Left control */
#define loW_Control_R                     (1L<<3)  /* Right control */
#define loW_Control                       (loW_Control_L|loW_Control_R)
#define loW_Caps_Lock                     (1L<<4)  /* Caps lock */
#define loW_Shift_Lock                    (1L<<5)  /* Shift lock */
#define loW_Meta_L                        (1L<<6)  /* Left meta */
#define loW_Meta_R                        (1L<<7)  /* Right meta */
#define loW_Meta                          (loW_Meta_L|loW_Meta_R)
#define loW_Alt_L                         (1L<<8)  /* Left alt */
#define loW_Alt_R                         (1L<<9)  /* Right alt */
#define loW_Alt                           (loW_Alt_L|loW_Alt_R)
#define loW_Super_L                       (1L<<10)  /* Left super */
#define loW_Super_R                       (1L<<11)  /* Right super */
#define loW_Super                         (loW_Super_L|loW_Super_R)
#define loW_Hyper_L                       (1L<<12)  /* Left hyper */
#define loW_Hyper_R                       (1L<<13)  /* Right hyper */
#define loW_Hyper                         (loW_Hyper_L|loW_Hyper_R)

// Mouse buttons.
#define LoW_LeftMouse       1
#define LoW_CenterMouse     2
#define LoW_RightMouse      3
#define LoW_ScrollUpMouse   4
#define LoW_ScrollDownMouse 5

typedef void *loWContext;

typedef struct {
	void (*init)(loWContext ctx);
	void (*expose)(loWContext ctx, int width, int height);
	void (*shutdown)(loWContext ctx);
	void (*idle)(loWContext ctx);

	void (*keyPress)(loWContext ctx, unsigned long keysym, unsigned long metaKeys);
	void (*keyRelease)(loWContext ctx, unsigned long keysym, unsigned long metaKeys);
	void (*buttonPress)(loWContext ctx, unsigned long button,
	                    int winX, int winY, unsigned long metaKeys);
	void (*buttonRelease)(loWContext ctx, unsigned long button,
	                      int winX, int winY, unsigned long metaKeys);
	void (*mouseMove)(loWContext ctx,
	                  int winX, int winY, unsigned long metaKeys);
} loWCallbacks;

loWContext loWinit(loWCallbacks *c, int width, int height, loWBool startFullscreen);
void loWsetCallbacks(loWContext context, loWCallbacks *c);
void loWshutdown(loWContext context);
void loWloop(loWContext context);

void loWswapBuffers(loWContext context);
void loWtoggleFullscreen(loWContext context);
loWBool loWisFullscreen(loWContext context);
void loWsetWindowTitle(loWContext context, const char *title);
unsigned long loWgetMetaKeys(loWContext context);
void loWgetWidthHeight(loWContext context, int *width, int *height);

void loWconvertWinToGL(loWContext context, const int winx, const int winy,
                    float *glx, float *gly);

double loWgetSeconds();
void loWnanoSleep(unsigned long nsec);

#ifdef __cplusplus
}
#endif

#endif // LOWCORE_H
