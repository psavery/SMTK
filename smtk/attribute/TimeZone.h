//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_TimeZone_h
#define __smtk_attribute_TimeZone_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/CompilerInformation.h"

#ifndef SHIBOKEN_SKIP
SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/date_time/local_time/local_time.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
#endif

#include <string>

namespace smtk {
  namespace attribute {

//.NAME TimeZone - Representation for time zones
//.SECTION Description
// A minimal wrapper for boost::posix_time::posix_time_zone
class SMTKCORE_EXPORT TimeZone
{
 public:
  TimeZone();

  bool isSet() const;

  // static std::vector<std::string> regions();

  /// Set timezone via region name (table lookup)
  bool setRegion(const std::string& region);
  std::string region() const;

  /// Set timezone to posix-formatted string
  bool setPosixString(const std::string& posixTimeZoneString);
  std::string posixString() const;

  std::string stdZoneName() const;
  std::string stdZoneAbbreviation() const;
  std::string dstZoneName() const;
  std::string dstZoneAbbreviation() const;

  bool hasDST() const;
  bool utcOffset(int& hours, int& minutes) const;
  bool dstShift(int& hours, int& minutes) const;

#ifndef SHIBOKEN_SKIP
  // Intended for internal use
  const boost::local_time::time_zone_ptr boostPointer() const;
#endif
 protected:
  boost::local_time::time_zone_ptr m_boostTimeZone;
  std::string m_region;

  // Static timezone database
  static boost::local_time::tz_database s_database;
  static bool s_databaseLoaded;
};

  } // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_TimeZone_h
