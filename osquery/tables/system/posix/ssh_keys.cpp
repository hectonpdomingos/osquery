/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <string>
#include <vector>

#include <osquery/core.h>
#include <osquery/tables.h>
#include <osquery/filesystem.h>

#include "osquery/tables/system/system_utils.h"

namespace osquery {
namespace tables {

const std::string kSSHUserKeysDir = ".ssh/";

void genSSHkeyForHosts(const std::string& uid,
                       const std::string& directory,
                       QueryData& results) {
  // Get list of files in directory
  boost::filesystem::path keys_dir = directory;
  keys_dir /= kSSHUserKeysDir;
  std::vector<std::string> files_list;
  auto status = listFilesInDirectory(keys_dir, files_list, false);
  if (!status.ok()) {
    return;
  }
  // Go through each file
  for (const auto& kfile : files_list) {
    std::string keys_content;
    if (!forensicReadFile(kfile, keys_content).ok()) {
      // Cannot read a specific keys file.
      continue;
    }

    if (keys_content.find("PRIVATE KEY") != std::string::npos) {
      // File is private key, create record for it
      Row r;
      r["uid"] = uid;
      r["path"] = kfile;
      r["encrypted"] = INTEGER(0);

      // Check to see if the file is encrypted
      if (keys_content.find("ENCRYPTED") != std::string::npos) {
        r["encrypted"] = INTEGER(1);
      }
      results.push_back(r);
    }
  }
}

QueryData getUserSshKeys(QueryContext& context) {
  QueryData results;

  // Iterate over each user
  auto users = usersFromContext(context);
  for (const auto& row : users) {
    if (row.count("uid") > 0 && row.count("directory") > 0) {
      genSSHkeyForHosts(row.at("uid"), row.at("directory"), results);
    }
  }

  return results;
}
}
}
