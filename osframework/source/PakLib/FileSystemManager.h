#ifndef __PAKLIB_FILESYSTEM_MANAGER_H__
#define __PAKLIB_FILESYSTEM_MANAGER_H__

#include "FileSystemDriverFactory.h"

namespace PakLib {

	class FileSystemManager: FileSystem
	{
	private:
		FileSystemManager();
		~FileSystemManager();

	public:
		static FileSystemManager*       getManager();

	public:
		bool                            addResource(const std::string &location,
							    const std::string &type,
							    int priority);

	public:
		virtual File*			open(const char* theFileName,
						     const char* theAccess);
		virtual File*			open(const wchar_t* theFileName,
						     const wchar_t* theAccess);

		virtual Dir*                    openDir(const char *theDir);

	private:

		void                            addDefaultLocations();

		struct FileSystemCompare
		{
			bool operator() (FileSystem* const & lhs,
					 FileSystem* const & rhs) const
			{
				return lhs->getPriority() < rhs->getPriority();
			}
		};
		typedef std::multiset<FileSystem*, FileSystemCompare> FileSystemList;

		bool                            mInitialized;
		bool                            mLoaded;
		FileSystemDriverFactory         mFactory;
		FileSystemList                  mFileSystems;
	};

}

#endif
