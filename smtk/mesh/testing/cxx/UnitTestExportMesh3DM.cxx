//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"
#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/ReadMesh.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}

void verify_write_empty_collection()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".3dm";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  test(c->isValid(), "empty collection is empty");

  const bool result = smtk::io::exportMesh(write_path, c);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);
  test(result == false, "nothing to write for an empty collection");
}

void verify_write_null_collection()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".3dm";

  //use a null collection ptr
  smtk::mesh::CollectionPtr c;

  const bool result = smtk::io::exportMesh(write_path, c);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);

  test(result == false, "Can't save null collection to disk");
}

void verify_write_valid_collection()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".3dm";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::readMesh(file_path, manager);
  test(c->isValid(), "collection should be valid");

  //export the volume elements
  const bool result = smtk::io::exportMesh(write_path, c);
  cleanup(write_path);

  if (!result)
  {
    test(result == true, "failed to properly write out a valid 3dm file");
  }
}
}

int UnitTestExportMesh3DM(int, char** const)
{
  verify_write_empty_collection();
  verify_write_null_collection();
  verify_write_valid_collection();

  return 0;
}
