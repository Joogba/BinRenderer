#pragma once
#include "RendererAPI.h"
#include <string>
#include <unordered_map>

namespace BinRenderer {

    struct PassResources {
        // Name -> real View Handle mapping
        std::unordered_map<std::string, RenderTargetViewHandle>   rtvs;
        std::unordered_map<std::string, ShaderResourceViewHandle> srvs;
        std::unordered_map<std::string, DepthStencilViewHandle>   dsvs;

        RenderTargetViewHandle   GetRTV(const char* name) const { return rtvs.at(name); }
        ShaderResourceViewHandle GetSRV(const char* name) const { return srvs.at(name); }
        DepthStencilViewHandle   GetDSV(const char* name) const { return dsvs.at(name); }
    };

} // namespace BinRenderer
