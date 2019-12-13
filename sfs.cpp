#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
# pragma comment(lib, "Shlwapi.lib")
#else
# include <errno.h>
# include <unistd.h>
# include <sys/stat.h>
#endif

#include "sfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

bool sfs::exists(const char* _path, bool _dir)
{
#ifdef _MSC_VER
	const uint32_t attributes = ::GetFileAttributesA(_path);
	const bool isDir = attributes & FILE_ATTRIBUTE_DIRECTORY;

	return attributes != INVALID_FILE_ATTRIBUTES && (_dir ? isDir : true);
#else
	FILE* f = ::fopen(_path, "r");

	if (f != NULL)
	{
		::fclose(f);
		f = NULL;
		return true;
	}

	return false;
#endif
}

void sfs::makeFile(const char* _path)
{
	fclose(::fopen(_path, "w+"));
}

void sfs::removeFile(const char* _path)
{
	remove(_path);
}

void sfs::makeDir(const char* _path)
{
#ifdef _MSC_VER
	::CreateDirectoryA(_path, NULL);
#else
	::mkdir(_path, 0775);
#endif
}

void sfs::removeDir(const char* _path)
{
#ifdef _MSC_VER
	::RemoveDirectoryA(_path);
#else
	removeFile(_path);
#endif
}

sfs::OpenResult::Enum sfs::openFile(const char* _path, File* _file)
{
	if (_path == NULL || _file == NULL)
	{
		return OpenResult::NullError;
	}

	if (!exists(_path))
	{
		return OpenResult::NotExists;
	}

	::memset(_file, 0, sizeof(File));
	_file->stream = ::fopen(_path, "r+");

	if (_file->stream == NULL)
	{
		return OpenResult::UnknownError;
	}

#ifdef _MSC_VER
	::GetFullPathNameA(_path, SFS_MAX_PATH, _file->info.path, NULL);
	char* name = ::PathFindFileNameA(_file->info.path);
	::strcpy(_file->info.name, name);
#else
	// TODO: Does anyone know a multiplatform solution?
#endif

	::fseek((FILE*)_file->stream, 0l, SEEK_END);
	_file->info.size = ::ftell((FILE*)_file->stream);
	::fseek((FILE*)_file->stream, 0l, SEEK_SET);

	return OpenResult::Success;
}

void sfs::closeFile(File* _file)
{
	if (_file == NULL)
	{
		return;
	}

	if (_file->stream != NULL)
	{
		::fclose((FILE*)_file->stream);
	}

	::memset(_file, 0, sizeof(File));
}

sfs::ReadResult::Enum sfs::readFile(File* _file, char* _buffer, size_t _bufferSize)
{
	if (_file == NULL || _file->stream == NULL || _file->info.size == 0u || _buffer == NULL)
	{
		return ReadResult::NullOrEmptyError;
	}

	const size_t size = _bufferSize > 0u ? _bufferSize : _file->info.size;
	::memset(_buffer, 0, size);
	::rewind((FILE*)_file->stream);
	::fread(_buffer, sizeof(char), size, (FILE*)_file->stream);
	::fseek((FILE*)_file->stream, 0l, SEEK_END);

	return ReadResult::Success;
}

sfs::WriteResult::Enum sfs::writeFile(File* _file, const char* _buffer, bool append)
{
	if (_file == NULL || _file->stream == NULL || _buffer == NULL)
	{
		return WriteResult::NullOrEmptyError;
	}

	const size_t wlen = ::strlen(_buffer) + 1;
	const size_t len = append ? _file->info.size + wlen : wlen;

	if (append && _file->info.size > 0)
	{
		char* buffer = (char*)::malloc(len);
		::memset(buffer, 0, len);

		readFile(_file, buffer, _file->info.size);

		::strcat(buffer, _buffer);
		::freopen(_file->info.path, "w+", (FILE*)_file->stream);
		::fwrite(buffer, sizeof(char), len, (FILE*)_file->stream);
		::free(buffer);
	}
	else
	{
		::freopen(_file->info.path, "w+", (FILE*)_file->stream);
		::fwrite(_buffer, sizeof(char), wlen, (FILE*)_file->stream);
	}

	_file->info.size = len;

	return WriteResult::Success;
}

sfs::OpenResult::Enum sfs::openDir(const char* _path, Dir* _dir)
{
	if (_path == NULL || _dir == NULL)
	{
		return OpenResult::NullError;
	}

	if (!exists(_path, true))
	{
		return OpenResult::NotExists;
	}

	::memset(_dir, 0, sizeof(Dir));

	_dir->files = (File*)::calloc(SFS_MAX_FILES, sizeof(File));
	_dir->dirs = (Dir*)::calloc(SFS_MAX_FILES, sizeof(Dir));
	::GetFullPathNameA(_path, SFS_MAX_PATH, _dir->info.path, NULL);

	char* dirPath = (char*)::malloc(SFS_MAX_PATH);
	::strcpy(dirPath, _dir->info.path);
	::strcat(dirPath, "\\*.*");

#ifdef _MSC_VER
	WIN32_FIND_DATAA data;
	HANDLE file;
	if ((file = ::FindFirstFileA(dirPath, &data)) == INVALID_HANDLE_VALUE)
	{
		return OpenResult::UnknownError;
	}

	do
	{
		char path[SFS_MAX_PATH];
		::strcpy(path, _dir->info.path);
		::strcat(path, "\\");
		::strcat(path, data.cFileName);

		if (!::strcmp(data.cFileName, ".") || 
			!::strcmp(data.cFileName, "..") ||
			data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			continue;
		}

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			Dir* dir = &_dir->dirs[_dir->dirsCount];

			if (openDir(path, dir) == OpenResult::Success)
			{
				++_dir->dirsCount;
			}
		}
		else
		{
			File* file = &_dir->files[_dir->filesCount];

			if (openFile(path, file) == OpenResult::Success)
			{
				++_dir->filesCount;
			}
		}
	} while (::FindNextFileA(file, &data));
	
	::FindClose(file);

	if (_dir->filesCount < 1)
	{
		::free(_dir->files);
		_dir->files = NULL;
	}

	if (_dir->dirsCount < 1)
	{
		::free(_dir->dirs);
		_dir->dirs = NULL;
	}
#else
	// TODO: Does anyone know a multiplatform solution?
#endif

	::free(dirPath);

	return OpenResult::Success;
}

void sfs::closeDir(Dir* _dir)
{
	if (_dir == NULL)
	{
		return;
	}

	if (_dir->files != NULL)
	{
		while (_dir->filesCount--)
		{
			closeFile(&_dir->files[_dir->filesCount]);
		}

		::free(_dir->files);
	}

	if (_dir->dirs != NULL)
	{
		while (_dir->dirsCount--)
		{
			closeDir(&_dir->dirs[_dir->dirsCount]);
		}

		::free(_dir->dirs);
	}

	::memset(_dir, 0, sizeof(Dir));
}