//
//  lua50_profiler.h
//  RDTower
//
//  Created by changshuai on 14/11/10.
//
//

#ifndef RDTower_lua50_profiler_h
#define RDTower_lua50_profiler_h


#ifdef __cplusplus
extern "C" {
#endif

#include "tolua++.h"
int luaopen_profiler(lua_State *L);
#ifdef __cplusplus
}
#endif

#endif
