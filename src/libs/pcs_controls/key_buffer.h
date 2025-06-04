#pragma once

#include <vector>

#include <libs/core/controls.h>

struct ControlKeyBuffer {
public:
    ControlKeyBuffer();
    ~ControlKeyBuffer();

    void Reset();
    void AddKey(char* u8_str, int u8_size, bool bSystem);
    void AddKey(KeyDescr const& key);

    int32_t GetBufferLength()
    {
        return pcBuffer_.size();
    }

    // FIXME: Not good
    KeyDescr const* c_str()
    {
        return pcBuffer_.data();
    }

    std::vector<KeyDescr> pcBuffer_;
};
