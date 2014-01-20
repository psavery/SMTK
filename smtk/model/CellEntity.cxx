#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CursorArrangementOps.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/UseEntity.h"

#include <deque>

namespace smtk {
  namespace model {


/**\brief Return the model owning this cell (or an invalid cursor if the cell is free).
  *
  */
ModelEntity CellEntity::model() const
{
  StoragePtr store = this->storage();
  return ModelEntity(
    store,
    store->modelOwningEntity(this->entity()));
}

/**\brief Report the toplevel shell records associated with the cell.
  *
  * The uses can be used to discover lower-dimensional cells that
  * form the boundary of this cell.
  */
ShellEntities CellEntity::shellEntities() const
{
  ShellEntities result;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
}

/**\brief Report all of the lower-dimensional cells bounding this cell.
  *
  * The bounding cells are obtained by traversing all of the shells of
  * the first use of this cell.
  */
CellEntities CellEntity::boundingCells() const
{
  CellEntities cells;
  UseEntities uses = this->uses<UseEntities>();
  if (!uses.empty())
    {
    std::deque<ShellEntity> tmp =
      uses[0].shellEntities<std::deque<ShellEntity> >();
    ShellEntity shell;
    for (; !tmp.empty(); tmp.pop_front())
      {
      shell = tmp.front();
      // Add the shell's use's cells to the output
      UseEntities su = shell.uses<UseEntities>();
      UseEntities::iterator uit;
      for (uit = su.begin(); uit != su.end(); ++uit)
        {
        CellEntity ce = uit->cell();
        if (ce.isValid())
          cells.insert(cells.end(), ce);
        }
      // Add the shell's inner shells to tmp
      ShellEntities innerShells =
        shell.containedShellEntities<ShellEntities>();
      ShellEntities::iterator iit;
      for (iit = innerShells.begin(); iit != innerShells.end(); ++iit)
        tmp.push_back(*iit);
      }
    }
  return cells; // or CellEntities(cells.begin(), cells.end()); if cells is a set.
}

/*! \fn CellEntity::inclusions() const
 * \brief Return the list of all entities embedded in this cell.
 *
 * Note that the inverse of this relation is provided by
 * Cursor::embeddedIn().
 */

/*! \fn CellEntity::uses() const
 * \brief Report all of the "use" records associated with the cell.
 *
 * The uses can be used to discover higher-dimensional cells that
 * this cell borders.
 * Each sense of a cell has its own use.
 */

  } // namespace model
} // namespace smtk
