#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <optional>

namespace STB
{
enum class Widget : std::size_t { Resist, Level, Equip, Shout, Gold, PlayTime, GameTime, Weight, Count };
constexpr auto WidgetCount = static_cast<std::size_t>(Widget::Count);

struct AlphaRequest
{
    float value;
    std::optional<float> minimumCurrent;
};

struct HudFrame
{
    std::uint64_t generation;
    std::uint64_t configuration;
    bool ready;
    bool visible;
};

// Producers never own a menu, movie, GFxValue, or queued closure. At most one
// alpha request per widget survives until that widget's next UI frame.
class HudRequests
{
public:
    void BeginSaveLoad()
    {
        const std::scoped_lock lock(mutex_);
        Invalidate();
        saveLoading_ = true;
        worldReady_ = false;
    }

    void FinishSaveLoad(bool success)
    {
        const std::scoped_lock lock(mutex_);
        saveLoading_ = false;
        worldReady_ = success;
        ++configuration_;
    }

    void NewGame()
    {
        const std::scoped_lock lock(mutex_);
        Invalidate();
        saveLoading_ = false;
        worldReady_ = true;
    }

    void LoadingMenu(bool opening)
    {
        const std::scoped_lock lock(mutex_);
        if (opening && !menuLoading_) {
            Invalidate();
        }
        menuLoading_ = opening;
    }

    void Configure()
    {
        const std::scoped_lock lock(mutex_);
        ++configuration_;
    }

    void Visibility(bool visible)
    {
        const std::scoped_lock lock(mutex_);
        visible_ = visible;
    }

    void Alpha(Widget widget, AlphaRequest request)
    {
        const std::scoped_lock lock(mutex_);
        if (Ready()) {
            alpha_[static_cast<std::size_t>(widget)] = request;
        }
    }

    [[nodiscard]] HudFrame Frame() const
    {
        const std::scoped_lock lock(mutex_);
        return { generation_, configuration_, Ready(), visible_ };
    }

    [[nodiscard]] std::optional<AlphaRequest> TakeAlpha(Widget widget, std::uint64_t generation)
    {
        const std::scoped_lock lock(mutex_);
        if (!Ready() || generation != generation_) {
            return std::nullopt;
        }
        auto& slot = alpha_[static_cast<std::size_t>(widget)];
        const auto result = slot;
        slot.reset();
        return result;
    }

private:
    [[nodiscard]] bool Ready() const { return worldReady_ && !saveLoading_ && !menuLoading_; }

    void Invalidate()
    {
        ++generation_;
        ++configuration_;
        visible_ = true;
        alpha_.fill(std::nullopt);
    }

    mutable std::mutex mutex_;
    std::uint64_t generation_ = 1;
    std::uint64_t configuration_ = 1;
    bool worldReady_ = false;
    bool saveLoading_ = false;
    bool menuLoading_ = false;
    bool visible_ = true;
    std::array<std::optional<AlphaRequest>, WidgetCount> alpha_{};
};

// Owned by one menu instance, accessed only from its AdvanceMovie callback.
struct WidgetFrameState
{
    std::uint64_t configuration = 0;
    float elapsed = 100.0f;
};
}
