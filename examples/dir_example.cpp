#include <stdio.h>
#include "sfs.h"

int main()
{
	::printf("SFS v%s | dir example\n\n", SFS_VERSION);

	if (!sfs::exists("sfs_dir", true))
	{
		sfs::makeDir("sfs_dir");
		sfs::makeDir("sfs_dir\\sfs_child_dir");

		sfs::makeFile("sfs_dir\\sfs_0.txt");
		sfs::makeFile("sfs_dir\\sfs_1.txt");
		sfs::makeFile("sfs_dir\\sfs_2.txt");
	}

	sfs::removeFile("sfs_dir\\sfs_2.txt");

	sfs::Dir dir;
	int result;

	::printf("Try sfs::openDir:\n\tPath: 'sfs_dir'\n\n");

	if ((result = sfs::openDir("sfs_dir", &dir)) != sfs::OpenResult::Success)
	{
		printf("sfs::openDir failed! code: %i", result);
		return -1;
	}

	::printf("Successful.\n\n");

	::printf("%s:\n", dir.info.path);

	for (int i = 0; i < dir.filesCount; i++)
	{
		sfs::File* f = &dir.files[i];

		::printf("'- [F] %s (%i)\n", f->info.path, f->info.size);
	}

	for (int i = 0; i < dir.dirsCount; i++)
	{
		sfs::Dir* d = &dir.dirs[i];

		::printf("'- [D] %s (%i)\n", d->info.path, d->info.size);

		for (int i = 0; i < d->filesCount; i++)
		{
			sfs::File* f = &d->files[i];

			::printf("'-- [F] %s (%i)\n", f->info.path, f->info.size);
		}

		for (int i = 0; i < d->dirsCount; i++)
		{
			d = &d->dirs[i];

			::printf("'-- [D] %s (%i)\n", d->info.path, d->info.size);
		}
	}

	sfs::closeDir(&dir);

	return 0;
}