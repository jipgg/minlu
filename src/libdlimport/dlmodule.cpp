#include "dlimport.hpp"
#include <format>
#include <common.hpp>
constexpr int unintialized{-1};
static int importfunction_sa{unintialized};
using dlimport::Dlmodule;

static int index(Lstate L) {
    Dlmodule* module = dlimport::lua_tomodule(L, 1);
    const Strview key = luaL_checkstring(L, 2);
    if (key == "path") {
        lua_pushstring(L, module->path.string().c_str());
        return 1;
    } else if (key == "name") {
        lua_pushstring(L, module->name.c_str());
        return 1;
    }
    luaL_argerrorL(L, 2, "index was null");
}

static int import_function(Lstate L) {
    const String proc_key = String("dlexport_") + luaL_checkstring(L, 2);
    auto opt = dlimport::find_proc_address(*dlimport::lua_tomodule(L, 1), proc_key);
    if (not opt) luaL_errorL(L, "lua_CFunction '%s' was not found ", proc_key.c_str());
    const auto fmt = std::format("dlimported:{}", proc_key);
    lua_pushcfunction(L, reinterpret_cast<lua_CFunction>(*opt), fmt.c_str());
    return 1;
}
static int namecall(Lstate L) {
    Dlmodule& module = *dlimport::lua_tomodule(L, 1);
    int atom;
    lua_namecallatom(L, &atom);
    if (atom == importfunction_sa) return import_function(L);
    dluau::error(L, "invalid namecall '{}'", atom);
}

void Dlmodule::init(Lstate L) {
    if (luaL_newmetatable(L, Dlmodule::tname)) {
        importfunction_sa = dluau_stringatom(L, "import");
        lua_setlightuserdataname(L, Dlmodule::tag, Dlmodule::tname);
        const Lreg meta[] = {
            {"__index", index},
            {"__namecall", namecall},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
    }
    lua_pop(L, 1);
}
