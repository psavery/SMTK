//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtCategorySelectorView.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtInputsItem.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtModelEntityAttributeView.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtSelectorView.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/io/Logger.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QToolButton>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/view/View.h"

#include <math.h>

using namespace smtk::attribute;
using namespace smtk::extension;

qtTextEdit::qtTextEdit(QWidget* inParent)
  : QTextEdit(inParent)
{
}

QSize qtTextEdit::sizeHint() const
{
  return QSize(200, 70);
}

qtUIManager::qtUIManager(smtk::attribute::ResourcePtr resource)
  : m_parentWidget(nullptr)
  , m_attResource(resource)
  , m_useInternalFileBrowser(false)
{
  this->commonConstructor();
}

qtUIManager::qtUIManager(smtk::operation::OperationPtr op)
  : m_parentWidget(nullptr)
  , m_operation(op)
{
  if (!op)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Asked to create an operation view with no operation.");
  }

  auto spec = op->specification();
  if (!spec)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Asked to create an operation view of "
        << op->index() << " (" << op->typeName() << ")"
        << " but operator has no specification.");
  }
  m_attResource = spec;

  this->commonConstructor();
}

void qtUIManager::commonConstructor()
{
  m_resourceManager = m_attResource ? m_attResource->manager() : nullptr;
  m_useInternalFileBrowser = true;
  m_topView = nullptr;
  m_activeModelView = nullptr;
  m_maxValueLabelLength = 200;
  m_minValueLabelLength = 50;

  // default settings
  this->advFont.setBold(true);
  this->advFont.setItalic(false);
  this->DefaultValueColor.setRgbF(1.0, 1.0, 0.5);
  this->InvalidValueColor.setRgbF(1.0, 0.5, 0.5);

  m_currentAdvLevel = 0;

  // Lets register some basic view constructors
  this->registerViewConstructor("Attribute", qtAttributeView::createViewWidget);
  this->registerViewConstructor("Group", qtGroupView::createViewWidget);
  this->registerViewConstructor("Instanced", qtInstancedView::createViewWidget);
  this->registerViewConstructor("Operation", qtOperationView::createViewWidget);
  this->registerViewConstructor("Selector", qtSelectorView::createViewWidget);
  this->registerViewConstructor("SimpleExpression", qtSimpleExpressionView::createViewWidget);
  this->registerViewConstructor("Category", qtCategorySelectorView::createViewWidget);
  this->registerViewConstructor("ModelEntity", qtModelEntityAttributeView::createViewWidget);

  // register view constructors coming from plugins.
  qtSMTKUtilities::updateViewConstructors(this);
}

qtUIManager::~qtUIManager()
{
  if (m_topView)
  {
    delete m_topView;
  }
}

void qtUIManager::initializeUI(QWidget* pWidget, bool useInternalFileBrowser)
{
  m_useInternalFileBrowser = useInternalFileBrowser;
  m_parentWidget = pWidget;
  if (m_topView)
  {
    delete m_topView;
    m_topView = NULL;
  }

  if (!m_smtkView)
  {
    return;
  }
  this->internalInitialize();

  if (!m_operation)
  {
    smtk::extension::ViewInfo vinfo(m_smtkView, pWidget, this);
    m_topView = this->createView(vinfo);
  }
  else
  {
    smtk::extension::OperationViewInfo vinfo(m_smtkView, m_operation, pWidget, this);
    m_topView = this->createView(vinfo);
  }
  if (m_topView)
  {
    if (m_currentAdvLevel) // only build advanced level when needed)
    {
      m_topView->showAdvanceLevel(m_currentAdvLevel);
    }
    // In case we are filtering by categories by default or
    // its set on permanently we need to  have the top level view
    // initially set its category.  The reason is because the UIManager's
    // currentCategory method is used to determine the current category but at
    // qtView construction time the UIManager's TopLevel View is not currently set
    // resulting in the current category not being set.  This method allows the UI
    // Manager to have the current category being set.
    m_topView->setInitialCategory();
  }
}

void qtUIManager::initializeUI(
  const smtk::extension::ViewInfo& viewInfo, bool useInternalFileBrowser)
{
  m_useInternalFileBrowser = useInternalFileBrowser;
  m_parentWidget = viewInfo.m_parent;
  if (m_topView)
  {
    delete m_topView;
    m_topView = NULL;
  }

  if (!m_smtkView)
  {
    return;
  }
  this->internalInitialize();

  m_topView = this->createView(viewInfo);
  if (m_topView && m_currentAdvLevel) // only build advanced level when needed
  {
    m_topView->showAdvanceLevel(m_currentAdvLevel);
  }
  m_topView->setInitialCategory();
}

