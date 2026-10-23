#pragma once
#include "entity.h"
#include "string_compare.hpp"
#include <cstdio>
#include <stdexcept>
#include <string>
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

    // An unknown name does NOT throw.
    //
    // This is called from the script VM, in the middle of a frame: the
    // game's own startup does
    //
    //   PROGRAM/seadogs.c(675)  LayerCreate("realize", 1);
    //
    // and the layer functions are called from many places besides. A
    // throw here unwinds through the interpreter, and a script can
    // produce a name this table does not know - a name built at run
    // time, or one from another Storm title. Crashing the engine
    // because a script asked for a layer by a name we did not list is
    // not acceptable behaviour, so an unknown name is reported and
    // given a real layer to use.
    //
    // Layers are addressed by index, and the engine's array has 32 of
    // them, so the last few are the ones to hand out: 30 and 31 are
    // not named in the table above.
    static layer_index_t nextUnnamed = 30;
    std::fprintf(stderr,
                 "engine layers: unknown layer name '%s', using layer %u\n",
                 std::string(name).c_str(), static_cast<unsigned>(nextUnnamed));
    const auto assigned = nextUnnamed;
    if (nextUnnamed < 31)
        ++nextUnnamed;
    return assigned;
}
inline layer_index_t ScriptLayerIndex(DATA*value) {
    if (!value)
    {
        std::fprintf(stderr, "engine layers: the layer argument is null, using layer 0\n");
        return 0;
    }
    auto real=value->GetVarPointer();

    // GetVarPointer returns null for a reference with no target - an
    // uninitialised ref, or one whose variable was unloaded with its
    // segment. Dereferencing that here is what crashed the engine, so
    // it is checked.
    if (!real)
    {
        std::fprintf(stderr,
                     "engine layers: the layer argument is an uninitialised reference, using layer 0\n");
        return 0;
    }

    if(real->GetType()==VAR_STRING){const char*name=nullptr;real->Get(name);return LegacyLayerIndex(name?name:"");}
    int32_t index=0;
    if(!real->Get(index)||index<0||index>=32)
    {
        std::fprintf(stderr, "engine layers: invalid layer index, using layer 0\n");
        return 0;
    }
    return static_cast<layer_index_t>(index);
}
