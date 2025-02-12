// Copyright (c) 2002-present, OpenMS Inc. -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Chris Bielow $
// $Authors: Clemens Groepl, Chris Bielow $
// --------------------------------------------------------------------------

#include <OpenMS/CONCEPT/VersionInfo.h>

#include <OpenMS/DATASTRUCTURES/String.h>
#include <OpenMS/CONCEPT/Exception.h>

#include <fstream>

// these headers are generated by the build system
// and therefore intentionally break the naming convention (tagging them as automatically build)
#include <OpenMS/openms_package_version.h>

using namespace std;

namespace OpenMS
{
  const VersionInfo::VersionDetails VersionInfo::VersionDetails::EMPTY;

  bool VersionInfo::VersionDetails::operator<(const VersionInfo::VersionDetails & rhs) const
  {
    return (this->version_major < rhs.version_major)
           || (this->version_major == rhs.version_major && this->version_minor < rhs.version_minor)
           || (this->version_major == rhs.version_major && this->version_minor == rhs.version_minor && this->version_patch < rhs.version_patch)
           // note: if one version is pre-release, then it should compare as "less than"
           || (this->version_major == rhs.version_major && this->version_minor == rhs.version_minor && this->version_patch == rhs.version_patch && 
                (!this->pre_release_identifier.empty() && rhs.pre_release_identifier.empty()) );
  }

  bool VersionInfo::VersionDetails::operator==(const VersionInfo::VersionDetails & rhs) const
  {
    return this->version_major == rhs.version_major &&
      this->version_minor == rhs.version_minor &&
      this->version_patch == rhs.version_patch &&
      this->pre_release_identifier == rhs.pre_release_identifier;
  }

  bool VersionInfo::VersionDetails::operator!=(const VersionInfo::VersionDetails & rhs) const
  {
    return !(this->operator==(rhs));
  }

  bool VersionInfo::VersionDetails::operator>(const VersionInfo::VersionDetails & rhs) const
  {
    return !(*this < rhs || *this == rhs);
  }

  VersionInfo::VersionDetails VersionInfo::VersionDetails::create(const String & version) //static
  {
    VersionInfo::VersionDetails result;

    size_t first_dot = version.find('.');
    // we demand at least one "."
    if (first_dot == string::npos)
    {
      return VersionInfo::VersionDetails::EMPTY;
    }

    try
    {
      result.version_major = String(version.substr(0, first_dot)).toInt();
    }
    catch (Exception::ConversionError & /*e*/)
    {
      return VersionInfo::VersionDetails::EMPTY;
    }

    // returns npos if no second "." is found - which does not hurt
    size_t second_dot = version.find('.', first_dot + 1);
    try
    {
      result.version_minor = String(version.substr(first_dot + 1, second_dot - (first_dot + 1))).toInt();
    }
    catch (Exception::ConversionError & /*e*/)
    {
      return VersionInfo::VersionDetails::EMPTY;
    }

    // if there is no second dot: return
    if (second_dot == string::npos)
    {
      return result;
    }

    // returns npos if no final pre-release dash "-" is found - which does not hurt
    size_t pre_release_dash = version.find('-', second_dot + 1);
    try
    {
      result.version_patch = String(version.substr(second_dot + 1, pre_release_dash - (second_dot + 1))).toInt();
    }
    catch (Exception::ConversionError & /*e*/)
    {
      return VersionInfo::VersionDetails::EMPTY;
    }

    if (pre_release_dash == string::npos)
    {
      return result;
    }

    result.pre_release_identifier = String(version.substr(pre_release_dash + 1, version.size() - (pre_release_dash + 1)));

    return result;
  }

  String VersionInfo::getTime()
  {
    static bool is_initialized = false;
    static String result;
    if (!is_initialized)
    {
      result = String(__DATE__) + ", " + __TIME__;
      is_initialized = true;
    }
    return result;
  }

  String VersionInfo::getVersion()
  {
    static bool is_initialized = false;
    static String result;
    if (!is_initialized)
    {
      result = OPENMS_PACKAGE_VERSION;
      result.trim();
      is_initialized = true;
    }
    return result;
  }

  VersionInfo::VersionDetails VersionInfo::getVersionStruct()
  {
    static bool is_initialized = false;
    static VersionDetails result;
    if (!is_initialized)
    {
      result = VersionDetails::create(getVersion());
      is_initialized = true;
    }
    return result;
  }

  String VersionInfo::getRevision()
  {
    return String(OPENMS_GIT_SHA1);
  }

  String VersionInfo::getBranch()
  {
    return String(OPENMS_GIT_BRANCH);
  }

}
