


#ifndef UTIL_FS_H
#define UTIL_FS_H


namespace UTIL
{
namespace FS
{
	// gets a file size in bytes
	uint64 UTIL_getFileSize(const char* file);

	// makes all folders along path
	void UTIL_recMakeFolder(const char* path );

	// makes one folder
	void UTIL_makeFolder(const char* path );

	// read all data from a file. Size is the len of buffer and will be updated to be the len of the file.
	uint32 UTIL_readWholeFile(const char* file, char** buf);

	//see if file is on drive and has size greater than zero
	bool UTIL_isValidFile(const char* file);

	//see if folder is on drive
	bool UTIL_isValidFolder(const char* folder);

	//removes a file from the os
	void UTIL_delFile(const char* file);

	//removes a folder and its contents from the os
	void UTIL_delFolder(const char* path);

	//recurivly removes empty folders from the path
	void UTIL_delEmptyFolders(const char* path);

	//see if a folder has any files in it.
	bool UTIL_isFolderEmpty(const char* path);


	enum
	{
		FILE_READ,
		FILE_WRITE,
		FILE_APPEND,
	};

	class FileHandle
	{
	public:
		FileHandle();
		FileHandle(const char* fileName, uint8 mode);
		~FileHandle();

		void open(const char* fileName, uint8 mode);
		void close();

		void read(char* buff, uint32 size);
		void write(const char* buff, uint32 size);
		void seek(uint64 pos);

#ifdef WIN32
		HANDLE getHandle(){return m_hFileHandle;}
#else
		FILE* getHandle(){return m_hFileHandle;}
#endif		

		bool isValidFile(){return (m_bIsOpen && m_hFileHandle);}

	private:
#ifdef WIN32
		HANDLE m_hFileHandle;
#else
		FILE* m_hFileHandle;
#endif
		bool m_bIsOpen;
	};
}
}

#endif
