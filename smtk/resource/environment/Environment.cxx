//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/environment/Environment.h"

#include "smtk/resource/RegisterOperations.h"
#include "smtk/resource/RegisterResources.h"
#include "smtk/resource/environment/Environment.h"

namespace
{
static unsigned int registerToEnvironmentCounter = 0;
}

namespace smtk
{
namespace resource
{
namespace environment
{
RegisterToEnvironment::RegisterToEnvironment()
{
  if (registerToEnvironmentCounter++ == 0)
  {
    registerOperations(smtk::environment::OperationManager::instance());
    registerResources(smtk::environment::ResourceManager::instance());
  }
}

RegisterToEnvironment::~RegisterToEnvironment()
{
  if (--registerToEnvironmentCounter == 0)
  {
    unregisterOperations(smtk::environment::OperationManager::instance());
    unregisterResources(smtk::environment::ResourceManager::instance());
  }
}
}
}
}