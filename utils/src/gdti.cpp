#include <cstdlib>
#include <vector>
#include <fstream>
#include <cstdint>
// copied from <https://wiki.osdev.org/Getting_to_Ring_3>
struct gdt_entry_bits {
	unsigned int limit_low              : 16;
	unsigned int base_low               : 24;
	unsigned int accessed               :  1;
	unsigned int read_write             :  1; // readable for code, writable for data
	unsigned int conforming_expand_down :  1; // conforming for code, expand down for data
	unsigned int code                   :  1; // 1 for code, 0 for data
	unsigned int code_data_segment      :  1; // should be 1 for everything but TSS and LDT
	unsigned int DPL                    :  2; // privilege level
	unsigned int present                :  1;
	unsigned int limit_high             :  4;
	unsigned int available              :  1; // only used in software; has no effect on hardware
	unsigned int long_mode              :  1;
	unsigned int big                    :  1; // 32-bit opcodes for code, uint32_t stack for data
	unsigned int gran                   :  1; // 1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high              :  8;
} __attribute__((packed));

void dump_gdte(gdt_entry_bits* e) {
	if ((*(uint64_t*) e) == 0) { printf("  NULL DESCRIPTOR\n"); return; }
	printf("  limit:        0x%x%x\n", e->limit_high, e->limit_low);
	printf("  base:         0x%x%x\n", e->base_high, e->base_low);
	printf("  accessed:     %i\n", e->accessed);
	printf("  %s    %s\n", e->code ? "writeable:" : "readable: ", e->read_write ? "no" : "yes");
	printf("  %s  %s\n", e->code ? "conforming: " : "expand down:", e->conforming_expand_down ? "no" : "yes");
	printf("  code:         %s\n", e->code && e->code_data_segment ? "yes" : "no");
	printf("  data:         %s\n", !e->code && e->code_data_segment ? "yes" : "no");
	printf("  DPL:          %i\n", e->DPL);
	printf("  present:      %s\n", e->present ? "yes" : "no");
	printf("  available:    %s\n", e->available ? "yes" : "no");
	printf("  long mode:    %s\n", e->long_mode ? "yes" : "no");
	printf("  size/db/bits: %s\n", e->big ? "32" : "16");
	printf("  granularity:  %s\n", e->gran ? "4k" : "1b");
		
}

int main(int argc, char **argv) {
	gdt_entry_bits* gdte;
	printf("NOTE: assuming you've given an argument\n");
	std::ifstream gdtf(argv[1], std::ios::binary);
	if (!gdtf) return 1;
	std::vector<char> data = std::vector<char>(std::istreambuf_iterator<char>(gdtf), std::istreambuf_iterator<char>());
	for (int i = 0; i < data.size(); i += 8) {
		printf("INDEX %i/%i\n", i, i / 8);
		dump_gdte((gdt_entry_bits*)&data[i]);
	}
}

