#pragma once
#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__
#include <xmmintrin.h>
#ifdef __clang__
#define INLINE inline __attribute__((__always_inline__))
#define ALIGN(_n) __attribute__((aligned(_n)))
#define clz(n) __builtin_clz(n)
#define prefetch(ad) __builtin_prefetch(ad, 0, 1)
#endif
#ifdef _MSC_VER
#define INLINE __forceinline
#define ALIGN(_n) __declspec(align(_n))
#define clz(n) __lzcnt(n)
#define prefetch(ad) _mm_prefetch(ad, 3)
#endif
#ifdef __GNUC__
#define INLINE inline __attribute__((__always_inline__))
#define ALIGN(_n) __attribute__((aligned(_n)))
#define clz(n) __builtin_clz(n)
#define prefetch(ad) __builtin_prefetch(ad, 0, 1)
#endif
#ifdef _MSC_VER
#define INLINE __forceinline
#define ALIGN(_n) __declspec(align(_n))
#define clz(n) __lzcnt(n)
#define prefetch(ad) _mm_prefetch(ad, 3)
#endif
/*
#ifdef _WIN32
#define cpuid(info, x)    __cpuidex(info, x, 0)
#else
#include <cpuid.h>

void cpuid(int info[4], int InfoType){
        __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}

#endif
*/

#endif
