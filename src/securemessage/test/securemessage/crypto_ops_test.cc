/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/securemessage/include/securemessage/crypto_ops.h"

#include <gtest/gtest.h>
#include <gtest/gtest-message.h>
#include <gtest/internal/gtest-internal.h>
#include <stddef.h>

using std::unique_ptr;

namespace securemessage {

//
//  Class for exposing CryptOps private helper functions for testing purposes
//
class CryptoOpsTest {
 public:
  static const uint8_t* Salt() { return CryptoOps::kSalt; }

  static unsigned int SaltSize() { return CryptoOps::kSaltSize; }

  static unsigned int AesKeySize() { return CryptoOps::kAesKeySize; }

  static unique_ptr<ByteBuffer> Sha256(ByteBuffer message) {
    return CryptoOps::Sha256(message);
  }

  static unique_ptr<ByteBuffer> HkdfSha256Extract(ByteBuffer key,
                                                  ByteBuffer salt) {
    return CryptoOps::HkdfSha256Extract(key, salt);
  }

  static unique_ptr<ByteBuffer> Aes256CBCDecrypt(
      CryptoOps::SecretKey decryptionKey, ByteBuffer iv,
      ByteBuffer ciphertext) {
    return CryptoOps::Aes256CBCDecrypt(decryptionKey, iv, ciphertext);
  }

  static unique_ptr<ByteBuffer> Aes256CBCEncrypt(
      CryptoOps::SecretKey encryptionKey, ByteBuffer iv, ByteBuffer plaintext) {
    return CryptoOps::Aes256CBCEncrypt(encryptionKey, iv, plaintext);
  }

  static std::string GetPurpose(CryptoOps::SigType sigType) {
    return CryptoOps::GetPurpose(sigType);
  }

  static std::string GetPurpose(CryptoOps::EncType encType) {
    return CryptoOps::GetPurpose(encType);
  }

  static std::string Int32BytesToString(int32_t value) {
    return CryptoOps::Int32BytesToString(value);
  }

  static bool StringToInt32Bytes(const std::string& value, int32_t* result) {
    return CryptoOps::StringToInt32Bytes(value, result);
  }
};
}  // namespace securemessage

