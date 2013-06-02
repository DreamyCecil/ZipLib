#pragma once
#include <istream>
#include <vector>
#include <algorithm>
#include <atomic>
#include <memory>
#include "EndOfCentralDirectoryBlock.h"
#include "ZipArchiveEntry.h"
#include "utils/threadsafe_holder.h"

/**
 * \brief Represents a package of compressed files in the zip archive format.
 */
class ZipArchive
{
  friend class ZipFile;
  friend class ZipArchiveEntry;

  public:
    typedef std::shared_ptr<ZipArchive> Ptr;

    /**
     * \brief Default constructor.
     */
    static ZipArchive::Ptr Create();

    /**
     * \brief Move constructor.
     *
     * \param other The ZipArchive instance to move.
     */
    static ZipArchive::Ptr Create(ZipArchive::Ptr&& other);
    
    /**
     * \brief Constructor.
     *
     * \param stream The input stream of the zip archive content. Must be seekable.
     */
    static ZipArchive::Ptr Create(std::istream& stream);

    /**
     * \brief Constructor. It optionally allows to simultaneously destroy and dealloc the input stream
     *        with the ZipArchive.
     *
     * \param stream                The input stream of the zip archive content. Must be seekable.
     * \param destroySimultaneously If true, it calls "delete stream" in the ZipArchive destructor.
     */
    static ZipArchive::Ptr Create(std::istream* stream, bool destroySimultaneously);

    /**
     * \brief Destructor.
     */
    ~ZipArchive();

    /**
     * \brief Move assignment operator.
     *
     * \param other The ZipArchive instance to move.
     *
     * \return  A shallow copy of this object.
     */
    ZipArchive& operator = (ZipArchive&& other);

    /**
     * \brief Creates an zip entry with given file name.
     *
     * \param fileName  Filename of the file.
     *
     * \return  nullptr if it fails, else the new entry.
     */
    ZipArchiveEntry::Ptr CreateEntry(const std::string& fileName);

    /**
     * \brief Gets the comment of the zip archive.
     *
     * \return  The comment.
     */
    const std::string& GetComment() const;

    /**
     * \brief Sets a comment of the zip archive.
     *
     * \param comment The comment.
     */
    void SetComment(const std::string& comment);

    /**
     * \brief Gets a pointer to the zip entry located on the given index.
     *
     * \param index Zero-based index of the.
     *
     * \return  null if it fails, else the entry.
     */
    ZipArchiveEntry::Ptr GetEntry(int index);

    /**
     * \brief Gets a const pointer to the zip entry with given file name.
     *
     * \param entryName Name of the entry.
     *
     * \return  null if it fails, else the entry.
     */
    ZipArchiveEntry::Ptr GetEntry(const std::string& entryName);

    /**
     * \brief Gets the number of the zip entries in this archive.
     *
     * \return  The number of the zip entries in this archive.
     */
    size_t GetEntriesCount() const;

    /**
     * \brief Removes the entry by the file name.
     *
     * \param entryName Name of the entry.
     */
    void RemoveEntry(const std::string& entryName);

    /**
    * \brief Removes the entry by the index.
     *
     * \param index Zero-based index of the.
     */
    void RemoveEntry(int index);

    /**
     * \brief Writes the zip archive content to the stream. It must be seekable.
     *
     * \param stream The stream to write in.
     */
    void WriteToStream(std::ostream& stream);

    /**
     * \brief Swaps this instance of ZipArchive with another instance.
     *
     * \param other The instance to swap with.
     */
    void Swap(ZipArchive::Ptr other);

  private:
    ZipArchive();
    ZipArchive(const ZipArchive&);
    ZipArchive& operator = (const ZipArchive& other);

    enum class SeekDirection
    {
      Forward,
      Backward
    };

    // used in ZipArchiveEntry::CopyStream method for effective allocation
    class InternalSharedBuffer
      : public enable_threadsafe_from_this<InternalSharedBuffer>
    {
      public:
        static InternalSharedBuffer* GetInstance();

        char* GetBuffer();
        size_t GetBufferSize();
        void IncRef();
        void DecRef();

      private:
        enum
        {
          INTERNAL_BUFFER_SIZE = 1024 * 1024
        };

        // counter is independent of the buffer
        std::atomic<size_t> _refCount;
        char* _buffer;
    };

    bool EnsureCentralDirectoryRead();
    bool ReadEndOfCentralDirectory();
    bool SeekToSignature(uint32_t signature, SeekDirection direction);

    void InternalDestroy();

    EndOfCentralDirectoryBlock _endOfCentralDirectoryBlock;
    std::vector<std::shared_ptr<ZipArchiveEntry>> _entries;
    std::istream* _zipStream;
    bool _destroySimultaneously;
};
