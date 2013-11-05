/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ModelEntity.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_ModelEntity_h
#define __smtk_view_ModelEntity_h

#include "smtk/view/Base.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT ModelEntity : public Base
    {
    public:
      static smtk::view::ModelEntityPtr New(const std::string &myName)
      { return smtk::view::ModelEntityPtr(new ModelEntity(myName)); }

      ModelEntity(const std::string &myTitle);

      virtual ~ModelEntity();
      virtual Base::Type type() const;
      // NEED TO CLARIFY - are groups of entities
      // represented by this mask?  For example, a set of model faces??
      // If not we need to add something to clarify this
      smtk::model::MaskType modelEntityMask() const
      {return this->m_modelEntityMask;}
      void setModelEntityMask(smtk::model::MaskType mask)
      {this->m_modelEntityMask = mask;}

      // If this def is not null then the section should
      // display all model entities of the requested mask along
      // with the attribute of this type in a table view
      smtk::attribute::DefinitionPtr definition() const
      {return this->m_attributeDefinition;}
      void setDefinition(smtk::attribute::DefinitionPtr def)
      {this->m_attributeDefinition = def;}

    protected:
      smtk::model::MaskType m_modelEntityMask;
      smtk::attribute::DefinitionPtr m_attributeDefinition;
    private:

    };
  }
}

#endif /* __smtk_view_ModelEntity_h */