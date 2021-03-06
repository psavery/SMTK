//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Session.h"

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/operation/Manager.h"

#include <complex>

namespace
{
static const double tolerance = 1.e-5;
}

int UnitTestPolygonCreateModel(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::bridge::polygon::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register operators to the operation manager
  {
    operationManager->registerOperation<smtk::bridge::polygon::CreateModel>(
      "smtk::bridge::polygon::CreateModel");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  double originCoords[] = { 1., 2., 3. };
  double vx[] = { 0.707107, 0.707107, 0. };
  double vy[] = { -0.707107, 0.707107, 0. };
  {
    // Create an "create model" operator
    smtk::bridge::polygon::CreateModel::Ptr createOp =
      operationManager->create<smtk::bridge::polygon::CreateModel>();
    if (!createOp)
    {
      std::cerr << "No create operator\n";
      return 1;
    }

    // Specify model name, origin, x axis, y axis and feature size
    smtk::attribute::StringItemPtr name = createOp->parameters()->findString("name");
    test(name != nullptr, "Name attribute not found");
    name->setIsEnabled(true);
    test(name->setValue("my model"), "Could not set name attribute");

    smtk::attribute::IntItemPtr constructionMethod =
      createOp->parameters()->findInt("construction method");
    test(constructionMethod != nullptr, "Construction method not found");
    test(constructionMethod->setValue(0), "Could not set construction method");
    int numActiveChildren = static_cast<int>(constructionMethod->numberOfActiveChildrenItems());
    std::cout << "Available active children items: " << std::endl;
    for (int i = 0; i < numActiveChildren; ++i)
    {
      smtk::attribute::ItemPtr item = constructionMethod->activeChildItem(i);
      std::cout << item->name() << std::endl;
    }

    smtk::attribute::ItemPtr origin =
      constructionMethod->findChild("origin", smtk::attribute::SearchStyle::ACTIVE_CHILDREN);
    test(origin != nullptr, "Origin not found");
    for (int i = 0; i < 3; ++i)
    {
      test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(origin)->setValue(
             i, originCoords[i]),
        "Setting origin failed");
    }

    smtk::attribute::ItemPtr xAxis =
      constructionMethod->findChild("x axis", smtk::attribute::SearchStyle::ACTIVE_CHILDREN);
    test(xAxis != nullptr, "x axis not found");
    for (int i = 0; i < 3; ++i)
    {
      test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(xAxis)->setValue(i, vx[i]),
        "Setting x axis failed");
    }

    smtk::attribute::ItemPtr yAxis =
      constructionMethod->findChild("y axis", smtk::attribute::SearchStyle::ACTIVE_CHILDREN);
    test(yAxis != nullptr, "y axis not found");
    for (int i = 0; i < 3; ++i)
    {
      test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(yAxis)->setValue(i, vy[i]),
        "Setting y axis failed");
    }

    smtk::attribute::ItemPtr featureSize =
      constructionMethod->findChild("feature size", smtk::attribute::SearchStyle::ACTIVE_CHILDREN);
    test(featureSize != nullptr, "feature size not found");
    test(smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(featureSize)->setValue(1.),
      "Setting feature size failed");

    // Apply the operation and check the result
    smtk::operation::Operation::Result createOpResult = createOp->operate();

    test(createOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
      "Create operator failed");

    // Retrieve the resulting model item
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        createOpResult->findComponent("model"));

    // Access the generated model
    model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());
  }

  {
    test(model != nullptr, "Could not find the created model");
    smtk::model::EntityRef myModel = model->referenceAs<smtk::model::Model>();

    // Verify the name, origin, x axis, y axis, feature size
    std::cout << "Model name: " << myModel.name() << std::endl;
    test(myModel.name() == "my model", "Incorrect model name");

    std::cout << "Model origin: ";
    smtk::model::FloatList modelOrigin = myModel.floatProperty("origin");
    for (int i = 0; i < 3; ++i)
    {
      std::cout << modelOrigin[i] << " ";
      test(std::abs(modelOrigin[i] - originCoords[i]) < tolerance, "Incorrect model origin");
    }
    std::cout << "\nModel x axis: ";
    smtk::model::FloatList modelXAxis = myModel.floatProperty("x axis");
    for (int i = 0; i < 3; ++i)
    {
      std::cout << modelXAxis[i] << " ";
      test(std::abs(modelXAxis[i] - vx[i]) < tolerance, "Incorrect model x axis");
    }
    std::cout << "\nModel y axis: ";
    smtk::model::FloatList modelYAxis = myModel.floatProperty("y axis");
    for (int i = 0; i < 3; ++i)
    {
      std::cout << modelYAxis[i] << " ";
      test(std::abs(modelYAxis[i] - vy[i]) < tolerance, "Incorrect model y axis");
    }
    std::cout << std::endl;
  }

  return 0;
}
