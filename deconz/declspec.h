#ifndef DECONZ_DECLSPEC_H
#define DECONZ_DECLSPEC_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifdef __GNUC__
  #define DECONZ_VISIBILITY_AVAILABLE
#endif

#ifndef DECONZ_DECL_IMPORT
    #ifdef DECONZ_VISIBILITY_AVAILABLE
      #define DECONZ_DECL_IMPORT     __attribute__((visibility("default")))
      #define DECONZ_DECL_EXPORT     __attribute__((visibility("default")))
    #endif

    #ifdef _WIN32
      #ifndef DECONZ_VISIBILITY_AVAILABLE
        #define DECONZ_DECL_IMPORT     __declspec(dllimport)
        #define DECONZ_DECL_EXPORT     __declspec(dllexport)
      #endif
    #endif
#endif /* DECONZ_DECL_IMPORT */

#endif // DECONZ_DECLSPEC_H
