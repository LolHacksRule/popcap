#ifndef __DRIVERFACTORY_H__
#define __DRIVERFACTORY_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy {
	class Driver
	{
	public:
		std::string mName;
		int	    mPriority;

		Driver (const std::string theName,
			int	       thePriority = 0);
		virtual ~Driver ();

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

	class SEXY_EXPORT DriverFactory
	{
	public:
		typedef std::multiset<Driver*, DriverCompare> Drivers;

		void		       AddDriver (Driver * theDriver);
		void		       RemoveDriver (Driver * theDriver);

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
