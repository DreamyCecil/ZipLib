#pragma once

#define GENERAL_PASSWORD "abcd"

#define GetRandomCompressionMethod()  (ZipCompressionMethod)(rand()%4)
#define GetRandomEncryptionMethod()   (ZipEncryptionMethod)(rand()%5)
#define GetRandomCompressionMode()    (ZipArchiveEntry::CompressionMode)(rand()%2)

#define FromZipCompressionMetod(cm)   cm == ZipCompressionMethod::Store   ? (CompressionMethod::Ptr)StoreMethod::Create()   :\
                                      cm == ZipCompressionMethod::Deflate ? (CompressionMethod::Ptr)DeflateMethod::Create() :\
                                      cm == ZipCompressionMethod::Bzip2   ? (CompressionMethod::Ptr)Bzip2Method::Create()   :\
                                      cm == ZipCompressionMethod::Lzma    ? (CompressionMethod::Ptr)LzmaMethod::Create()    :\
                                                                             CompressionMethod::Ptr()

#define FromZipEncryptionMethod(em)   em == ZipEncryptionMethod::ZipCrypto ? (EncryptionMethod::Ptr)ZipCryptoMethod::Create(GENERAL_PASSWORD) :                                                            \
                                      em == ZipEncryptionMethod::Aes128    ? (EncryptionMethod::Ptr)AesCryptoMethod::CreateWithPasswordAndMode(GENERAL_PASSWORD, AesCryptoMethod::EncryptionMode::AES128) :\
                                      em == ZipEncryptionMethod::Aes192    ? (EncryptionMethod::Ptr)AesCryptoMethod::CreateWithPasswordAndMode(GENERAL_PASSWORD, AesCryptoMethod::EncryptionMode::AES192) :\
                                      em == ZipEncryptionMethod::Aes256    ? (EncryptionMethod::Ptr)AesCryptoMethod::CreateWithPasswordAndMode(GENERAL_PASSWORD, AesCryptoMethod::EncryptionMode::AES256) :\
                                                                              EncryptionMethod::Ptr()

#define IsAesEncryption(em)           (em == ZipEncryptionMethod::Aes128 || em == ZipEncryptionMethod::Aes192 || em == ZipEncryptionMethod::Aes256)

//////////////////////////////////////////////////////////////////////////

#if !ZIPLIB_VS_TEST

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "Catch/catch.h"

#define ZIPLIB_TEST_GROUP_BEGIN(name)
#define ZIPLIB_TEST_GROUP_END()

#define ZIPLIB_TEST_CASE_BEGIN(name)                              \
  TEST_CASE(#name)

#define ZIPLIB_TEST_CASE_END()

#define ZIPLIB_TEST_IS_TRUE(expr)                                 \
  REQUIRE(expr)

#define ZIPLIB_TEST_IS_FALSE(expr)                                \
  REQUIRE_FALSE(expr)

#else

#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define ZIPLIB_TEST_GROUP_BEGIN(name)                             \
  TEST_CLASS(name)                                                \
  {                                                               \
    public:                                                       

#define ZIPLIB_TEST_GROUP_END()                                   \
  };

#define ZIPLIB_TEST_CASE_BEGIN(name)                              \
      TEST_METHOD(name)                                           \
      {                                                           \
        SetCurrentDirectory(TEXT("..\\..\\Playground\\Temp"));

#define ZIPLIB_TEST_CASE_END()                                    \
      }

#define ZIPLIB_TEST_IS_TRUE(expr)                                 \
  Assert::IsTrue(expr)

#define ZIPLIB_TEST_IS_FALSE(expr)                                \
  Assert::IsFalse(expr)

#endif
