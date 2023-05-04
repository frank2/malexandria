#ifndef __MALEXANDRIA_PLATFORM_HPP
#define __MALEXANDRIA_PLATFORM_HPP

#if defined(_WIN32) || defined(WIN32)
#define MALEXANDRIA_WIN32
#endif

/*
#if defined(MALEXANDRIA_SHARED)

#if defined(MALEXANDRIA_WIN32)

#if defined(MALEXANDRIA_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#else
#define EXPORT __attribute__((visibility("default")))
#endif

#else
#define EXPORT
#endif
*/

#if defined(MALEXANDRIA_WIN32)
#define PACK(alignment) __pragma(pack(push, alignment))
#define UNPACK() __pragma(pack(pop))
#else
#define PACK(alignment) __attribute__((packed,aligned(alignment)))
#define UNPACK()
#endif

#endif
