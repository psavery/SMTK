#ifndef __smtk_model_VolumeUse_h
#define __smtk_model_VolumeUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Shell;
class VolumeUse;
class Volume;
typedef std::vector<Shell> Shells;
typedef std::vector<VolumeUse> VolumeUses;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT VolumeUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(VolumeUse,UseEntity,isVolumeUse);

  Volume volume() const; // The volume bounded by this face use (if any)
  Shells shells() const; // The toplevel boundary loops for this face (hole-loops not included)
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_VolumeUse_h
