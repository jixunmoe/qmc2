﻿// QMC2-decoder.cpp : Defines the entry point for the application.
//

#include "QMC2-decoder.h"

#include <qmc2-crypto/IKeyDec.h>
#include <qmc2-crypto/QMCDetection.h>
#include <qmc2-crypto/StreamCencrypt.h>

#include <cstring>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

// 1M buffer
constexpr size_t read_buf_len = 1 * 1024 * 1024;

static_assert(read_buf_len > footer_detection_size && read_buf_len > encrypted_key_size_v1 &&
                  read_buf_len > encrypted_key_size_v2,
              "'read_buf_len' should be larger than 'footer_detection_size' "
              "and 'encrypted_key_size'.");

using namespace std;

StreamCencrypt *createInstWidthEKey(const char *ekey_b64, uint8_t seed, const uint8_t *mix_key_1,
                                    const uint8_t *mix_key_2)
{
  auto key_dec = std::make_unique<KeyDec>();
  key_dec->InitDecryptionKey(seed, mix_key_1, mix_key_2);
  if (!key_dec->SetKey(ekey_b64, strlen(ekey_b64)))
  {
    return nullptr;
  }
  StreamCencrypt *stream = new StreamCencrypt();
  stream->SetKeyDec(key_dec.get());
  return stream;
}

int main(int argc, char **argv)
{
  fprintf(stderr, "QMC2 decoder (cli) v1.0.0-R1 by Jixun\n\n");

  if (argc < 6)
  {
    fprintf(stderr, "usage: %s <seed> <mix_key_1> <mix_key_2> <input> <output> [ignored]\n", argv[0]);
    return 1;
  }

  auto seed = (uint8_t)atoi(argv[1]);
  uint8_t mix_key_1[16] = {0};
  uint8_t mix_key_2[16] = {0};
  memcpy(mix_key_1, argv[2], 16);
  memcpy(mix_key_2, argv[3], 16);

  ifstream stream_input(argv[4], ios::in | ios::binary);
  if (stream_input.fail())
  {
    fprintf(stderr, "ERROR: could not open input file %s\n", argv[1]);
    return 1;
  }

  uint8_t *buf = new uint8_t[read_buf_len]();

  // embeded ekey detection & extraction
  stream_input.seekg(0, ios::end);
  auto input_file_len = size_t(stream_input.tellg());
  stream_input.seekg(input_file_len - footer_detection_size, ios::beg);
  stream_input.read(reinterpret_cast<char *>(buf), footer_detection_size);

  qmc_detection detection;
  if (!detect_key_end_position(detection, buf, footer_detection_size))
  {
    fprintf(stderr, "ERROR: could not derive embedded ekey from file.\n");
    fprintf(stderr, "       %s\n", detection.error_msg);
    stream_input.close();
    delete[] buf;
    return 1;
  }

  fprintf(stderr, "song_id: %s\n", detection.song_id[0] ? detection.song_id : "(unknown)");

  // size_t decrypted_file_size = input_file_len - footer_detection_size +
  // position - encrypted_key_size;
  size_t decrypted_file_size = input_file_len - footer_detection_size + detection.ekey_position;

  // Extract base64_encoded_ekey
  stream_input.seekg(decrypted_file_size, ios::beg);
  stream_input.read(reinterpret_cast<char *>(buf), detection.ekey_len);
  buf[detection.ekey_len] = 0;
  auto stream = createInstWidthEKey(reinterpret_cast<char *>(buf), seed, mix_key_1, mix_key_2);
  if (!stream)
  {
    fprintf(stderr, "ERROR: could not parse embed_key\n");
    return 1;
  }

  ofstream stream_out(argv[5], ios::out | ios::binary);
  if (stream_out.fail())
  {
    fprintf(stderr, "ERROR: could not open output file %s\n", argv[2]);
    return 1;
  }

  fprintf(stderr, "decrypting...");
  fflush(stderr);

  // Reset input position
  stream_input.seekg(0, ios::beg);
  uint64_t offset = 0;
  size_t to_decrypt_len = decrypted_file_size;

  // Begin decryption
  while (to_decrypt_len > 0)
  {
    auto block_size = std::min(read_buf_len, to_decrypt_len);
    stream_input.read(reinterpret_cast<char *>(buf), block_size);
    auto bytes_read = stream_input.gcount();

    stream->StreamDecrypt(offset, buf, bytes_read);
    stream_out.write(reinterpret_cast<char *>(buf), bytes_read);

    offset += bytes_read;
    to_decrypt_len -= bytes_read;
    fprintf(stderr, ".");
    fflush(stderr);
  }

  fprintf(stderr, "ok! saved to %s\n", argv[5]);

  delete[] buf;
  return 0;
}
