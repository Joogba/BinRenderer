#include "Vulkan/Context.h"

using namespace BinRenderer::Vulkan;
using namespace std;

int main()
{
    vector<const char*> requiredInstanceExtensions = {};
    bool useSwapchain = false;

    Context ctx(requiredInstanceExtensions, useSwapchain);

    return 0;
}
