#include <uefi.h>
#include <stdbool.h>
#include <string.h>
#include "magic.h"

#define RETURN___STATUS_ERROR 1
#define RETURN___CRC32_NO_MATCH 2
#define RETURN___RESERVED_WRONG 3

#include "osinfo.h"
#include "crc64/crc64.c"
#include "regs.h"

const char *BINNAME = "efipart/EFI/BOOT/BOOTX64.EFI";
const char *OBJNAME = "uefi.o";
const int backupClearscreenNewlineCount = 200;

int charToBcd(int x);
int askUserContinue(const char *message, int noVal, int yesVal);
char *specializedShortToString(char *buffer, short x);
void apitest(void);
void call_function_with_gregs(void *func, gregs_t *regs) __attribute__((sysv_abi, no_caller_saved_registers));

int main(void) {
	/* some variables */
	efi_status_t status;
	uint64_t tmp = 0;
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
	status = ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
	if (EFI_ERROR(status)) if (askUserContinue("Unable to disable UEFI watchdog. Do you want to continue [y/n]. ", RETURN___STATUS_ERROR, 0)) return RETURN___STATUS_ERROR;
	if (!EFI_ERROR(status)) printf("Disabled UEFI watchdog. If %s hangs, UEFI will not forcefully exit.\n", OSNAME);

	uint64_t revision_buffer_canary_pre = canary_value;
	char revision[5+1+2+1]; /* SIZE: 5 (major) + 1 (dot) + 1 (minor upper) + 1 (dot) + 1 (minor lower) */
	uint64_t revision_buffer_canary_post = canary_value;
	char *dotAddr = specializedShortToString(revision, ((ST->Hdr.Revision) & 0xFFFF0000) >> 16); /* X.y.z */
	*dotAddr++ = '.';
	*dotAddr++ = '0' + (((ST->Hdr.Revision % 0x100) / 10) % 10); /* x.Y.z*/
	if ((ST->Hdr.Revision % 0x100) % 10) *dotAddr++ = '.'; /* add dot in case of x.y.z */
	*dotAddr++ = '0' + ((ST->Hdr.Revision % 0x100) % 10); /* x.y.Z*/
	*dotAddr++ = 0;

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

	FILE *file;
	char *buff;
	long int size;
	file = fopen("\\file.bin", "r");
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	printf("File size: %d bytes\n", size);
	buff = malloc(size);
	fread(buff, size, 1, file);
	fclose(file);

	gregs_t regs;
	memset((void*)&regs, 0, sizeof(regs));
	regs.rax = (uint64_t)apitest;
	call_function_with_gregs((void*)buff, &regs);

	//((void(*)())buff)();


	printf("WERE OUTTA THE PROGRAM\n");

	while (true);
	return 0;
}

/*
 * The following code is licensed unfer CC BY-SA 2.5
 * Source: https://stackoverflow.com/a/784455/19418319
 * The name of the function have been modified without endorsement from the original source. The coding style has been modified without endorsement from the original source.
 * Written by GManNickG (https://stackoverflow.com/users/87234/gmannickg)
 * Modified by saltq144 on github (https://github.com/saltq144)
 * A function that reverses a string.
 * (c) GManNickG 2009
 */
void strrev(char *str) {
	/* skip null */
	if (str == 0) {
		return;
	}

	/* skip empty string */
	if (*str == 0) {
		return;
	}

	/* get range */
	char *start = str;
	char *end = start + strlen(str) - 1; /* -1 for \0 */
	char temp;

	/* reverse */
	while (end > start) {
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

char *specializedShortToString(char *buffer, short x) {
	char *nullAdr = buffer;
	while (x) {
		*nullAdr++ = '0' + (x % 10);
		x /= 10;
	}
	*nullAdr = 0;
	strrev(buffer);
	return nullAdr;
}
void apitest(void) {
	printf("apitest: called\n");
	return;
}

