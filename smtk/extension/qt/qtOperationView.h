//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperationView - UI components for attribute Operation View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtOperationView_h
#define __smtk_extension_qtOperationView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QPointer>

class qtOperationViewInternals;
class QPushButton;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT OperationViewInfo : public ViewInfo
{
public:
  OperationViewInfo(smtk::view::ViewPtr view, smtk::operation::OperationPtr targetOperation,
    QWidget* parent, qtUIManager* uiman)
    : ViewInfo(view, parent, uiman)
    , m_operator(targetOperation)
  {
  }

  OperationViewInfo(smtk::view::ViewPtr view, smtk::operation::OperationPtr targetOperation,
    QWidget* parent, qtUIManager* uiman, const std::map<std::string, QLayout*>& layoutDict)
    : ViewInfo(view, parent, uiman, layoutDict)
    , m_operator(targetOperation)
  {
  }

  OperationViewInfo() {}
  smtk::operation::OperationPtr m_operator;
};

class SMTKQTEXT_EXPORT qtOperationView : public qtBaseView
{
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const ViewInfo& info);

  qtOperationView(const OperationViewInfo& info);
  virtual ~qtOperationView();

  QPointer<QPushButton> applyButton() const;
  smtk::operation::OperationPtr operation() const;

public slots:
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  virtual void onModifiedParameters();
  virtual void onOperate();

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);

protected:
  void createWidget() override;
  void setInfoToBeDisplayed() override;
  bool m_applied; // indicates if the current settings have been applied

private:
  qtOperationViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