smtk::view::ViewPtr qtUIManager::findOrCreateOperationView() const
{
  smtk::view::ViewPtr view;
  auto op = m_operation;
  if (!op)
  {
    return view;
  }

  smtk::attribute::ResourcePtr rsrc = op->specification();
  if (!rsrc)
  {
    return view;
  }

  smtk::attribute::AttributePtr params = op->parameters();

  // See if the operation has a manually-specified view.
  for (auto it = rsrc->views().begin(); it != rsrc->views().end(); ++it)
  {
    // If this is an "Operation" view we need to check its InstancedAttributes child,
    // otherwise we need to check AttributeTypes:
    int i; // View Component index we need to check
    if (it->second->type() == "Operation")
    {
      i = it->second->details().findChild("InstancedAttributes");
    }
    else
    {
      i = it->second->details().findChild("AttributeTypes");
    }
    if (i < 0)
    {
      continue;
    }
    smtk::view::View::Component& comp = it->second->details().child(i);
    for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
      std::string optype;
      if (comp.child(ci).attribute("Type", optype) && optype == params->type())
      {
        view = it->second;
        // If we are dealing with an Operation View - The Name attribute needs to be
        // set to the same of the attribute the operator is using - so in practice
        // the Name attribute does not have to be set in the operator's sbt info
        if (view->type() == "Operation")
        {
          comp.child(ci).setAttribute("Name", params->name());
        }
        break;
      }
    }
    if (view && this->hasViewConstructor(view->type()))
    {
      break;
    }
  }

  // There is no view or there is a view but the UI manager does
  // not know about it (perhaps because the plugin which should
  // register widgets for the view is not loaded, perhaps because
  // the view name is incorrect).
  if (!view || !this->hasViewConstructor(view->type()))
  { // Create an "Operation" view.
    if (op)
    {
      view = smtk::view::View::New("Operation", op->typeName());
      view->details().setAttribute("UseSelectionManager", "true");
      smtk::view::View::Component& comp =
        view->details().addChild("InstancedAttributes").addChild("Att");
      comp.setAttribute("Type", params->type()).setAttribute("Name", params->name());
      rsrc->addView(view);
    }
  }

  view->details().setAttribute("TopLevel", "true");
  return view;
}

qtBaseView* qtUIManager::setSMTKView(
  const smtk::extension::ViewInfo& viewInfo, bool useInternalFileBrowser)
{
  if ((m_smtkView == viewInfo.m_view) && (m_parentWidget == viewInfo.m_parent) &&
    (m_useInternalFileBrowser == useInternalFileBrowser))
  {
    return m_topView;
  }
  m_smtkView = viewInfo.m_view;
  this->initializeUI(viewInfo, m_useInternalFileBrowser);
  return m_topView;
}

qtBaseView* qtUIManager::setSMTKView(smtk::view::ViewPtr v)
{
  if (m_smtkView != v)
  {
    m_smtkView = v;
    this->initializeUI(m_parentWidget, m_useInternalFileBrowser);
  }
  return m_topView;
}

qtBaseView* qtUIManager::setSMTKView(
  smtk::view::ViewPtr v, QWidget* pWidget, bool useInternalFileBrowser)
{
  if ((m_smtkView != v) || (m_parentWidget != pWidget) ||
    (m_useInternalFileBrowser != useInternalFileBrowser))
  {
    m_smtkView = v;
    this->initializeUI(pWidget, useInternalFileBrowser);
  }
  return m_topView;
}

void qtUIManager::setActiveModelView(smtk::extension::qtModelView* mv)
{
  if (m_activeModelView != mv)
  {
    m_activeModelView = mv;
  }
}

smtk::extension::qtModelView* qtUIManager::activeModelView()
{
  return m_activeModelView;
}

void qtUIManager::internalInitialize()
{
  this->findDefinitionsLongLabels();

  // initialize initial advance level
  const std::map<int, std::string>& levels = m_attResource->advanceLevels();
  if (levels.size() > 0)
  {
    // use the minimum enum value as initial advance level
    std::map<int, std::string>::const_iterator ait = levels.begin();
    int minLevel = ait->first;
    ait++;
    for (; ait != levels.end(); ++ait)
    {
      minLevel = std::min(minLevel, ait->first);
    }
    // m_currentAdvLevel can not be lower than the minLevel
    m_currentAdvLevel = std::max(minLevel, m_currentAdvLevel);
  }
}

