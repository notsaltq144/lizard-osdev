#include <uefi.h>
#include <stdbool.h>
#include <string.h>
#include "uefi_types.h"
#include "verify_types.h"
#include "magic.h"

#define RETURN___STATUS_ERROR 1
#define RETURN___CRC32_NO_MATCH 2
#define RETURN___RESERVED_WRONG 3

const char *OSNAME = "lizard";
const char *BINNAME = "/EFI/BOOT/BOOTX64.EFI";
const char *OBJNAME = "uefi.o";
const int backupClearscreenNewlineCount = 200;

void bcdToAscii(unsigned char src, char *dest); // This function is licensed under CC BY-SA 2.5. See definition for more details.
int charToBcd(int x); // This function however, is not.
int askUserContinue(const char *message, int noVal, int yesVal);
char *specializedShortToString(const char* buffer, short x);

int main(void) {
	/* some variables */
	efi_status_t status;
	u64 tmp = 0;
	/* we don't need UEFI console bloat */
	status = ST->ConOut->ClearScreen(ST->ConOut);
	if (EFI_ERROR(status)) {
		for (int i = 0; i < backupClearscreenNewlineCount; i++) printf("\n");
		tmp = 1;
	}
	/* some info */
	printf("Booting to %s, currently in src:%s obj:%s bin:%s!\n", OSNAME, __FILE__, OBJNAME, BINNAME);
	if (tmp == 1)
		printf("WARNING: Screen was unable clear using UEFI. The screen was cleared using %d newlines. This may be a sympton of a larger problem with %s or your firmware.\n",
		  backupClearscreenNewlineCount, OSNAME);
	printf("A LOT of messages will be printed.\n", charToBcd(backupClearscreenNewlineCount));
	/* uefi watchdog is a good thing, no uefi app should run for ~5 minutes, except ui applications, which this is (very minimaly) */
	ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	printf("Disabled UEFI watchdog. If %s hangs, UEFI will not forcefully exit.\n", OSNAME);

	u64 revision_buffer_canary_pre = canary_value;
#warning you need to account for version 2.4.6 and similar
	char revision[5+1+2+1]; /* SIZE: 5 (major) + 1 (dot) + 2 (minor) + 1 (null) */
	u64 revision_buffer_canary_post = canary_value;
	char *dotAddr = specializedShortToString(revision, ((ST->Hdr.Revision) & 0xFFFF0000) >> 16);
	*dotAddr++ = '.';

	bcdToAscii((char)ST->Hdr.Revision & 0x000000FF, dotAddr);
	printf("SystemTable (ST) Header (ST->Hdr)\n");
	printf("  Signature: 0x%x\n", ST->Hdr.Signature);
	printf("  Revision: 0x%x (%s, this may not have a spec listed with it, in this case round down to the nearest with errata in mind)\n", ST->Hdr.Revision, revision);
	printf("  CRC32: 0x%x\n", ST->Hdr.CRC32);
	printf("  HeaderSize: %d\n", ST->Hdr.HeaderSize);
	if (ST->Hdr.Reserved != 0) if (askUserContinue("ST->Hdr.Reserved value is incorrect. Do you want to continue [y/n]. ", RETURN___RESERVED_WRONG, 0)) return RETURN___RESERVED_WRONG;
	if ((revision_buffer_canary_pre != canary_value) || (revision_buffer_canary_post != canary_value)) {
		printf("Canary corruption, system may be in unstable state!\n Data: sizeof(buffer) %d canary-pre %p canary-post %p buffer %p\n",
		  sizeof(revision), &revision_buffer_canary_pre, &revision_buffer_canary_post, revision); 
		if (askUserContinue("Canary corruption has occured. Do you want to continue [y/n]. ", RETURN___STATUS_ERROR, 0)) return RETURN___STATUS_ERROR;
	}
	/* calculate crc32 of SystemTable */
	uint32_t calculatedCRC32;
	uint32_t passedCRC32 = ST->Hdr.CRC32;
	ST->Hdr.CRC32 = 0; /* this is how it's calculated. spec says it's calculated with this zerod */
	status = ST->BootServices->CalculateCrc32(&ST->Hdr, ST->Hdr.HeaderSize, &calculatedCRC32); /* calculate and store status */
	ST->Hdr.CRC32 = passedCRC32; /* repair system table header */
	if (EFI_ERROR(status)) {
		if (askUserContinue("Unable to calculate CRC32 of ST->Hdr. Do you want to continue [y/n]. ", RETURN___STATUS_ERROR, 0)) return RETURN___STATUS_ERROR;
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

/*
 * The following code is licensed unfer CC BY-SA 2.5
 * Source: https://stackoverflow.com/a/784455/19418319
 * The name of the function have been modified without endorsement from the original source
 * Written by GManNickG (https://stackoverflow.com/users/87234/gmannickg)
 * Modified by saltq144 on github (https://github.com/saltq144)
 * A function that reverses a string.
 * (c) GManNickG 2009
 */
void strrev(char *str)
{
    /* skip null */
    if (str == 0)
    {
        return;
    }

    /* skip empty string */
    if (*str == 0)
    {
        return;
    }

    /* get range */
    char *start = str;
    char *end = start + strlen(str) - 1; /* -1 for \0 */
    char temp;

    /* reverse */
    while (end > start)
    {
        /* swap */
        temp = *start;
        *start = *end;
        *end = temp;

        /* move */
        ++start;
        --end;
    }
}

int charToBcd(int x) {
	/* i have nothing to say, just visualize ret wot and x while running this. it works */
	int ret = 0;
	int wot = 1;
	while (x) {
		ret += (x % 10) * wot;
		x = x / 10;
		wot *= 16;
	}
	return ret;
}

int askUserContinue(const char *message, int noVal, int yesVal) {
	char userInput = ' ';
askUserContinue_ask:
	printf(message);
	userInput = getchar();
	printf("%c\n", userInput);
	switch (userInput) {
		case 'n':
		case 'N':
		return noVal;
		case 'y':
		case 'Y':
		return yesVal;
	}
	goto askUserContinue_ask;
}

char *specializedShortToString(const char* buffer, short x) {
	char *nullAdr = (char*)buffer;
	while (x) {
		*nullAdr++ = '0' + (x % 10);
		x /= 10;
	}
	*nullAdr = 0;
	strrev((char*)buffer);
	return nullAdr;
}
