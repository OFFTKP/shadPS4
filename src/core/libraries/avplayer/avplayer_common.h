// SPDX-FileCopyrightText: Copyright 2024 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "avplayer.h"

#include "common/assert.h"
#include "common/logging/log.h"
#include "core/libraries/kernel/thread_management.h"

#include <optional>
#include <utility>
#include <queue>

#define AVPLAYER_IS_ERROR(x) ((x) < 0)

namespace Libraries::AvPlayer {

enum class AvState {
    Initial,
    AddingSource,
    Ready,
    Play,
    Stop,
    EndOfFile,
    Pause,
    C0x08,
    Jump,
    TrickMode,
    C0x0B,
    Buffering,
    Starting,
    Error,
};

enum class AvEventType {
    ChangeFlowState = 21,
    WarningId = 22,
    RevertState = 30,
    AddSource = 40,
    Error = 255,
};

union AvPlayerEventData {
    u32 num_frames; // 20
    AvState state;  // AvEventType::ChangeFlowState
    s32 error;      // AvEventType::WarningId
    u32 attempt;    // AvEventType::AddSource
};

struct AvPlayerEvent {
    AvEventType event;
    AvPlayerEventData payload;
};

template <class T>
class AvPlayerQueue {
public:
    size_t Size() {
        return m_size;
    }

    void Push(T&& value) {
        std::lock_guard guard(m_mutex);
        m_queue.emplace(std::forward<T>(value));
        ++m_size;
    }

    std::optional<T> Pop() {
        if (Size() == 0) {
            return std::nullopt;
        }
        std::lock_guard guard(m_mutex);
        if (Size() == 0) { // in case size changes while the mutex is being locked
            return std::nullopt;
        }
        auto result = std::move(m_queue.front());
        m_queue.pop();
        --m_size;
        return result;
    }

    void Clear() {
        std::lock_guard guard(m_mutex);
        m_queue = {};
        m_size = 0;
    }

private:
    std::mutex m_mutex{};
    std::atomic_size_t m_size{0};
    std::queue<T> m_queue{};
};

SceAvPlayerSourceType GetSourceType(std::string_view path);

} // namespace Libraries::AvPlayer
