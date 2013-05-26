#pragma once
#include <istream>
#include <vector>
#include <algorithm>
#include "EndOfCentralDirectoryBlock.h"
#include "ZipArchiveEntry.h"

class ZipArchive
{
  friend class ZipArchiveEntry;

  public:
    ZipArchive();
    ZipArchive(ZipArchive&& other);
    ZipArchive(std::istream* stream);
    ZipArchive(std::istream* stream, bool destroySimultaneously = false);
    ~ZipArchive();

    ZipArchive& operator = (ZipArchive&& other);

    ZipArchiveEntry* CreateEntry(const std::string& fileName);

    const std::string& GetComment() const;
    void SetComment(const std::string& comment);

    const ZipArchiveEntry* GetEntry(int index) const;
    ZipArchiveEntry* GetEntry(int index);
    const ZipArchiveEntry* GetEntry(const std::string& entryName) const;
    ZipArchiveEntry* GetEntry(const std::string& entryName);

    size_t GetEntriesCount() const;

    void RemoveEntry(const std::string& entryName);
    void RemoveEntry(int index);

    void WriteToStream(std::ostream& stream);

    const ZipArchive* operator -> () const { return this; }
    ZipArchive* operator -> () { return this; }

  private:
    enum class SeekDirection
    {
      Forward,
      Backward
    };

    ZipArchive(const ZipArchive&);
    ZipArchive& operator = (const ZipArchive& other);

    bool EnsureCentralDirectoryRead();
    bool ReadEndOfCentralDirectory();
    bool SeekToSignature(uint32_t signature, SeekDirection direction);

    std::istream* _zipStream;
    bool _destroySimultaneously;
    EndOfCentralDirectoryBlock _endOfCentralDirectoryBlock;
    std::vector<ZipArchiveEntry*> _entries;
};
