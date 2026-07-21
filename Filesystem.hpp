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

        // Owns a raw ::File* freed in the destructor; copying would double-close it.
        // Declaring the copy operations deleted also suppresses the implicit moves,
        // so a File cannot be copied or moved (the app only uses scoped locals).
        File(const File&) = delete;
        File& operator=(const File&) = delete;

        bool open(const UFZ::Filesystem& store, const char* path, FS_AccessMode accessMode, FS_OpenMode openMode) noexcept;

        [[nodiscard]] bool isOpen() const noexcept;
        [[nodiscard]] bool isDirectory() const noexcept;

        size_t read(void* buffer, size_t bytesToRead) const noexcept;

        // Appends the file contents to buffer, growing it chunkSize elements at a time.
        // chunkSize is a count of T elements; each iteration reads up to that many
        // elements and stops on the first short read (end of file). Returns bytes read.
        template<typename T>
        size_t read(std::vector<T>& buffer, size_t chunkSize = 128) const noexcept
        {
            const size_t start = buffer.size();
            const size_t chunkBytes = chunkSize * sizeof(T);
            size_t elementsRead = 0;
            size_t bytes;

            do
            {
                buffer.resize(start + elementsRead + chunkSize);
                bytes = read(buffer.data() + start + elementsRead, chunkBytes);
                elementsRead += bytes / sizeof(T);
            } while (bytes == chunkBytes);

            buffer.resize(start + elementsRead);
            return elementsRead * sizeof(T);
        }

        size_t write(const void* buffer, size_t bytesToWrite) const noexcept;

        template<typename T>
        size_t write(const std::vector<T>& buffer) const noexcept
        {
            return write(buffer.data(), buffer.size() * sizeof(T));
        }

        [[nodiscard]] bool seek(uint32_t offset, bool bFromStart) const noexcept;
        [[nodiscard]] uint64_t tell() const noexcept;
        [[nodiscard]] bool truncate() const noexcept;
        [[nodiscard]] uint64_t size() const noexcept;

        [[nodiscard]] bool sync() const noexcept;
        [[nodiscard]] bool eof() const noexcept;

        static bool copyToFile(const File& source, const File& destination, size_t size) noexcept;

        void close() noexcept;

        ~File() noexcept;
    private:
        friend class Filesystem;
        friend class Directory;

        ::File* file = nullptr;
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
        [[nodiscard]] bool close() const noexcept;

        bool read(FileInfo* info, char* name, uint16_t nameLength) const noexcept;
        [[nodiscard]] bool rewind() const noexcept;
    private:
        friend class Filesystem;

        File* file{};
    };
}