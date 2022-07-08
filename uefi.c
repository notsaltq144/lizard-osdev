#include <uefi.h>
#include <stdbool.h>

#define RETURN___STATUS_ERROR 1
#define RETURN___CRC32_NO_MATCH 2

const char *OSNAME = "lizard";
const char *BINNAME = "BOOTX64.EFI";
const char *OBJNAME = "uefi.o";

void bcdToAscii(unsigned char src, char *dest); // This function is licensed under CC BY-SA 2.5. See definition for more details.
int charToBcd(char x); // This function however, is not.

int main(int argc, char **argv) {
	/* some variables */
	efi_status_t status;
	/* we don't need UEFI console bloat */
	status = ST->ConOut->ClearScreen(ST->ConOut);
	if (EFI_ERROR(status)) {
		printf("Unable to clear screen\n");
		return RETURN___STATUS_ERROR;
	}
	printf("Booting to %s, currently in src:%s obj:%s bin:%s!\n", OSNAME, __FILE__, OBJNAME, BINNAME);
	printf("A LOT of messages will be printed.\n");
	ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	char revision[8];
	revision[0] = '0' + ( ST->Hdr.Revision >> 16 );
	revision[1] = '.';
	bcdToAscii((char)ST->Hdr.Revision & 0x000000FF, revision + 2); /* In spec! Minor can be in range 0..99 no more, this is in range 0..255, way more! */
	printf("Disabled UEFI watchdog. If %s hangs, UEFI will not forcefully exit.\n", OSNAME);
	printf("SystemTable (ST) Header (ST->Hdr)\n");
	printf("  Signature: 0x%x\n", ST->Hdr.Signature);
	printf("  Revision: 0x%x (%s, this may not have a spec listed with it, in this case round down to the nearest with errata in mind)\n", ST->Hdr.Revision, revision);
	printf("  CRC32: 0x%x\n", ST->Hdr.CRC32);
	printf("  HeaderSize: %x\n", charToBcd(ST->Hdr.HeaderSize));

	/* calculate crc32 of SystemTable */
	uint32_t calculatedCRC32;
	uint32_t passedCRC32 = ST->Hdr.CRC32;
	ST->Hdr.CRC32 = 0; /* this is how it's calculated. spec says it's calculated with this zerod */
	status = ST->BootServices->CalculateCrc32(&ST->Hdr, ST->Hdr.HeaderSize, &calculatedCRC32); /* calculate and store status */
	ST->Hdr.CRC32 = passedCRC32; /* repair system table header */
	if (EFI_ERROR(status)) {
		printf("Unable to calculate CRC32 of ST->Hdr\n");
		return RETURN___STATUS_ERROR;
	}
	/* print crc32 pair */
	printf("CRC32 of ST->Hdr:\n");
	printf("  Calculated: %x\n", calculatedCRC32);
	printf("  Passed: %x\n", passedCRC32);
	printf("  Status: %s\n", calculatedCRC32 == passedCRC32 ? "Match!" : "No match!");
	if (calculatedCRC32 != passedCRC32) {
		printf("Calculated CRC32 and given CRC32 do not match (ST->Hdr)\n");
		return RETURN___CRC32_NO_MATCH;
	}
	while (true);
	return 0;
}
/*
 * The following code is licensed under CC BY-SA 2.5
 * Source: https://stackoverflow.com/a/3579170/19418319
 * The name (not parameters) of the function has been modified without endorsement from the original source. Spaces have been modified to tabs without endorsement from the original source.
 * Written by Thomas Matthews on stackoverflow (https://stackoverflow.com/users/225074/thomas-matthews)
 * Modified by saltq144 on github (https://github.com/saltq144)
 * A function that converts a given BCD byte to a string and stores it in a given destination
 * (c) Thomas Matthews 2010
 */
void bcdToAscii(unsigned char src, char *dest) {
	static const char outputs[] = "0123456789ABCDEF";
	*dest++ = outputs[src>>4];
	*dest++ = outputs[src&0xf];
	*dest = '\0';
}
/*
 * End of CC BY-SA licensed code.
 */
int charToBcd(char x) {
	return (x % 10) + ((x % 100) / 10)*16 + ((x % 1000) / 100)*16*16; /* what the fuck */
}

