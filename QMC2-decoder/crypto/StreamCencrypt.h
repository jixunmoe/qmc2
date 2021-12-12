#pragma once

#include "IStreamEncryptAndDecrypt.h"
#include "KeyDec.h"

#include <cstdint>

class StreamCencrypt : public IStreamEncryptAndDecrypt {
public:
	void StreamEncrypt(uint64_t offset, uint8_t* buf, size_t len);
	void StreamDecrypt(uint64_t offset, uint8_t* buf, size_t len);

	bool CheckCallerLegal() {
		return true;
	}
	void Uninit();
	void EncASegment(uint8_t* sbox, size_t offset, uint8_t* buf, size_t len);
	void EncFirstSegment(size_t offset, uint8_t* buffer, size_t size);
	void ProcessByRC4(size_t offset, uint8_t* buffer, size_t buffer_size);

private:
	uint32_t key_hash = 0;

	// RC4 vars
	uint8_t* rc4_key = nullptr;
	uint8_t* S = nullptr;
	size_t N = 0;

	void InitRC4KSA();
	void GetHashBase();
	uint8_t mapL(uint64_t offset);

	uint64_t GetSegmentKey(uint64_t a, uint64_t b);

	void SetKeyDec(KeyDec* key_dec);
};