void qtUIManager::setAdvanceLevel(int b)
{
  if (m_currentAdvLevel == b)
  {
    return;
  }

  m_currentAdvLevel = b;
  if (m_topView)
  {
    m_topView->showAdvanceLevel(b);
  }
}

void qtUIManager::initAdvanceLevels(QComboBox* combo)
{
  combo->blockSignals(true);
  const std::map<int, std::string>& levels = m_attResource->advanceLevels();
  if (levels.size() == 0)
  {
    // for backward compatibility, we automatically add
    // two levels which is implicitly supported in previous version
    combo->addItem("General", 0);
    combo->addItem("Advanced", 1);

    combo->setCurrentIndex(m_currentAdvLevel);
  }
  else
  {
    std::map<int, std::string>::const_iterator ait;
    for (ait = levels.begin(); ait != levels.end(); ++ait)
    {
      combo->addItem(ait->second.c_str(), ait->first);
      if (m_currentAdvLevel == ait->first)
      {
        combo->setCurrentIndex(combo->count() - 1);
      }
    }
  }
  combo->blockSignals(false);
}

void qtUIManager::updateModelViews()
{
  if (!m_topView)
  {
    return;
  }
  m_topView->updateModelAssociation();
}

std::string qtUIManager::currentCategory()
{
  return m_topView ? m_topView->currentCategory() : "";
}

bool qtUIManager::categoryEnabled()
{
  return m_topView ? m_topView->categoryEnabled() : false;
}

bool qtUIManager::passAdvancedCheck(int level)
{
  return (level <= this->advanceLevel());
}

bool qtUIManager::passAttributeCategoryCheck(smtk::attribute::ConstDefinitionPtr AttDef)
{
  return this->passCategoryCheck(AttDef->categories());
}

bool qtUIManager::passItemCategoryCheck(smtk::attribute::ConstItemDefinitionPtr ItemDef)
{
  return this->passCategoryCheck(ItemDef->categories());
}

bool qtUIManager::passCategoryCheck(const std::set<std::string>& categories)
{
  return !this->categoryEnabled() || categories.find(this->currentCategory()) != categories.end();
}

QString qtUIManager::clipBoardText()
{
  const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
  return clipboard->text();
}

void qtUIManager::setClipBoardText(QString& text)
{
  QApplication::clipboard()->setText(text);
}

void qtUIManager::clearRoot()
{
  if (m_topView)
  {
    delete m_topView;
    m_topView = NULL;
  }
}

void qtUIManager::setAdvanceFontStyleBold(bool val)
{
  this->advFont.setBold(val);
}

bool qtUIManager::advanceFontStyleBold() const
{
  return this->advFont.bold();
}

void qtUIManager::setAdvanceFontStyleItalic(bool val)
{
  this->advFont.setItalic(val);
}

bool qtUIManager::advanceFontStyleItalic() const
{
  return this->advFont.italic();
}

void qtUIManager::setDefaultValueColor(const QColor& color)
{
  this->DefaultValueColor = color;
}

void qtUIManager::setInvalidValueColor(const QColor& color)
{
  this->InvalidValueColor = color;
}

void qtUIManager::setWidgetColorToInvalid(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, this->InvalidValueColor);
  widget->setPalette(pal);
}

void qtUIManager::setWidgetColorToDefault(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, this->DefaultValueColor);
  widget->setPalette(pal);
}

void qtUIManager::setWidgetColorToNormal(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, Qt::white);
  widget->setPalette(pal);
}

void qtUIManager::updateArrayTableWidget(
  smtk::attribute::GroupItemPtr dataItem, QTableWidget* widget)
{
  widget->clear();
  widget->setRowCount(0);
  widget->setColumnCount(0);

  if (!dataItem)
  {
    return;
  }

  std::size_t n = dataItem->numberOfGroups();
  std::size_t m = dataItem->numberOfItemsPerGroup();
  if (!m || !n)
  {
    return;
  }
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return;
  }

  int numCols = static_cast<int>(n * m), numRows = static_cast<int>(item->numberOfValues());
  widget->setColumnCount(numCols);
  widget->setRowCount(numRows);
  for (int h = 0; h < numCols; h++)
  {
    QTableWidgetItem* qtablewidgetitem = new QTableWidgetItem();
    widget->setHorizontalHeaderItem(h, qtablewidgetitem);
  }
  for (int j = 0; j < numCols; j++) // expecting one item for each column
  {
    qtUIManager::updateTableColRows(dataItem->item(j), j, widget);
  }
}

