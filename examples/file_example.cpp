#if 0

#include <stdio.h>
#include "sfs.h"

int main()
{
	::printf("SFS v%s | file example\n\n", SFS_VERSION);

	if (!sfs::exists("sfs.txt"))
	{
		sfs::makeFile("sfs.txt");
	}

	sfs::File file;
	int result;

	::printf("Try sfs::openFile:\n\tPath: 'sfs.txt'\n\n");

	if ((result = sfs::openFile("sfs.txt", &file)) != sfs::OpenResult::Success)
	{
		::printf("sfs::openFile failed! code: %i", result);
		return -1;
	}

	::printf("Successful.\n\nPath: %s\nSize: %i\n\n", file.info.path, file.info.size);
	::printf("Try sfs::writeFile:\n\tString: 'Hello'\n\tAppend: false\n\n");

	if ((result = sfs::writeFile(&file, "Hello")) != sfs::WriteResult::Success)
	{
		::printf("sfs::writeFile failed! code: %i", result);
		sfs::closeFile(&file);
		return -2;
	}

	::printf("Successful.\n\n");

	::printf("Try sfs::writeFile:\n\tString: ', sfs!'\n\tAppend: true\n\n");

	if ((result = sfs::writeFile(&file, ", sfs!", true)) != sfs::WriteResult::Success)
	{
		::printf("sfs::writeFile failed! code: %i", result);
		sfs::closeFile(&file);
		return -2;
	}

	::printf("Successful.\n\n");

	char* buffer = (char*)::malloc(file.info.size);

	::printf("Try sfs::readFile: no params\n\n");

	if ((result = sfs::readFile(&file, buffer) != sfs::ReadResult::Success))
	{
		::printf("sfs::readFile failed! code: %i", result);
		::free(buffer);
		sfs::closeFile(&file);
		return -3;
	}

	::printf("Successful.\n\nText: %s\n\n", buffer);

	::free(buffer);
	sfs::closeFile(&file);

	return 0;
}

#endif