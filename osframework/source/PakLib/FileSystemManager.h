#ifndef __PAKLIB_FILESYSTEM_MANAGER_H__
#define __PAKLIB_FILESYSTEM_MANAGER_H__

#include "FileSystemDriverFactory.h"
#include "CritSect.h"

namespace PakLib {

	class FileSystemManager: FileSystem
	{
		friend class File;
	private:
		FileSystemManager();
		~FileSystemManager();

	public:
		static FileSystemManager*       getManager();

	public:
		bool                            addResource(const std::string &location,
							    const std::string &type,
							    int priority);

		File*                           getFile(int id);

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);

		virtual Dir*                    openDir(const char *theDir);

	private:

		void                            addDefaultLocations();

		int                             addFile(File* theFile);
		void                            removeFile(File* theFile);

		struct FileSystemCompare
		{
			bool operator() (FileSystem* const & lhs,
					 FileSystem* const & rhs) const
			{
				return lhs->getPriority() < rhs->getPriority();
			}
		};
		typedef std::multiset<FileSystem*, FileSystemCompare> FileSystemList;
		typedef std::map<int, File*>    FileIdMap;

		bool                            mInitialized;
		bool                            mLoaded;
		FileSystemDriverFactory         mFactory;
		FileSystemList                  mFileSystems;
		int                             mFileNextId;
		FileIdMap                       mFileIdMap;
		CritSect                        mCritSect;
		CritSect                        mInitCritSect;
	};

}

#endif