void qtUIManager::updateTableColRows(
  smtk::attribute::ItemPtr dataItem, int col, QTableWidget* widget)
{
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem);
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return;
  }
  int numRows = static_cast<int>(item->numberOfValues());
  widget->setRowCount(numRows);
  QString strValue;
  for (int row = 0; row < numRows; row++)
  {
    strValue = item->valueAsString(row).c_str();
    widget->setItem(row, col, new QTableWidgetItem(strValue));
  }
}

void qtUIManager::updateArrayDataValue(
  smtk::attribute::GroupItemPtr dataItem, QTableWidgetItem* item)
{
  if (!dataItem)
  {
    return;
  }
  smtk::attribute::DoubleItemPtr dItem =
    dynamic_pointer_cast<DoubleItem>(dataItem->item(item->column()));
  smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(item->column()));
  if (dItem)
  {
    dItem->setValue(item->row(), item->text().toDouble());
  }
  else if (iItem)
  {
    iItem->setValue(item->row(), item->text().toInt());
  }
}

bool qtUIManager::getExpressionArrayString(
  smtk::attribute::GroupItemPtr dataItem, QString& strValues)
{
  if (!dataItem || !dataItem->numberOfRequiredGroups())
  {
    return false;
  }
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return false;
  }
  int numberOfComponents = static_cast<int>(dataItem->numberOfItemsPerGroup());
  int nVals = static_cast<int>(item->numberOfValues());
  QStringList strVals;
  smtk::attribute::ValueItemPtr valueitem;
  for (int i = 0; i < nVals; i++)
  {
    for (int c = 0; c < numberOfComponents - 1; c++)
    {
      valueitem = dynamic_pointer_cast<ValueItem>(dataItem->item(c));
      strVals << valueitem->valueAsString(i).c_str() << "\t";
    }
    valueitem = dynamic_pointer_cast<ValueItem>(dataItem->item(numberOfComponents - 1));
    strVals << valueitem->valueAsString(i).c_str();
    strVals << LINE_BREAKER_STRING;
  }
  strValues = strVals.join(" ");
  return true;
}

void qtUIManager::removeSelectedTableValues(
  smtk::attribute::GroupItemPtr dataItem, QTableWidget* table)
{
  if (!dataItem)
  {
    return;
  }

  int numRows = table->rowCount(), numCols = table->columnCount();
  for (int r = numRows - 1; r >= 0; --r)
  {
    if (table->item(r, 0)->isSelected())
    {
      for (int i = 0; i < numCols; i++)
      {
        smtk::attribute::DoubleItemPtr dItem = dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
        smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(i));
        if (dItem)
        {
          dItem->removeValue(r);
        }
        else if (iItem)
        {
          iItem->removeValue(r);
        }
      }
      table->removeRow(r);
    }
  }
}

void qtUIManager::addNewTableValues(
  smtk::attribute::GroupItemPtr dataItem, QTableWidget* table, double* vals, int numVals)
{
  int numCols = table->columnCount();
  if (!dataItem || numCols != numVals)
  {
    return;
  }
  int totalRow = table->rowCount();
  table->setRowCount(++totalRow);

  for (int i = 0; i < numVals; i++)
  {
    smtk::attribute::DoubleItemPtr dItem = dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
    smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(i));
    if (dItem)
    {
      dItem->appendValue(vals[i]);
    }
    else if (iItem)
    {
      iItem->appendValue(vals[i]);
    }
    QString strValue = QString::number(vals[i]);
    table->setItem(totalRow - 1, i, new QTableWidgetItem(strValue));
  }
}

void qtUIManager::onFileItemCreated(qtFileItem* fileItem)
{
  if (m_useInternalFileBrowser)
  {
    fileItem->enableFileBrowser();
  }
  else
  {
    emit this->fileItemCreated(fileItem);
  }
}

void qtUIManager::onModelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem)
{
  emit this->modelEntityItemCreated(entItem);
}

