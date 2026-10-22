#pragma once
#include "entity.h"
#include "string_compare.hpp"
#include <stdexcept>
#include <string_view>
// Original Storm 2 scripts identify layers by names. Keep the modern engine's
// fixed indices so native modules and original scripts address the same layers.
inline layer_index_t LegacyLayerIndex(std::string_view name) {
    struct Entry {const char*name;layer_index_t index;};
    static constexpr Entry entries[]={
        {"execute",0},{"realize",1},{"sea_execute",2},{"sea_realize",3},
        {"iexecute",4},{"irealize",5},{"fader_execute",6},{"fader_realize",7},
        {"lighter_execute",8},{"lighter_realize",9},{"video_execute",10},{"video_realize",11},
        {"editor_realize",12},{"inf_realize",13},{"sound_debug_realize",14},
        {"sea_reflection",15},{"sea_reflection2",16},{"sea_sunroad",17},
        {"sun_trace",18},{"sails_trace",19},{"hull_trace",20},{"mast_island_trace",21},
        {"mast_ship_trace",22},{"ship_cannon_trace",23},{"fort_cannon_trace",24},
        {"island_trace",25},{"shadow",26},{"blood",27},{"rain_drops",28},{"system_messages",29}};
    for(auto entry:entries)if(storm::iEquals(name,std::string_view(entry.name)))return entry.index;
    throw std::runtime_error("Unknown legacy engine layer: "+std::string(name));
}
inline layer_index_t ScriptLayerIndex(DATA*value) {
    auto real=value->GetVarPointer();
    if(real->GetType()==VAR_STRING){const char*name=nullptr;real->Get(name);return LegacyLayerIndex(name?name:"");}
    int32_t index=0;if(!real->Get(index)||index<0||index>=32)throw std::runtime_error("Invalid engine layer index");
    return static_cast<layer_index_t>(index);
}
