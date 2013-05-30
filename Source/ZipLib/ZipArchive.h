#pragma once
#include <istream>
#include <vector>
#include <algorithm>
#include "EndOfCentralDirectoryBlock.h"
#include "ZipArchiveEntry.h"

/**
 * \brief Represents a package of compressed files in the zip archive format.
 */
class ZipArchive
{
  friend class ZipArchiveEntry;

  public:
    /**
     * \brief Default constructor.
     */
    ZipArchive();

    /**
     * \brief Move constructor.
     *
     * \param other The ZipArchive instance to move.
     */
    ZipArchive(ZipArchive&& other);

    /**
     * \brief Constructor.
     *
     * \param stream The input stream of the zip archive content. Must be seekable.
     */
    ZipArchive(std::istream* stream);

    /**
     * \brief Constructor. It optionally allows to simultaneously destroy and dealloc the input stream
     *        with the ZipArchive.
     *
     * \param stream                The input stream of the zip archive content. Must be seekable.
     * \param destroySimultaneously If true, it calls "delete stream" in the ZipArchive destructor.
     */
    ZipArchive(std::istream* stream, bool destroySimultaneously);

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
    ZipArchiveEntry* CreateEntry(const std::string& fileName);

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
     * \brief Gets a const pointer to the zip entry located on the given index.
     *
     * \param index Zero-based index of the zip entry.
     *
     * \return  null if it fails, else the entry.
     */
    const ZipArchiveEntry* GetEntry(int index) const;

    /**
     * \brief Gets a const pointer to the zip entry with given file name.
     *
     * \param entryName Name of the entry.
     *
     * \return  null if it fails, else the entry.
     */
    const ZipArchiveEntry* GetEntry(const std::string& entryName) const;

    /**
     * \brief Gets a pointer to the zip entry located on the given index.
     *
     * \param index Zero-based index of the.
     *
     * \return  null if it fails, else the entry.
     */
    ZipArchiveEntry* GetEntry(int index);

    /**
     * \brief Gets a const pointer to the zip entry with given file name.
     *
     * \param entryName Name of the entry.
     *
     * \return  null if it fails, else the entry.
     */
    ZipArchiveEntry* GetEntry(const std::string& entryName);

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

    void Swap(ZipArchive& other);

    const ZipArchive* operator -> () const { return this; }
    ZipArchive* operator -> () { return this; }

  private:
    enum class SeekDirection
    {
      Forward,
      Backward
    };

    // used in ZipArchiveEntry::CopyStream method for effective allocation
    struct InternalSharedBuffer 
    {
      static InternalSharedBuffer* GetInstance();

      size_t GetBufferSize();
      char* GetBuffer();
      void IncRef();
      void DecRef();

      private:
        enum
        {
          INTERNAL_BUFFER_SIZE = 1024 * 1024
        };

        char* _buffer;
        size_t   _refCount;
    };

    ZipArchive(const ZipArchive&);
    ZipArchive& operator = (const ZipArchive& other);

    bool EnsureCentralDirectoryRead();
    bool ReadEndOfCentralDirectory();
    bool SeekToSignature(uint32_t signature, SeekDirection direction);

    EndOfCentralDirectoryBlock _endOfCentralDirectoryBlock;
    std::vector<ZipArchiveEntry*> _entries;
    std::istream* _zipStream;
    bool _destroySimultaneously;
};