bool qtUIManager::updateTableItemCheckState(
  QTableWidgetItem* labelitem, smtk::attribute::ItemPtr attItem)
{
  bool bEnabled = true;
  if (attItem->definition()->isOptional())
  {
    Qt::CheckState checkState = attItem->isEnabled()
      ? Qt::Checked
      : (attItem->definition()->isEnabledByDefault() ? Qt::Checked : Qt::Unchecked);
    labelitem->setCheckState(checkState);
    QVariant vdata;
    vdata.setValue(static_cast<void*>(attItem.get()));
    labelitem->setData(Qt::UserRole, vdata);
    labelitem->setFlags(labelitem->flags() | Qt::ItemIsUserCheckable);
    bEnabled = (checkState == Qt::Checked);
  }
  return bEnabled;
}

void qtUIManager::registerViewConstructor(const std::string& vtype, widgetConstructor f)
{
  m_constructors[vtype] = f;
}

qtBaseView* qtUIManager::createView(const ViewInfo& info)
{
  if (info.m_UIManager != this)
  {
    // The view being constructed is not refering to this manager!
    return NULL;
  }

  std::map<std::string, widgetConstructor>::const_iterator it;
  it = m_constructors.find(info.m_view->type());
  if (it == m_constructors.end())
  {
    // Constructor for that type could not be found)
    return NULL;
  }

  qtBaseView* qtView = (it->second)(info);
  return qtView;
}

void qtUIManager::onViewUIModified(
  smtk::extension::qtBaseView* bview, smtk::attribute::ItemPtr item)
{
  emit this->viewUIChanged(bview, item);
}

void qtUIManager::onOperationFinished()
{
  emit this->refreshEntityItems();
}

int qtUIManager::getWidthOfAttributeMaxLabel(smtk::attribute::DefinitionPtr def, const QFont& font)
{
  std::string text;
  if (this->Def2LongLabel.contains(def))
  {
    text = this->Def2LongLabel[def];
  }
  else
  {
    this->findDefinitionLongLabel(def, text);
  }

  this->Def2LongLabel[def] = text;
  QFontMetrics fontsize(font);
  return fontsize.width(text.c_str());
}

void qtUIManager::findDefinitionLongLabel(
  smtk::attribute::DefinitionPtr def, std::string& labelText)
{
  QList<smtk::attribute::ItemDefinitionPtr> itemDefs;
  int i, n = static_cast<int>(def->numberOfItemDefinitions());
  for (i = 0; i < n; i++)
  {
    itemDefs.push_back(def->itemDefinition(i));
  }

  this->getItemsLongLabel(itemDefs, labelText);
}

void qtUIManager::getItemsLongLabel(
  const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs, std::string& labelText)
{
  bool hasOptionalItem = false;
  foreach (smtk::attribute::ItemDefinitionPtr itDef, itemDefs)
  {
    smtk::attribute::Item::Type itType = itDef->type();
    // GROUP and VOID type uses their own label length
    if (itType == Item::GroupType || itType == Item::VoidType)
    {
      continue;
    }
    std::string text = itDef->label().empty() ? itDef->name() : itDef->label();
    if (itDef->isOptional())
    {
      hasOptionalItem = true;
    }
    labelText = (text.length() > labelText.length()) ? text : labelText;
  }

  // Add spaces to compensate checkbox width and some spacing.
  labelText += (hasOptionalItem ? "     " : " ");
}

int qtUIManager::getWidthOfItemsMaxLabel(
  const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs, const QFont& font)
{
  std::string text;
  this->getItemsLongLabel(itemDefs, text);
  QFontMetrics fontsize(font);
  return fontsize.width(text.c_str());
}

void qtUIManager::findDefinitionsLongLabels()
{
  this->Def2LongLabel.clear();
  // Generate list of all concrete definitions in the manager
  std::vector<smtk::attribute::DefinitionPtr> defs;
  std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
  m_attResource->findBaseDefinitions(baseDefinitions);
  std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;

  for (baseIter = baseDefinitions.begin(); baseIter != baseDefinitions.end(); baseIter++)
  {
    std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
    m_attResource->findAllDerivedDefinitions(*baseIter, true, derivedDefs);
    defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
  }

  std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
  for (defIter = defs.begin(); defIter != defs.end(); defIter++)
  {
    std::string text;
    this->findDefinitionLongLabel(*defIter, text);
    this->Def2LongLabel[*defIter] = text;
  }
}
