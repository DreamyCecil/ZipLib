#pragma once
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>

#include "ZipLocalFileHeader.h"
#include "ZipCentralDirectoryFileHeader.h"

#include "streams/infstream.h"
#include "streams/substream.h"
#include "EnumTools.h"

class ZipArchive;

class ZipArchiveEntry
{
  friend class ZipArchive;
  friend class ZipCrypto;

  public:
    /**
     * \brief Values that represent the method of compression.
     */
    enum class CompressionMethod : uint16_t
    {
      Deflate = 8,
      Stored = 0
    };

    /**
     * \brief Values that represent the level of the compression.
     */
    enum class CompressionLevel
    {
      Stored = 0,
      BestSpeed = 1,
      Default = 6,
      BestCompression = 9
    };

    /**
     * \brief Values that represent the way the zip entry will be compressed.
     */
    enum class CompressionMode
    {
      Immediate,
      Deferred
    };

    /**
     * \brief Values that represent the MS-DOS file attributes.
     */
    enum class Attributes : uint32_t
    {
      None = 0,
      ReadOnly = 1,
      Hidden = 2,
      System = 4,
      Directory = 16,
      Archive = 32,
      Device = 64,
      Normal = 128,
      Temporary = 256,
      SparseFile = 512,
      ReparsePoint = 1024,
      Compressed = 2048,
    };

    MARK_AS_TYPED_ENUMFLAGS_FRIEND(CompressionMethod);
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(CompressionLevel);
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(Attributes);
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(CompressionMode);

    /**
     * \brief Destructor.
     */
    ~ZipArchiveEntry();

    /**
     * \brief Gets full path of the entry.
     *
     * \return  The full name with the path.
     */
    const std::string& GetFullName() const;

    /**
     * \brief Sets full name with the path of the entry.
     *
     * \param fullName The full name with the path.
     */
    void SetFullName(const std::string& fullName);

    /**
     * \brief Gets only the file name of the entry (without path).
     *
     * \return  The file name.
     */
    std::string GetName() const;

    /**
     * \brief Sets only a file name of the entry.
     *        If the file is located within some folder, the path is kept.
     *
     * \param name  The name.
     */
    void SetName(const std::string& name);

    const std::string& GetComment() const;
    void SetComment(const std::string& comment);

    time_t GetLastWriteTime() const;
    void SetLastWriteTime(time_t modTime);

    Attributes GetAttributes() const;
    void SetAttributes(Attributes value);

    const std::string& GetPassword() const;
    void SetPassword(const std::string& password);

    uint32_t GetCrc32() const;
    size_t GetSize() const;
    size_t GetCompressedSize() const;

    bool CanExtract() const;
    bool IsDirectory() const;
    bool IsUsingDataDescriptor() const;

    void UseDataDescriptor(bool use = true);

    std::istream* GetRawStream();
    std::istream* GetDecompressionStream();

    bool IsRawStreamOpened() const;
    bool IsDecompressionStreamOpened() const;

    void CloseRawStream();
    void CloseDecompressionStream();

    bool SetCompressionStream(std::istream*     stream,
                              CompressionLevel  level  = CompressionLevel::Default,
                              CompressionMethod method = CompressionMethod::Deflate,
                              CompressionMode   mode   = CompressionMode::Deferred);

    void Remove();

  private:
    static const uint16_t DEFAULT_VERSION_MADEBY = 20;

    enum class BitFlag : uint16_t
    {
      None = 0,
      Encrypted = 1,
      DataDescriptor = 8,
      UnicodeFileName = 0x800
    };

    enum class ZipVersionNeeded : uint16_t
    {
      Default = 10,
      DeflateAndExplicitDirectory = 20,
      Zip64 = 45
    };

    MARK_AS_TYPED_ENUMFLAGS_FRIEND(BitFlag);
    MARK_AS_TYPED_ENUMFLAGS_FRIEND(ZipVersionNeeded);

    ZipArchiveEntry();
    ZipArchiveEntry(const ZipArchiveEntry&);
    ZipArchiveEntry& operator = (ZipArchiveEntry&);

    // static methods
    static ZipArchiveEntry* CreateNew(ZipArchive* zipArchive, const std::string& fullPath);
    static ZipArchiveEntry* CreateExisting(ZipArchive* zipArchive, ZipCentralDirectoryFileHeader& cd);

    static void TimestampToDateTime(time_t dateTime, uint16_t& date, uint16_t& time);
    static time_t DateTimeToTimestamp(uint16_t date, uint16_t time);

    static bool IsValidFilename(const std::string& fullPath);
    static std::string GetFilenameFromPath(const std::string& fullPath);
    static bool IsDirectoryPath(const std::string& fullPath);
    static void CopyStream(std::istream& input, std::ostream& output);

    // methods
    CompressionMethod GetCompressionMethod() const;
    void SetCompressionMethod(CompressionMethod value);

    BitFlag GetGeneralPurposeBitFlag() const;
    void SetGeneralPurposeBitFlag(BitFlag value, bool set = true);

    ZipVersionNeeded GetVersionToExtract() const;
    void SetVersionToExtract(ZipVersionNeeded value);

    uint16_t GetVersionMadeBy() const;
    void SetVersionMadeBy(uint16_t value);

    int32_t GetOffsetOfLocalHeader() const;
    void SetOffsetOfLocalHeader(int32_t value);

    bool HasCompressionStream() const;

    void FetchLocalFileHeader();
    void CheckFilenameCorrection();
    void FixVersionToExtractAtLeast(ZipVersionNeeded value);

    void SyncLFH_with_CDFH();
    void SyncCDFH_with_LFH();

    std::ios::pos_type GetOffsetOfCompressedData();
    std::ios::pos_type SeekToCompressedData();

    void SerializeLocalFileHeader(std::ostream& stream);
    void SerializeCentralDirectoryFileHeader(std::ostream& stream);

    void UnloadCompressionData();
    void InternalCompressStream(std::istream& inputStream, std::ostream& outputStream);

    // for encryption

    void FigureCrc32();
    uint8_t GetLastByteOfEncryptionHeader();

    //////////////////////////////////////////////////////////////////////////
    ZipArchive* _archive;

    std::istream* _rawStream;
    std::istream* _openedArchive;
    std::istream* _inflateStream;
    std::istream* _zipCryptoStream;

    // TODO: make as flags
    bool _originallyInArchive;
    bool _isNewOrChanged;
    bool _hasLocalFileHeader;

    ZipLocalFileHeader _localFileHeader;
    ZipCentralDirectoryFileHeader _centralDirectoryFileHeader;

    std::ios::pos_type _offsetOfCompressedData;
    std::ios::pos_type _offsetOfSerializedLocalFileHeader;

    std::string _password;

    // internal compression data
    std::istream* _compressionStream;
    CompressionMode _compressionMode;
    CompressionLevel _compressionLevel;
    std::stringstream _compressionImmediateBuffer; // needed for both reading and writing
};
