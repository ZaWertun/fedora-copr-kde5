/*
 *  This file is part of libkolab.
 *  SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

// #include <kdemacros.h>

#ifndef KOLAB_EXPORT
#if defined(KOLAB_STATIC_LIBS)
/* No export/import for static libraries */
#define KOLAB_EXPORT
#elif defined(MAKE_KOLAB_LIB)
/* We are building this library */
#define KOLAB_EXPORT __attribute__((visibility("default")))
#else
/* We are using this library */
#define KOLAB_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef KOLAB_EXPORT_DEPRECATED
#define KOLAB_EXPORT_DEPRECATED KDE_DEPRECATED __attribute__((visibility("default")))
#endif

/**
 *  @namespace Kolab
 *
 *  @brief
 *  Contains all the KOLAB library global classes, objects, and functions.
 */
