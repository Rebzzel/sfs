#ifndef __SFS_H__
#define __SFS_H__

#include <stdint.h>

#define SFS_VERSION "1.1.1"

#ifdef _MSC_VER
# include <Windows.h>
# include <shlwapi.h>
# define SFS_MAX_PATH MAX_PATH
#endif

#ifndef SFS_MAX_PATH
# define SFS_MAX_PATH 4096
#endif

#ifndef SFS_MAX_FILENAME
# define SFS_MAX_FILENAME 260
#endif

#ifndef SFS_MAX_FILES
# define SFS_MAX_FILES 128
#endif

namespace sfs
{
	struct FileInfo
	{
		char path[SFS_MAX_PATH];
		char name[SFS_MAX_FILENAME];
		size_t size;
	};

	struct File
	{
		void* stream;
		FileInfo info;
	};

	typedef FileInfo DirInfo;

	struct Dir
	{
		File* files;
		size_t filesCount;
		Dir* dirs;
		size_t dirsCount;
		DirInfo info;
	};

	struct OpenResult
	{
		enum Enum
		{
			UnknownError = -1,
			NullError = -2,
			NotExists = -3,
			Success = 0,
		};
	};

	struct ReadResult
	{
		enum Enum
		{
			UnknownError = -1,
			NullOrEmptyError = -2,
			Success = 0
		};
	};

	struct WriteResult
	{
		enum Enum
		{
			UnknownError = -1,
			NullOrEmptyError = -2,
			Success = 0
		};
	};

	bool exists(const char* path, bool dir = false);

	void makeFile(const char* path);
	void removeFile(const char* path);

	void makeDir(const char* path);
	void removeDir(const char* path);

	OpenResult::Enum openFile(const char* path, File* file);
	void closeFile(File* file);

	ReadResult::Enum readFile(File* file, char* buffer, size_t bufferSize = 0u);
	WriteResult::Enum writeFile(File* file, const char* buffer, bool append = false);

	OpenResult::Enum openDir(const char* path, Dir* dir);
	void closeDir(Dir* dir);
}

#endif // __SFS_H__