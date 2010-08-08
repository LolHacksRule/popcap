#ifndef __PAK_LIB_DRIVERFACTORY_H__
#define __PAK_LIB_DRIVERFACTORY_H__

#include <string>
#include <set>

namespace PakLib {
	class Driver
	{
	public:
		std::string mName;
		int	    mPriority;

		Driver (const std::string theName,
			int	       thePriority = 0);
		virtual ~Driver ();

		std::string GetName() const;
		int         GetPriority() const;

		bool operator< (const Driver& other) const
		{
			return mPriority < other.mPriority;
		}
	};

	struct DriverCompare
	{
		bool operator() (Driver* const & lhs, Driver* const & rhs) const
		{
			return *lhs < *rhs;
		}
	};

	class DriverFactory
	{
	public:
		typedef std::multiset<Driver*, DriverCompare> Drivers;

		void		       AddDriver (Driver * theDriver);
		void		       RemoveDriver (Driver * theDriver,
						     bool     freeIt = true);

		Driver*		       Find (const std::string name = "auto");
		Driver*		       FindNext (Driver * theDriver);

	public:
		const Drivers*         GetDrivers();

	private:
		Drivers		       mDrivers;

	public:
		DriverFactory ();
		~DriverFactory ();
	};

}

#endif
