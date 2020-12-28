#ifndef DRIVER2_H
#define DRIVER2_H

#include <stdbool.h>
#include <stdio.h>

#include "KERNEL.H"
#include "TYPES.H"
#include "LIBCD.H"
#include "LIBGTE.H"
#include "LIBGPU.H"
#include "LIBSPU.H"

#include "PSYX_COMPAT.H"

#ifdef PSX
// TODO: Include PSX STUFF
#define trap(code) printf("ERROR OCCURED!\n")

#define printMsg					(void)
#define printInfo					(void)
#define printWarning				(void)
#define printError					(void)

#else

// from redriver2_psxpc
void printMsg(char *fmt, ...);
void printInfo(char *fmt, ...);
void printWarning(char *fmt, ...);
void printError(char *fmt, ...);

#if _MSC_VER >= 1400
#define trap(ode) {printError("EXCEPTION code: %x\n", ode); __debugbreak();}
#elif defined(__GNUC__)
#define trap(ode) {__asm__("int3");}
#else
#define trap(ode) {_asm int 0x03}
#endif

#endif // PSX

#include "REVERSING.H"

#include "VERSION.H"
#include "DR2MATH.H"
#include "DR2LIMITS.H"
#include "DR2TYPES.H"
#include "DR2LOCALE.H"

#endif // DRIVER2_H
