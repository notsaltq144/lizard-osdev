// This is ported to C from <https://github.com/jancajthaml-go/crc64/blob/master/crc.go>
// That code is licensed under <Unlicense>, however this code is licensed under the license of this (osdev) project.

#include <uefi.h>

uint64_t crc64(uint8_t data[], size_t len, uint64_t poly, uint64_t init, uint64_t xorout) {
	uint64_t crc = init;
	uint64_t bit;
	for (size_t i = 0; i < len; i++) {
		for (uint8_t j = 0x80; j != 0; j >>= 1) {
			if ((data[i] & j) != 0) bit = (crc & 0x8000000000000000) ^ 0x8000000000000000;
			else bit = crc & 0x8000000000000000;

			if (bit == 0) crc <<= 1;
			else crc = (crc << 1) ^ poly;
		}
	}
	return crc ^ xorout;
}

