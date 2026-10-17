#pragma once

// UBT defines <MODULE>_API as DLLEXPORT / DLLIMPORT in modular builds, but those macros come from
// UE's Platform.h, which pure modules never include (ADR 0005). Provide the same definitions here
// so exported free functions in pure modules compile under UBT. The headless harness defines the
// <MODULE>_API macros as empty and never needs these.

#ifndef DLLEXPORT
	#if defined(_MSC_VER)
		#define DLLEXPORT __declspec(dllexport)
	#else
		#define DLLEXPORT __attribute__((visibility("default")))
	#endif
#endif

#ifndef DLLIMPORT
	#if defined(_MSC_VER)
		#define DLLIMPORT __declspec(dllimport)
	#else
		#define DLLIMPORT __attribute__((visibility("default")))
	#endif
#endif
