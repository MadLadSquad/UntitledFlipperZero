#pragma once
#include "Common.hpp"
#include <vector>

namespace UFZ
{
    class Filesystem;

    class File
    {
    public:
        File() = default;
        explicit File(const Filesystem& store) noexcept;
        File(const UFZ::Filesystem& store, const char* path, FS_AccessMode accessMode, FS_OpenMode openMode) noexcept;
        bool open(const UFZ::Filesystem& store, const char* path, FS_AccessMode accessMode, FS_OpenMode openMode) noexcept;

        bool isOpen() noexcept;
        bool isDirectory() noexcept;

        size_t read(void* buffer, size_t bytesToRead) noexcept;

        template<typename T>
        size_t read(std::vector<T>& buffer, size_t chunkSize = 128) noexcept
        {
            size_t sz = 0;
            buffer.resize(buffer.size() + chunkSize);

            while (size_t a = read(buffer.data() + sz, chunkSize) != 0)
            {
                sz += a;
                buffer.resize(buffer.size() + chunkSize);
            }
            buffer.shrink_to_fit();
            return sz;
        }

        size_t write(void* buffer, size_t bytesToWrite) noexcept;

        template<typename T>
        size_t write(const std::vector<T>& buffer) noexcept
        {
            return write(buffer.data(), buffer.size());
        }

        bool seek(uint32_t offset, bool bFromStart) noexcept;
        uint64_t tell() noexcept;
        bool truncate() noexcept;
        uint64_t size() noexcept;

        bool sync() noexcept;
        bool eof() noexcept;

        static bool copyToFile(File& source, File& destination, size_t size) noexcept;

        void close() noexcept;

        ~File() noexcept;
    private:
        friend class Filesystem;
        friend class Directory;

        ::File* file = nullptr;
        //Filesystem storage{};
        Filesystem* storage = nullptr;

        void init() noexcept;
        void free() noexcept;
    };

    class Directory
    {
    public:
        Directory() = default;
        explicit Directory(File& f) noexcept;

        bool open(File& f, const char* path) noexcept;
        bool close() noexcept;

        bool read(FileInfo* info, char* name, uint16_t nameLength) noexcept;
        bool rewind() noexcept;
    private:
        friend class Filesystem;

        File* file{};
    };
}