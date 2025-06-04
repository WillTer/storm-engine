// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <tuple>

#include <spdlog/common.h>

namespace storm::logging::details
{
// Helper class for file sinks.
// When failing to open a file, retry several times(5) with a delay interval(10 ms).
// Throw spdlog_ex exception on errors.

class SPDLOG_API file_helper
{
public:
    explicit file_helper() = default;

    file_helper(file_helper const&)            = delete;
    file_helper& operator=(file_helper const&) = delete;
    ~file_helper();

    void                      open(spdlog::filename_t const& fname, bool truncate = false);
    void                      reopen(bool truncate);
    void                      flush();
    void                      close();
    void                      write(spdlog::memory_buf_t const& buf);
    size_t                    size() const;
    spdlog::filename_t const& filename() const;

    std::FILE* getfd() const;

    //
    // return file path and its extension:
    //
    // "mylog.txt" => ("mylog", ".txt")
    // "mylog" => ("mylog", "")
    // "mylog." => ("mylog.", "")
    // "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
    //
    // the starting dot in filenames is ignored (hidden files):
    //
    // ".mylog" => (".mylog". "")
    // "my_folder/.mylog" => ("my_folder/.mylog", "")
    // "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
    static std::tuple<spdlog::filename_t, spdlog::filename_t> split_by_extension(spdlog::filename_t const& fname);

private:
    int const          open_tries_    = 5;
    int const          open_interval_ = 10;
    std::FILE*         fd_ {nullptr};
    spdlog::filename_t filename_;
};
}  // namespace storm::logging::details