namespace securemessage {
namespace unittesting {
namespace {

// HKDF Test Case 1 IKM from RFC 5869
static unsigned const int kHkdfCase1IkmLength = 22;
static unsigned const char kHkdfCase1Ikm[kHkdfCase1IkmLength] = {
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};

// HKDF Test Case 1 salt from RFC 5869
static unsigned const int kHkdfCase1SaltLength = 13;
static unsigned const char kHkdfCase1Salt[kHkdfCase1SaltLength] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};

// HKDF Test Case 1 info from RFC 5869
static unsigned const int kHkdfCase1InfoLength = 10;
static unsigned const char kHkdfCase1Info[kHkdfCase1InfoLength] = {
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9};

// First 32 bytes of HKDF Test Case 1 OKM (output) from RFC 5869
static unsigned const int kHkdfCase1OkmLength = 32;
static unsigned const char kHkdfCase1Okm[kHkdfCase1OkmLength] = {
    0x3c, 0xb2, 0x5f, 0x25, 0xfa, 0xac, 0xd5, 0x7a, 0x90, 0x43, 0x4f,
    0x64, 0xd0, 0x36, 0x2f, 0x2a, 0x2d, 0x2d, 0x0a, 0x90, 0xcf, 0x1a,
    0x5a, 0x4c, 0x5d, 0xb0, 0x2d, 0x56, 0xec, 0xc4, 0xc5, 0xbf};

// Expected results (based on Java SecureMessage codebase)
static unsigned const int kHkdfExtractExpectedLength = 32;
static unsigned const char kHkdfExtractExpected[kHkdfExtractExpectedLength] = {
    0x07, 0x77, 0x09, 0x36, 0x2c, 0x2e, 0x32, 0xdf, 0x0d, 0xdc, 0x3f,
    0x0d, 0xc4, 0x7b, 0xba, 0x63, 0x90, 0xb6, 0xc7, 0x3b, 0xb5, 0x0f,
    0x9c, 0x31, 0x22, 0xec, 0x84, 0x4a, 0xd7, 0xc2, 0xb3, 0xe5};

// AES Test Vectors
static unsigned const char kAesCiphertext[32] = {
    0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab,
    0xfb, 0x5f, 0x7b, 0xfb, 0xd6, 0x48, 0x5a, 0x5c, 0x81, 0x51, 0x9c,
    0xf3, 0x78, 0xfa, 0x36, 0xd4, 0x2b, 0x85, 0x47, 0xed, 0xc0};

static unsigned const char kAesPlaintext[16] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};

static unsigned const int kAesIvLength = 16;
static unsigned const char KAesIv[kAesIvLength] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

static unsigned const char kAesKey[32] = {
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae,
    0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61,
    0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};

TEST(CryptoOpsTest, TestSalt) {
  const uint8_t* salt = CryptoOpsTest::Salt();
  unique_ptr<ByteBuffer> expected_salt =
      CryptoOpsTest::Sha256(ByteBuffer("SecureMessage"));

  EXPECT_EQ(expected_salt->size(), CryptoOpsTest::SaltSize());
  EXPECT_TRUE(expected_salt->Equals(ByteBuffer(salt, CryptoOpsTest::SaltSize())));
}

TEST(CryptoOpsTest, TestHkdfExtract) {
  EXPECT_EQ(kHkdfCase1IkmLength, sizeof(kHkdfCase1Ikm));
  EXPECT_EQ(kHkdfCase1SaltLength, sizeof(kHkdfCase1Salt));

  ByteBuffer key_data(kHkdfCase1Ikm, kHkdfCase1IkmLength);
  ByteBuffer salt(kHkdfCase1Salt, kHkdfCase1SaltLength);

  unique_ptr<ByteBuffer> result =
      CryptoOpsTest::HkdfSha256Extract(key_data, salt);

  EXPECT_NE(nullptr, result);
  EXPECT_EQ(kHkdfExtractExpectedLength, result->size());
  EXPECT_TRUE(result->Equals(
      ByteBuffer(kHkdfExtractExpected, kHkdfExtractExpectedLength)));
}

TEST(CryptoOpsTest, TestHkdf) {
  EXPECT_EQ(kHkdfCase1IkmLength, sizeof(kHkdfCase1Ikm));
  EXPECT_EQ(kHkdfCase1SaltLength, sizeof(kHkdfCase1Salt));
  EXPECT_EQ(kHkdfCase1InfoLength, sizeof(kHkdfCase1Info));

  ByteBuffer key_data(kHkdfCase1Ikm, kHkdfCase1IkmLength);
  ByteBuffer salt(kHkdfCase1Salt, kHkdfCase1SaltLength);
  ByteBuffer info(kHkdfCase1Info, kHkdfCase1InfoLength);

  unique_ptr<std::string> result =
      CryptoOps::Hkdf(key_data.String(), salt.String(), info.String());

  EXPECT_NE(nullptr, result);
  EXPECT_EQ(kHkdfCase1OkmLength, result->length());
  EXPECT_TRUE(ByteBuffer(kHkdfCase1Okm, kHkdfCase1OkmLength)
                  .Equals(ByteBuffer(*result)));
}

TEST(CryptoOpsTest, TestDeriveAes256KeyFor) {
  CryptoOps::SecretKey aesKey1(
      std::string(CryptoOpsTest::AesKeySize(), static_cast<char>(1)),
      CryptoOps::AES_256_KEY);
  CryptoOps::SecretKey aesKey2(
      std::string(CryptoOpsTest::AesKeySize(), static_cast<char>(2)),
      CryptoOps::AES_256_KEY);

  // Test that deriving with the same key and purpose twice is deterministic
  EXPECT_EQ(CryptoOps::DeriveAes256KeyFor(aesKey1, "A")->data().String(),
            CryptoOps::DeriveAes256KeyFor(aesKey1, "A")->data().String());

  // Test that derived keys with different purposes differ
  EXPECT_NE(CryptoOps::DeriveAes256KeyFor(aesKey1, "A")->data().String(),
            CryptoOps::DeriveAes256KeyFor(aesKey1, "B")->data().String());

  // Test that derived keys with the same purpose but different master keys
  // differ
  EXPECT_NE(CryptoOps::DeriveAes256KeyFor(aesKey1, "A")->data().String(),
            CryptoOps::DeriveAes256KeyFor(aesKey2, "A")->data().String());
}

TEST(CryptoOpsTest, TestAes256CbcEncryptDecrypt) {
  std::string plaintext(16, static_cast<char>(1));
  std::string iv(16, static_cast<char>(2));
  std::string key(32, static_cast<char>(3));
  CryptoOps::SecretKey aesKey(key, CryptoOps::AES_256_KEY);

  unique_ptr<ByteBuffer> ciphertext_ptr = CryptoOpsTest::Aes256CBCEncrypt(
      aesKey, ByteBuffer(iv), ByteBuffer(plaintext));
  EXPECT_NE(nullptr, ciphertext_ptr);
  EXPECT_NE(ciphertext_ptr->String(), plaintext);

  unique_ptr<ByteBuffer> plaintext_ptr =
      CryptoOpsTest::Aes256CBCDecrypt(aesKey, ByteBuffer(iv), *ciphertext_ptr);
  EXPECT_NE(nullptr, plaintext_ptr);
  EXPECT_EQ(plaintext, plaintext_ptr->String());
}

TEST(CryptoOpsTest, TestAes256CbcEncryptDecrypt2) {
  // First test the encrypt portion using known vectors
  ByteBuffer plaintext_string(kAesPlaintext, sizeof(kAesPlaintext));
  ByteBuffer iv_string(KAesIv, sizeof(KAesIv));
  ByteBuffer key_string(kAesKey, sizeof(kAesKey));
  CryptoOps::SecretKey aesKey(key_string.String(), CryptoOps::AES_256_KEY);

  unique_ptr<ByteBuffer> ciphertext_string =
      CryptoOpsTest::Aes256CBCEncrypt(aesKey, iv_string, plaintext_string);

  EXPECT_NE(nullptr, ciphertext_string);
  EXPECT_TRUE(ByteBuffer(kAesCiphertext, sizeof(kAesCiphertext))
                  .Equals(*ciphertext_string));

  // Now test the decrypt portion
  unique_ptr<ByteBuffer> plaintext_ptr =
      CryptoOpsTest::Aes256CBCDecrypt(aesKey, iv_string, *ciphertext_string);

  EXPECT_NE(nullptr, plaintext_ptr);
  EXPECT_TRUE(
      plaintext_ptr->Equals(ByteBuffer(kAesPlaintext, sizeof(kAesPlaintext))));
}

TEST(CryptoOpsTest, TestEncryptDecrypt) {
  ByteBuffer plaintext("Hello World!");
  ByteBuffer key("5uper 5ecret");

  // Create a bogus iv with 16 bytes of the number 42.
  ByteBuffer iv(std::string(16, static_cast<char>(42)));

  CryptoOps::SecretKey aesKey(key.String(), CryptoOps::AES_256_KEY);
  unique_ptr<std::string> ciphertext_ptr = CryptoOps::Encrypt(
      aesKey, CryptoOps::AES_256_CBC, iv.String(), plaintext.String());

  EXPECT_NE(nullptr, ciphertext_ptr);
  EXPECT_NE(plaintext.String(), *ciphertext_ptr);

  iv = ByteBuffer(std::string(16, 42));
  unique_ptr<std::string> plaintext_ptr = CryptoOps::Decrypt(
      aesKey, CryptoOps::AES_256_CBC, iv.String(), *ciphertext_ptr);
  EXPECT_NE(nullptr, plaintext_ptr);
  EXPECT_EQ(plaintext.String(), *plaintext_ptr);
}

TEST(CryptoOpsTest, TestEncryptDecryptEmpty) {
  ByteBuffer plaintext;
  ByteBuffer key("5uper 5ecret");
  ByteBuffer iv(std::string(kAesIvLength, static_cast<char>(42)));

  CryptoOps::SecretKey aesKey(key.String(), CryptoOps::AES_256_KEY);
  unique_ptr<std::string> ciphertext_ptr = CryptoOps::Encrypt(
      aesKey, CryptoOps::AES_256_CBC, iv.String(), plaintext.String());

  EXPECT_NE(nullptr, ciphertext_ptr);
  EXPECT_NE(plaintext.String(), *ciphertext_ptr);

  iv = ByteBuffer(std::string(kAesIvLength, static_cast<char>(42)));
  unique_ptr<std::string> plaintext_ptr = CryptoOps::Decrypt(
      aesKey, CryptoOps::AES_256_CBC, iv.String(), *ciphertext_ptr);
  EXPECT_NE(nullptr, plaintext_ptr);
  EXPECT_EQ((size_t)0, plaintext_ptr->size());
}

TEST(CryptoOpsTest, TestGenerateIV) {
  unique_ptr<std::string> iv1 = CryptoOps::GenerateIv(CryptoOps::AES_256_CBC);
  unique_ptr<std::string> iv2 = CryptoOps::GenerateIv(CryptoOps::AES_256_CBC);
  unique_ptr<std::string> iv3 = CryptoOps::GenerateIv(CryptoOps::NONE);

  EXPECT_NE(nullptr, iv1);
  EXPECT_NE(nullptr, iv2);
  EXPECT_EQ(nullptr, iv3);

  EXPECT_EQ(kAesIvLength, iv1->length());
  EXPECT_EQ(kAesIvLength, iv2->length());

  EXPECT_NE(0, iv1->compare(*iv2));
}

TEST(CryptoOpsTest, TestNoPurposeConflicts) {
  // Ensure that signature algorithms and encryption algorithms are not given
  // identical purposes (this prevents confusion of derived keys).
  for (int sig_type = 0; sig_type < CryptoOps::SigType::SIG_TYPE_END;
       sig_type++) {
    for (int enc_type = 0; enc_type < CryptoOps::EncType::ENC_TYPE_END;
         enc_type++) {
      EXPECT_FALSE(CryptoOpsTest::GetPurpose((CryptoOps::SigType)sig_type) ==
                   CryptoOpsTest::GetPurpose((CryptoOps::EncType)enc_type));
    }
  }
}

TEST(CryptoOpsTest, TestSignVerifyHmacSha256) {
  // Test Case 1 from RFC4231
  // Note that we can't just use the standard expected signature since our
  // sign() function internally derives a key from the original key.  So
  // instead, we're testing the output against a known good output from the
  // Java implementation of securemessage.
  std::string key_string(20, static_cast<char>(0x0b));
  std::string data_string("Hi There");
  unsigned char expected_signature[32] = {
      0x3b, 0x14, 0x7b, 0x0f, 0xe6, 0x6a, 0x00, 0x47, 0xa2, 0x60, 0x4c,
      0xf2, 0x64, 0x29, 0xad, 0x07, 0x5d, 0x86, 0x8b, 0x01, 0xdb, 0x11,
      0xef, 0x6f, 0x4e, 0xc2, 0x2d, 0x8b, 0xdb, 0x66, 0xf1, 0x8c};
  CryptoOps::SecretKey secret_key(key_string,
                                  CryptoOps::KeyAlgorithm::AES_256_KEY);
  unique_ptr<std::string> signature =
      CryptoOps::Sign(CryptoOps::SigType::HMAC_SHA256, secret_key, data_string);
  EXPECT_NE(nullptr, signature);
  EXPECT_EQ((size_t)32, signature->length());
  EXPECT_TRUE(
      ByteBuffer(expected_signature, 32).Equals(ByteBuffer(*signature)));

  EXPECT_TRUE(CryptoOps::Verify(CryptoOps::SigType::HMAC_SHA256, secret_key,
                                *signature, data_string));
}

TEST(CryptoOpsTest, TestSignVerifyEcdsaP256sha256) {
  // We don't really test anything other than a signature is generated
  // and verified with no errors
  // We use sample pregenerated keys.  Below are their byte representations in
  // PKCS8 format
  unsigned const char public_key_bytes[335] = {
      0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
      0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
      0x42, 0x00, 0x04, 0x7f, 0x02, 0xe5, 0xd3, 0x30, 0x53, 0xff, 0x00, 0x82,
      0xf0, 0xa5, 0x5b, 0x3b, 0x61, 0xa5, 0x2e, 0x5a, 0x18, 0xd9, 0x5c, 0x51,
      0xa6, 0x7d, 0x07, 0x2d, 0x68, 0x8e, 0xd9, 0xfc, 0x6c, 0x16, 0xb7, 0x75,
      0xa6, 0xc7, 0xf6, 0x18, 0x79, 0xfa, 0xda, 0x9a, 0x31, 0x6c, 0x28, 0x7d,
      0xdc, 0x53, 0xfe, 0xad, 0x6d, 0x69, 0xaa, 0x34, 0xff, 0x17, 0x69, 0x0a,
      0xb0, 0xa3, 0xf2, 0x1b, 0x33, 0xee, 0xfb};

  unsigned const char private_key_bytes[381] = {
      0x30, 0x81, 0x87, 0x02, 0x01, 0x00, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
      0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
      0x03, 0x01, 0x07, 0x04, 0x6d, 0x30, 0x6b, 0x02, 0x01, 0x01, 0x04, 0x20,
      0x46, 0x4a, 0xa0, 0x20, 0x99, 0x69, 0x98, 0x72, 0x00, 0xc7, 0x8a, 0xc2,
      0xff, 0x4b, 0xf7, 0xa2, 0x5d, 0xf2, 0xbd, 0x3f, 0x72, 0x18, 0x25, 0xce,
      0xa0, 0x11, 0x23, 0x42, 0x99, 0xec, 0x38, 0x46, 0xa1, 0x44, 0x03, 0x42,
      0x00, 0x04, 0x7f, 0x02, 0xe5, 0xd3, 0x30, 0x53, 0xff, 0x00, 0x82, 0xf0,
      0xa5, 0x5b, 0x3b, 0x61, 0xa5, 0x2e, 0x5a, 0x18, 0xd9, 0x5c, 0x51, 0xa6,
      0x7d, 0x07, 0x2d, 0x68, 0x8e, 0xd9, 0xfc, 0x6c, 0x16, 0xb7, 0x75, 0xa6,
      0xc7, 0xf6, 0x18, 0x79, 0xfa, 0xda, 0x9a, 0x31, 0x6c, 0x28, 0x7d, 0xdc,
      0x53, 0xfe, 0xad, 0x6d, 0x69, 0xaa, 0x34, 0xff, 0x17, 0x69, 0x0a, 0xb0,
      0xa3, 0xf2, 0x1b, 0x33, 0xee, 0xfb};

  std::string data_string("Hi There");

  // Test Signing
  CryptoOps::PrivateKey private_key(
      std::string((const char*)private_key_bytes, 381),
      CryptoOps::KeyAlgorithm::ECDSA_KEY);
  unique_ptr<std::string> signature = CryptoOps::Sign(
      CryptoOps::SigType::ECDSA_P256_SHA256, private_key, data_string);
  EXPECT_NE(nullptr, signature);

  // Test Verifying
  CryptoOps::PublicKey public_key(
      std::string((const char*)public_key_bytes, 335),
      CryptoOps::KeyAlgorithm::ECDSA_KEY);
  bool signature_verifies =
      CryptoOps::Verify(CryptoOps::SigType::ECDSA_P256_SHA256, public_key,
                        *signature, data_string);
  EXPECT_TRUE(signature_verifies);
}

TEST(CryptoOpsTest, TestSignVerifyRsa2048Sha256) {
  // We don't really test anything other than a signature is generated
  // and no error occurs

  // We use a valid pre-computed keys
  const uint8_t private_key_bytes[1193] = {
      0x30, 0x82, 0x04, 0xa5, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
      0x99, 0xa5, 0x48, 0x16, 0x40, 0x19, 0x1e, 0xcc, 0x78, 0xcf, 0x53, 0xd0,
      0x61, 0x83, 0x3b, 0xfc, 0x5d, 0xb9, 0x2e, 0x9a, 0xfa, 0x04, 0x10, 0xfb,
      0x65, 0xd1, 0xef, 0x7b, 0x2c, 0x74, 0x25, 0xf3, 0x79, 0x6a, 0xf6, 0x3f,
      0xe7, 0x36, 0xbd, 0x55, 0xb4, 0x96, 0x0b, 0x95, 0x8e, 0x07, 0x1a, 0xbd,
      0x35, 0x44, 0xe5, 0xf8, 0x6f, 0xd5, 0xa7, 0x85, 0x77, 0x43, 0xf9, 0xa4,
      0x1a, 0x78, 0xb8, 0xcf, 0x62, 0xc8, 0x2a, 0x88, 0x47, 0x3a, 0x4f, 0xd6,
      0x3b, 0x21, 0x2b, 0xb2, 0x6e, 0xea, 0x13, 0xdd, 0xde, 0xf4, 0x55, 0xf4,
      0x4b, 0x57, 0x11, 0xf3, 0xb3, 0x92, 0xd6, 0xce, 0x57, 0x28, 0x9e, 0x85,
      0xba, 0x79, 0x3b, 0xbb, 0x0b, 0x35, 0x7a, 0xa4, 0x69, 0x84, 0x1f, 0xbe,
      0x89, 0x35, 0xbd, 0x02, 0x25, 0x28, 0xec, 0x6f, 0xeb, 0x3d, 0x0a, 0xb0,
      0x02, 0x6b, 0x4b, 0xdb, 0xd2, 0x7c, 0x4e, 0xed, 0x44, 0x4f, 0xa1, 0xe6,
      0xd9, 0xc4, 0xf7, 0xe7, 0x7d, 0x6d, 0x06, 0xf9, 0x5a, 0x6c, 0xf0, 0x48,
      0xc4, 0x1d, 0xf9, 0xe7, 0x28, 0xc7, 0x75, 0xa2, 0x4a, 0xa1, 0x35, 0x5a,
      0xad, 0x0f, 0x1a, 0x9c, 0x70, 0x98, 0xcb, 0xfc, 0x48, 0xab, 0xf5, 0xc6,
      0xa4, 0x94, 0xa6, 0x9b, 0x64, 0x06, 0x0e, 0xb9, 0x38, 0xa9, 0x84, 0x32,
      0x38, 0x13, 0x84, 0x70, 0x99, 0x79, 0x25, 0xfe, 0x2c, 0xc0, 0xde, 0xec,
      0x9f, 0x99, 0x51, 0x4f, 0xfc, 0xac, 0xe8, 0x0d, 0x87, 0x6d, 0x43, 0x8b,
      0x25, 0xa7, 0x54, 0x47, 0x42, 0xfe, 0xe2, 0x67, 0xff, 0xc2, 0x8f, 0x1b,
      0x0e, 0x1a, 0x24, 0xdf, 0xa7, 0x40, 0xcd, 0xa4, 0xa2, 0xb4, 0xad, 0x43,
      0x72, 0xbc, 0x8d, 0xc0, 0x1d, 0xe0, 0x77, 0x76, 0x4b, 0xe0, 0xc2, 0x32,
      0x7a, 0x57, 0x33, 0xee, 0xa8, 0x9c, 0xfa, 0x49, 0x67, 0x42, 0x15, 0x31,
      0x0e, 0xd0, 0xc2, 0x7b, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
      0x01, 0x00, 0x90, 0x21, 0x31, 0xfc, 0x5d, 0x68, 0xb3, 0x31, 0x90, 0x6a,
      0xbc, 0xef, 0x0f, 0x6a, 0x72, 0x4d, 0x0d, 0x06, 0x78, 0x27, 0xbd, 0x3d,
      0x2f, 0x49, 0x05, 0x7c, 0xe8, 0x30, 0x1d, 0xc5, 0x5f, 0x0c, 0x84, 0xeb,
      0xc7, 0xd0, 0xae, 0x6e, 0xf5, 0x31, 0x7e, 0xd3, 0xfa, 0x4d, 0xf3, 0x0b,
      0xcb, 0x13, 0x8e, 0xf2, 0xf0, 0xe0, 0x1a, 0xd8, 0xcb, 0xeb, 0x31, 0xb4,
      0x3c, 0x6c, 0xaa, 0xc0, 0x70, 0x6d, 0x6a, 0xf6, 0xe6, 0x9f, 0x2c, 0x62,
      0x11, 0x1e, 0xa0, 0x1d, 0x3f, 0xc6, 0x84, 0xb1, 0x64, 0xad, 0x8f, 0x46,
      0x99, 0x93, 0x2f, 0x51, 0xa0, 0x6f, 0x82, 0x16, 0xcf, 0x16, 0x85, 0x40,
      0x7f, 0x64, 0x67, 0x46, 0xee, 0xb7, 0x49, 0x8e, 0x83, 0x5b, 0xd3, 0xf7,
      0xb3, 0x67, 0xa3, 0x83, 0x1b, 0xa8, 0xf5, 0x7e, 0xec, 0x3d, 0x18, 0xe9,
      0x0c, 0x2b, 0x8c, 0x39, 0x66, 0xd1, 0xf1, 0x23, 0xd7, 0x4c, 0xdb, 0xc0,
      0x79, 0xb1, 0x21, 0x80, 0xe6, 0xd2, 0x57, 0xb7, 0xdb, 0x17, 0xd8, 0xc0,
      0xf1, 0x38, 0x4d, 0x69, 0xf7, 0x8c, 0xb8, 0xb8, 0xc9, 0x06, 0xc2, 0x01,
      0x72, 0x97, 0xf1, 0x5f, 0x69, 0x3a, 0x29, 0x7a, 0xe9, 0x44, 0xf2, 0x88,
      0xc9, 0x81, 0x42, 0x14, 0x3f, 0x06, 0x2f, 0x3e, 0xc0, 0xca, 0x4c, 0xc8,
      0xcc, 0xca, 0xa1, 0xf7, 0x29, 0x05, 0x3b, 0xf5, 0x05, 0x3f, 0x3d, 0xde,
      0xba, 0x10, 0x55, 0x29, 0xbe, 0x2a, 0xef, 0x1c, 0x7d, 0xaa, 0xff, 0x98,
      0x1a, 0x7d, 0xf5, 0xb4, 0x1c, 0xe6, 0x1a, 0x30, 0x97, 0x35, 0x04, 0x03,
      0x5c, 0x36, 0xca, 0x10, 0x2a, 0xbf, 0x6c, 0xbc, 0xbd, 0x44, 0xb8, 0x28,
      0xe5, 0x21, 0x42, 0xf6, 0x4a, 0x17, 0xa7, 0x85, 0x37, 0x76, 0xc3, 0x8a,
      0x05, 0x79, 0x6f, 0x5d, 0x89, 0x8d, 0xd4, 0x12, 0xa4, 0x55, 0xc1, 0x70,
      0xe2, 0x3c, 0x3a, 0x8e, 0x7a, 0x51, 0x02, 0x81, 0x81, 0x00, 0xcb, 0x5d,
      0xff, 0xd3, 0x7d, 0xc5, 0x33, 0x57, 0x71, 0xe9, 0xd5, 0x99, 0xb5, 0x6c,
      0x54, 0x5e, 0x9f, 0xf2, 0x01, 0x8e, 0x01, 0x22, 0x42, 0x3c, 0x32, 0x07,
      0x4b, 0x85, 0x8a, 0xce, 0x22, 0xdc, 0x21, 0x0d, 0x01, 0x48, 0xea, 0x3b,
      0x44, 0xa6, 0x41, 0x42, 0xd3, 0x09, 0xac, 0x98, 0xde, 0x78, 0x3e, 0x85,
      0x33, 0x32, 0xda, 0xa2, 0xd9, 0xc9, 0xc0, 0x23, 0x99, 0x26, 0xe4, 0xfb,
      0xc2, 0x4d, 0xb9, 0x7a, 0xab, 0x08, 0x07, 0xb1, 0xe0, 0x6b, 0xde, 0xd5,
      0x39, 0x17, 0xb0, 0x27, 0x09, 0xd4, 0x5b, 0xb2, 0x91, 0xd5, 0xfa, 0x6e,
      0xe7, 0x7c, 0xa4, 0xf6, 0x2f, 0x53, 0x45, 0xd3, 0xb8, 0x1f, 0x31, 0x9d,
      0xa8, 0x96, 0x01, 0xc8, 0x57, 0x6d, 0x21, 0x3a, 0xf8, 0x6b, 0xfe, 0xce,
      0xbf, 0xc0, 0x1c, 0xd9, 0x47, 0x57, 0x7a, 0x84, 0x19, 0xd4, 0xf0, 0xd6,
      0x88, 0xb2, 0x0a, 0xc1, 0x0f, 0xa5, 0x02, 0x81, 0x81, 0x00, 0xc1, 0x69,
      0x00, 0xd5, 0xd7, 0xaa, 0x04, 0xfe, 0x1d, 0xc6, 0x7d, 0x51, 0xcd, 0xc1,
      0xd9, 0x14, 0x76, 0x65, 0x10, 0x69, 0xf1, 0x7f, 0x5d, 0xec, 0xa6, 0x76,
      0x6e, 0xcf, 0xa9, 0x8e, 0x5e, 0x0e, 0x6f, 0x38, 0xd4, 0xe7, 0x3c, 0xd6,
      0x4b, 0xaa, 0x01, 0x2c, 0x8f, 0xc2, 0x64, 0xc3, 0xf1, 0x69, 0xcd, 0xbb,
      0x3d, 0xd1, 0xf0, 0x9f, 0xfb, 0xb5, 0x53, 0xdd, 0x9c, 0x69, 0xde, 0x4d,
      0x4f, 0xa8, 0xe3, 0x3a, 0xc2, 0x8b, 0x4b, 0x16, 0x8a, 0xfa, 0x34, 0xf5,
      0xf9, 0x5a, 0x5d, 0x88, 0x46, 0x68, 0xa5, 0xa0, 0xa3, 0xdc, 0xb8, 0x32,
      0xaa, 0x77, 0x66, 0x7c, 0xf5, 0x86, 0x6b, 0x73, 0xda, 0xd7, 0xd7, 0xda,
      0x5b, 0xd6, 0xf0, 0xaf, 0x66, 0x7e, 0x9e, 0xaf, 0x2d, 0x86, 0x29, 0x74,
      0xad, 0x79, 0xff, 0xa7, 0x7a, 0x5d, 0x24, 0x7c, 0x07, 0xc9, 0x8b, 0x52,
      0x39, 0x62, 0x74, 0x89, 0xef, 0x9f, 0x02, 0x81, 0x81, 0x00, 0xa5, 0x58,
      0x53, 0xbb, 0x7c, 0x32, 0x6d, 0x3f, 0xd7, 0x9a, 0x2a, 0xd4, 0xc2, 0x30,
      0xc5, 0x97, 0xf9, 0xab, 0x25, 0xa0, 0x73, 0x43, 0x8b, 0x5e, 0xad, 0xbe,
      0x48, 0xa4, 0xd6, 0xea, 0x2a, 0x65, 0x97, 0x69, 0x9f, 0x75, 0xcd, 0x1b,
      0x4b, 0x01, 0x71, 0x66, 0x07, 0x77, 0x82, 0x20, 0xf8, 0x20, 0x03, 0x95,
      0x00, 0xbf, 0x84, 0x0b, 0x8b, 0xcf, 0x00, 0xac, 0xf0, 0xc4, 0x32, 0xc0,
      0x8e, 0x85, 0xeb, 0x1b, 0xd5, 0x1a, 0xbe, 0x46, 0xdd, 0x14, 0x57, 0x24,
      0x3d, 0x3b, 0x09, 0x39, 0x74, 0x40, 0x78, 0x1d, 0x83, 0x7c, 0xda, 0x14,
      0x79, 0x99, 0x59, 0xf5, 0xdf, 0x1c, 0x71, 0x55, 0x66, 0x09, 0xd6, 0xa9,
      0x3c, 0x7c, 0x5a, 0x0e, 0xad, 0x26, 0x49, 0x32, 0x4c, 0xf0, 0x61, 0x47,
      0x6f, 0x97, 0x9e, 0xdc, 0xf9, 0xa8, 0x22, 0x30, 0x6d, 0x60, 0x38, 0x3f,
      0xf4, 0xfb, 0xec, 0xca, 0x73, 0x11, 0x02, 0x81, 0x81, 0x00, 0xa5, 0x94,
      0xaa, 0xc5, 0x68, 0xa4, 0x43, 0x2f, 0xf6, 0xf7, 0xd6, 0x94, 0x31, 0x2e,
      0x33, 0x15, 0xc4, 0xa2, 0x93, 0x61, 0xd0, 0x01, 0xb5, 0xbc, 0x83, 0x6a,
      0xc3, 0x45, 0x7f, 0xa8, 0xc5, 0xb7, 0x5f, 0xda, 0xec, 0xd2, 0xa7, 0x0f,
      0xe3, 0xa9, 0x40, 0xe6, 0x10, 0x91, 0x61, 0x49, 0x2b, 0x25, 0xe4, 0x9e,
      0xd7, 0xb7, 0x23, 0x65, 0x23, 0xce, 0x42, 0x65, 0x68, 0xa2, 0x6e, 0x52,
      0x0b, 0xcf, 0xcf, 0xf1, 0x9f, 0x5a, 0x37, 0x47, 0xae, 0x65, 0xb4, 0xef,
      0x9c, 0xb1, 0x93, 0x7f, 0xb6, 0x9f, 0xa2, 0xa4, 0x9b, 0x84, 0xbc, 0x21,
      0x8c, 0x35, 0x3a, 0x85, 0xe2, 0x81, 0x58, 0xfe, 0xcf, 0xad, 0x98, 0x3b,
      0x76, 0x02, 0xd6, 0xfd, 0xa3, 0x26, 0xe5, 0xdd, 0x9d, 0x80, 0xcd, 0x7e,
      0xf9, 0x81, 0x87, 0xb0, 0xaf, 0x1e, 0x8c, 0xbc, 0xae, 0xc2, 0x0a, 0x47,
      0xb3, 0x9f, 0x29, 0x9c, 0x69, 0x8b, 0x02, 0x81, 0x80, 0x6f, 0xa1, 0xb9,
      0xeb, 0x3c, 0xea, 0xed, 0x30, 0x1f, 0x39, 0x3b, 0x36, 0x38, 0x41, 0x74,
      0xad, 0xae, 0x9e, 0x40, 0x3e, 0x45, 0xc2, 0x4a, 0x47, 0x9b, 0x71, 0x5a,
      0xab, 0xb0, 0x97, 0x98, 0x90, 0x22, 0x35, 0x87, 0x56, 0xcf, 0x0a, 0xf9,
      0x52, 0x4b, 0x30, 0x90, 0x03, 0x77, 0x5c, 0x6c, 0x9a, 0x9e, 0x77, 0xf9,
      0x6e, 0x87, 0x3a, 0xb5, 0x18, 0x23, 0xcb, 0xde, 0xec, 0xe3, 0xa2, 0xb3,
      0x57, 0xb7, 0xd1, 0xd1, 0xb9, 0x3d, 0xaa, 0x33, 0xee, 0x38, 0x88, 0xf5,
      0x03, 0x47, 0xde, 0x57, 0xd7, 0x43, 0xa8, 0x28, 0x24, 0xd5, 0xab, 0x19,
      0xf1, 0x80, 0xf9, 0x84, 0x42, 0xc8, 0xa7, 0xe9, 0xd0, 0xf3, 0xb0, 0x4d,
      0xd4, 0x06, 0xb9, 0xcb, 0x22, 0x2a, 0x37, 0x98, 0xc0, 0x40, 0x2b, 0x6b,
      0x3b, 0xef, 0x86, 0xcc, 0x30, 0x2d, 0xbc, 0xae, 0x4c, 0x82, 0x6f, 0xd2,
      0x28, 0x4b, 0xba, 0xb4, 0xfb};

  const uint8_t public_key_bytes[294] = {
      0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
      0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
      0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0x99, 0xa5, 0x48,
      0x16, 0x40, 0x19, 0x1e, 0xcc, 0x78, 0xcf, 0x53, 0xd0, 0x61, 0x83, 0x3b,
      0xfc, 0x5d, 0xb9, 0x2e, 0x9a, 0xfa, 0x04, 0x10, 0xfb, 0x65, 0xd1, 0xef,
      0x7b, 0x2c, 0x74, 0x25, 0xf3, 0x79, 0x6a, 0xf6, 0x3f, 0xe7, 0x36, 0xbd,
      0x55, 0xb4, 0x96, 0x0b, 0x95, 0x8e, 0x07, 0x1a, 0xbd, 0x35, 0x44, 0xe5,
      0xf8, 0x6f, 0xd5, 0xa7, 0x85, 0x77, 0x43, 0xf9, 0xa4, 0x1a, 0x78, 0xb8,
      0xcf, 0x62, 0xc8, 0x2a, 0x88, 0x47, 0x3a, 0x4f, 0xd6, 0x3b, 0x21, 0x2b,
      0xb2, 0x6e, 0xea, 0x13, 0xdd, 0xde, 0xf4, 0x55, 0xf4, 0x4b, 0x57, 0x11,
      0xf3, 0xb3, 0x92, 0xd6, 0xce, 0x57, 0x28, 0x9e, 0x85, 0xba, 0x79, 0x3b,
      0xbb, 0x0b, 0x35, 0x7a, 0xa4, 0x69, 0x84, 0x1f, 0xbe, 0x89, 0x35, 0xbd,
      0x02, 0x25, 0x28, 0xec, 0x6f, 0xeb, 0x3d, 0x0a, 0xb0, 0x02, 0x6b, 0x4b,
      0xdb, 0xd2, 0x7c, 0x4e, 0xed, 0x44, 0x4f, 0xa1, 0xe6, 0xd9, 0xc4, 0xf7,
      0xe7, 0x7d, 0x6d, 0x06, 0xf9, 0x5a, 0x6c, 0xf0, 0x48, 0xc4, 0x1d, 0xf9,
      0xe7, 0x28, 0xc7, 0x75, 0xa2, 0x4a, 0xa1, 0x35, 0x5a, 0xad, 0x0f, 0x1a,
      0x9c, 0x70, 0x98, 0xcb, 0xfc, 0x48, 0xab, 0xf5, 0xc6, 0xa4, 0x94, 0xa6,
      0x9b, 0x64, 0x06, 0x0e, 0xb9, 0x38, 0xa9, 0x84, 0x32, 0x38, 0x13, 0x84,
      0x70, 0x99, 0x79, 0x25, 0xfe, 0x2c, 0xc0, 0xde, 0xec, 0x9f, 0x99, 0x51,
      0x4f, 0xfc, 0xac, 0xe8, 0x0d, 0x87, 0x6d, 0x43, 0x8b, 0x25, 0xa7, 0x54,
      0x47, 0x42, 0xfe, 0xe2, 0x67, 0xff, 0xc2, 0x8f, 0x1b, 0x0e, 0x1a, 0x24,
      0xdf, 0xa7, 0x40, 0xcd, 0xa4, 0xa2, 0xb4, 0xad, 0x43, 0x72, 0xbc, 0x8d,
      0xc0, 0x1d, 0xe0, 0x77, 0x76, 0x4b, 0xe0, 0xc2, 0x32, 0x7a, 0x57, 0x33,
      0xee, 0xa8, 0x9c, 0xfa, 0x49, 0x67, 0x42, 0x15, 0x31, 0x0e, 0xd0, 0xc2,
      0x7b, 0x02, 0x03, 0x01, 0x00, 0x01};

  std::string data_string("Hi There");

  // Test Signing
  CryptoOps::PrivateKey private_key(
      ByteBuffer(private_key_bytes, 1193).String(),
      CryptoOps::KeyAlgorithm::RSA_KEY);
  unique_ptr<std::string> signature = CryptoOps::Sign(
      CryptoOps::SigType::RSA2048_SHA256, private_key, data_string);
  EXPECT_NE(nullptr, signature);

  // Test Verifying
  CryptoOps::PublicKey public_key(ByteBuffer(public_key_bytes, 294).String(),
                                  CryptoOps::KeyAlgorithm::RSA_KEY);
  bool signature_verifies = CryptoOps::Verify(
      CryptoOps::SigType::RSA2048_SHA256, public_key, *signature, data_string);
  EXPECT_TRUE(signature_verifies);
}

TEST(CryptoOpsTest, TestEcKeyAgreement) {
  unique_ptr<CryptoOps::KeyPair> client_key_pair =
      CryptoOps::GenerateEcP256KeyPair();
  unique_ptr<CryptoOps::KeyPair> server_key_pair =
      CryptoOps::GenerateEcP256KeyPair();

  std::string client_y, server_y;
  bool success;
  success = CryptoOps::ExportEcP256Key(*(client_key_pair->public_key),
                                       NULL, &client_y);
  ASSERT_TRUE(success);
  success = CryptoOps::ExportEcP256Key(*(server_key_pair->public_key),
                                       NULL, &server_y);
  ASSERT_TRUE(success);
  // Public Keys should not be equal.
  ASSERT_NE(client_y, server_y);

  // Run client side of the key exchange
  unique_ptr<CryptoOps::SecretKey> client_secret =
      CryptoOps::KeyAgreementSha256(*(client_key_pair->private_key),
                                    *(server_key_pair->public_key));
  ASSERT_NE(client_secret, nullptr);

  // Run the server side of the key exchange
  unique_ptr<CryptoOps::SecretKey> server_secret =
      CryptoOps::KeyAgreementSha256(*(server_key_pair->private_key),
                                    *(client_key_pair->public_key));
  ASSERT_NE(server_secret, nullptr);

  ASSERT_TRUE(client_secret->data().Equals(server_secret->data()));
}


TEST(CryptoOpsTest, TestGenerateAes256SecretKey) {
  unique_ptr<CryptoOps::SecretKey> key = CryptoOps::GenerateAes256SecretKey();
  EXPECT_NE(key, nullptr);
  EXPECT_EQ(CryptoOpsTest::AesKeySize(), key->data().size());
}

TEST(CryptoOpsTest, TestGenerateEcP256KeyPair) {
  unique_ptr<CryptoOps::KeyPair> key_pair = CryptoOps::GenerateEcP256KeyPair();
  EXPECT_NE(key_pair, nullptr);
  EXPECT_GT(key_pair->private_key->data().size(), 0U);
  EXPECT_GT(key_pair->public_key->data().size(), 0U);
}

TEST(CryptoOpsTest, TestGenerateRsa2048KeyPair) {
  unique_ptr<CryptoOps::KeyPair> key_pair = CryptoOps::GenerateRsa2048KeyPair();
  EXPECT_NE(key_pair, nullptr);
  EXPECT_GT(key_pair->private_key->data().size(), 0U);
  EXPECT_GT(key_pair->public_key->data().size(), 0U);
}

TEST(CryptoOpsTest, TestInt32BytesToString) {
  EXPECT_EQ(std::string("\x05\xF2\x23\x00", 4),
            CryptoOpsTest::Int32BytesToString(0x05F22300));

  // All zero bytes.
  EXPECT_EQ(std::string("\x0\x0\x0\x0", 4),
            CryptoOpsTest::Int32BytesToString(0));

  // Leading 0 byte.
  EXPECT_EQ(std::string("\x00\x00\x93\x06", 4),
            CryptoOpsTest::Int32BytesToString(0x00009306));

  // Negative value.
  EXPECT_EQ(std::string("\xE0\x00\x00\x00", 4),
            CryptoOpsTest::Int32BytesToString(0xE0000000));

  // Negative non-leading bytes.
  EXPECT_EQ(std::string("\x0F\x81\xA3\x99", 4),
            CryptoOpsTest::Int32BytesToString(0x0F81A399));
}

TEST(CryptoOpsTest, TestStringToInt32Bytes) {
  // Initially set to an arbitrary value;
  int32_t result = 0x34F4EE21;

  // Empty string.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(std::string(), &result));
  EXPECT_EQ(0, result);

  // All zero bytes.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\x00\x00\x00\x00", 4), &result));
  EXPECT_EQ(0, result);

  // Positive value.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\x05\xF2\x23\x00", 4), &result));
  EXPECT_EQ(0x05F22300, result);

  // Leading 0 byte.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\x00\x00\x93\x06", 4), &result));
  EXPECT_EQ(0x00009306, result);

  // Negative value.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\xE0\x00\x00\x00", 4), &result));
  EXPECT_EQ(0xE0000000, result);

  // Negative non-leading bytes.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\x0F\x81\xA3\x99", 4), &result));
  EXPECT_EQ(0x0F81A399, result);

  // String shorter than 4 bytes.
  EXPECT_TRUE(CryptoOpsTest::StringToInt32Bytes(std::string("\x81\xA3\x99", 3),
                                                &result));
  EXPECT_EQ(0x0081A399, result);

  // String longer than 4 bytes (invalid value).
  EXPECT_FALSE(CryptoOpsTest::StringToInt32Bytes(
      std::string("\x00\x00\x81\xA3\x99", 5), &result));
}

TEST(CryptoOpsTest, SecureRandom) {
  unique_ptr<ByteBuffer> random_bytes;

  // Length must be positive.
  random_bytes = CryptoOps::SecureRandom(0);
  EXPECT_FALSE(random_bytes);

  // Check length of |random_bytes| the same as the length requested.
  const size_t length1 = 32;
  random_bytes = CryptoOps::SecureRandom(length1);
  ASSERT_TRUE(random_bytes);
  EXPECT_EQ(length1, random_bytes->size());

  const size_t length2 = 64;
  random_bytes = CryptoOps::SecureRandom(length2);
  ASSERT_TRUE(random_bytes);
  EXPECT_EQ(length2, random_bytes->size());
}

TEST(CryptoOpsTest, Sha256) {
  // Hashing an empty message should fail.
  EXPECT_FALSE(CryptoOps::Sha256(ByteBuffer(std::string())));

  // Examples from
  // http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA256.pdf
  {
    const std::string input = "abc";
    const std::string digest =
        "0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    unique_ptr<ByteBuffer> hash = CryptoOps::Sha256(ByteBuffer(input));
    EXPECT_TRUE(hash);
    EXPECT_EQ(digest, hash->AsDebugHexString());
  }

  {
    const std::string input =
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const std::string digest =
        "0x248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
    unique_ptr<ByteBuffer> hash = CryptoOps::Sha256(ByteBuffer(input));
    EXPECT_TRUE(hash);
    EXPECT_EQ(digest, hash->AsDebugHexString());
  }
}

TEST(CryptoOpsTest, Sha512) {
  // Hashing an empty message should fail.
  EXPECT_FALSE(CryptoOps::Sha512(ByteBuffer(std::string())));

  // Examples from
  // http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA512.pdf
  {
    const std::string input = "abc";
    const std::string digest =
        "0xddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a219"
        "2992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";

    unique_ptr<ByteBuffer> hash = CryptoOps::Sha512(ByteBuffer(input));
    EXPECT_TRUE(hash);
    EXPECT_EQ(digest, hash->AsDebugHexString());
  }

  {
    const std::string input =
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklm"
        "nopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    const std::string digest =
        "0x8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501"
        "d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909";
    unique_ptr<ByteBuffer> hash = CryptoOps::Sha512(ByteBuffer(input));
    EXPECT_TRUE(hash);
    EXPECT_EQ(digest, hash->AsDebugHexString());
  }
}

}  // namespace
}  // namespace unittesting
}  // namespace securemessage
