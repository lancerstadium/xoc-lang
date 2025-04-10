/**
 * @file    xoc_api.h
 * @brief   XOC API header file
 * @author  lancerstadium
 * @details This file contains the declarations for the XOC API functions.
 *          It is designed to be included in both C and C++ projects.
 * @note    Use standard bare C headers for cross-compatibility:
 *          stdint.h, stdbool.h, stddef.h
 */

#ifndef XOC_API_H
#define XOC_API_H

#ifdef _WIN32
#   if defined(XOC_BUILD_STATIC)
#       define XOC_EXPORT
#       define XOC_IMPORT
#   else
#       define XOC_EXPORT __declspec(dllexport)
#       define XOC_IMPORT __declspec(dllimport)
#   endif
#else
#   define XOC_EXPORT __attribute__((visibility("default")))
#   define XOC_IMPORT __attribute__((visibility("default")))
#endif
#ifdef XOC_BUILD_DYNAMIC
#   define XOC_API XOC_EXPORT
#else
#   define XOC_API XOC_IMPORT
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus



int maxx(int a, int b) ;









#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // XOC_API_H