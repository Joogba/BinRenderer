#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace BinRenderer::Vulkan
{
    /**
     * @brief Vulkan Queue Family 인덱스
     *
     * Vulkan 디바이스 초기화 시 필요한 큐 패밀리 정보
     */
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value() &&
                presentFamily.has_value() &&
                computeFamily.has_value();
        }
    };

    /**
     * @brief Vulkan Swapchain 지원 정보
     *
     * 스왑체인 생성 시 필요한 표면 기능 정보
     */
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

} // namespace BinRenderer::Vulkan