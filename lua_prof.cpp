#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <map>
#include <vector>
#include <string>
#include<algorithm>

extern "C" 
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
};

namespace lua_prof {

typedef struct tagCallData {
    char        what[32];
    char        namewhat[32];
    uint64_t    start;
    uint64_t    times;
    uint64_t    usec;
} CallData;

typedef std::map<std::string, CallData*> CallDataMap_T;
typedef std::vector< std::pair<std::string, CallData*> > CallDataVec_T;

int cmp(const std::pair<std::string, CallData*> &x,const std::pair<std::string, CallData*> &y)
{
    return x.second->usec > y.second->usec;
}

void sort_map_by_usec(CallDataMap_T& t_map, CallDataVec_T& t_vec)
{
    for(CallDataMap_T::iterator it = t_map.begin(); it != t_map.end(); it++) {
        t_vec.push_back(make_pair(it->first, it->second)); 
    }
    sort(t_vec.begin(), t_vec.end(), cmp);
}

CallDataMap_T g_call_data_map;

static int lua_prof_sleep(lua_State* L);
static int lua_prof_init(lua_State* L);
static int lua_prof_fini(lua_State* L);
static void lua_prof_hook(lua_State* L, lua_Debug* ar);

static const luaL_Reg lua_prof_lib[] = {
    {"sleep", lua_prof_sleep},
    {"init", lua_prof_init},
    {"fini", lua_prof_fini},
    {NULL, NULL},
};

extern "C" int luaopen_lua_prof(lua_State* L)
{
    luaL_newlib(L, lua_prof_lib);
    return 1;
}

static int lua_prof_sleep(lua_State* L)
{
    sleep(1);
    return 0;
}

static int lua_prof_init(lua_State* L)
{
    lua_sethook(L, lua_prof_hook, LUA_MASKCALL | LUA_MASKRET, 0);
    return 0;
}

static int lua_prof_fini(lua_State* L)
{
    lua_sethook(L, NULL, 0, 0);
    lua_createtable(L, g_call_data_map.size(), 0);
    int i = 1;
    char buf[1024] = {0};
    CallDataVec_T call_data_vec;
    sort_map_by_usec(g_call_data_map, call_data_vec);
    for(int i = 0; i < call_data_vec.size(); i++) {
        snprintf(buf, sizeof(buf), "function: %s\n[%-3s], [%-7s], times [%llu], total [%llums], avg [%lluus]",
            call_data_vec[i].first.c_str(),
            call_data_vec[i].second->what,
            call_data_vec[i].second->namewhat,
            call_data_vec[i].second->times,
            call_data_vec[i].second->usec/1000,
            (call_data_vec[i].second->usec)/(call_data_vec[i].second->times));
        lua_pushinteger(L, i+1);
        lua_pushstring(L, buf);
        lua_settable(L, -3);
    }

    CallData* temp = NULL;
    for(CallDataMap_T::iterator it = g_call_data_map.begin();
        it != g_call_data_map.end(); it++) {
        temp = it->second;
        delete temp;
        temp = NULL;
    }

    g_call_data_map.clear();

    return 1;
}

static void lua_prof_hook(lua_State* L, lua_Debug* ar)
{
    int ret = 0;
    CallData* call_info = NULL;
    struct timeval now;
    gettimeofday(&(now), NULL);
    uint64_t now_usec = 1000000 * (now.tv_sec) + now.tv_usec;
    ret = lua_getinfo(L, "Sn", ar);
    if (ret == 0)
        return;

    if (ar->name == NULL)
        return;

    CallDataMap_T::iterator it = g_call_data_map.find(ar->name);

    switch(ar->event) {
    case LUA_HOOKCALL:
        if (it == g_call_data_map.end()) {
            // 第一次被调用
            call_info = new CallData;
            call_info->start = now_usec;
            call_info->times = 1;
            call_info->usec = 0;
            strncpy(call_info->what, ar->what, 32);
            strncpy(call_info->namewhat, ar->namewhat, 32);
            g_call_data_map[ar->name] = call_info;
        } else {
            call_info = it->second;
            call_info->times++;
            call_info->start = now_usec;
        }
        break;
    case LUA_HOOKRET:
        if (it != g_call_data_map.end()) {
            call_info = it->second;
            call_info->usec += (now_usec - call_info->start);
        }
        break;
    }
}

}
