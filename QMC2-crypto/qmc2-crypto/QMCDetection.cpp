#define _CRT_SECURE_NO_WARNINGS
#include "qmc2-crypto/QMCDetection.h"
#include "qmc2-crypto/__endian_helper.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

// Taken from: https://stackoverflow.com/a/3312896
#if defined(_MSC_VER)
#define PACK(__decl__) __pragma(pack(push, 1)) __decl__ __pragma(pack(pop))
#else
#define PACK(__decl__) __decl__ __attribute__((__packed__))
#endif

union eof_magic {
  PACK(struct {
    uint32_t _unused;
    uint32_t len; // LittleEndian
  })
  v1;

  PACK(struct {
    uint32_t len; // BigEndian
    uint32_t magic;
  })
  v2;
};

const char *MAGIC_QTAG = "QTag";

size_t find_comma(uint8_t *buf, size_t start, size_t end)
{
  for (auto i = start; i < end; i++)
  {
    if (buf[i] == ',')
    {
      return i;
    }
  }

  return -1;
}

bool detect_key_end_position(qmc_detection &result, uint8_t *buf, size_t len)
{
  memset(&result, 0x00, sizeof(result));
  if (len < sizeof(eof_magic))
  {
    strncpy(result.error_msg, "buffer too small", sizeof(result.error_msg));
    return false;
  }

  eof_magic eof = *reinterpret_cast<eof_magic *>(&buf[len - sizeof(eof_magic)]);
  {
    if (memcmp(MAGIC_QTAG, &eof.v2.magic, 4) == 0)
    {
      uint32_t payload_size = static_cast<uint32_t>(be32toh(eof.v2.len));

      result.ekey_position = int32_t(len) - 8 - payload_size;
      size_t start_idx = std::max(result.ekey_position, 0);
      size_t iter_max = len - sizeof(eof);

      auto key_end_loc = find_comma(buf, start_idx, iter_max);
      if (key_end_loc < 0)
      {
        strncpy(result.error_msg, "could not find end of ekey", sizeof(result.error_msg));
        return false;
      }

      result.ekey_len = key_end_loc - result.ekey_position;

      auto song_id_start_loc = key_end_loc + 1;
      auto song_id_end_loc = find_comma(buf, song_id_start_loc, iter_max);
      if (song_id_end_loc > 0)
      {
        size_t real_song_id_size = song_id_end_loc - song_id_start_loc;
        if (real_song_id_size <= sizeof(result.song_id) - 1)
        {
          memcpy(result.song_id, &buf[song_id_start_loc], real_song_id_size);
        }
        else
        {
          memcpy(result.song_id, "(overflow)", 10);
        }
      }

      return true;
    }
    // not v2
  }

  {
    // little-endian to host-endian
    uint32_t key_len = static_cast<uint32_t>(le32toh(eof.v1.len));

    // Currently known largest key size is 0x03f1 (1009)
    if (key_len < 0x400)
    {
      result.ekey_position = int32_t(len) - sizeof(eof.v1.len) - key_len;
      result.ekey_len = key_len;
      return true;
    }
  }

  if (eof.v1.len == 0)
  {
    strncpy(result.error_msg, "last 4 bytes are ZERO", sizeof(result.error_msg));
  }
  else
  {
    sprintf(result.error_msg, "unknown magic: %08x-%08x", eof.v2.magic, eof.v2.len);
  }

  return false;
}
