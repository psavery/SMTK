<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Split Face" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="split face" BaseType="operation" Label="Face - Split">
      <BriefDescription>
        Split a face/faces into several small faces. Feature angle is used to decide whether split the
        face or not.
      </BriefDescription>
      <DetailedDescription>
        Split a face/faces into several small faces. Feature angle is used to decide whether split the
        face or not.

        Only when the feature angle of two adjacent faces is smaller than the threthold provided by the
        user then would the face be split.

      </DetailedDescription>
      <AssociationsDef Name="face to split" NumberOfRequiredValues="0" Extensible="1">
        <Accepts><Resource Name="smtk::bridge::discrete::Resource" Filter="face"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Component Name="model" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::bridge::discrete::Resource" Filter="model"/></Accepts>
        </Component>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <DefaultValue>15.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">360.</Max>
          </RangeInfo>
           <BriefDescription>
             Feature angle is the angle between the normals of two adjacent faces. Here feature
             angle is defind as an upper bound.
           </BriefDescription>
           <DetailedDescription>
             Feature angle is the angle between the normals of two adjacent faces. Here feature
             angle is defined as an upper bound.

             Feature angle also is the supplementary angle of the dihedral angle between
             two adjacent faces.
           </DetailedDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(split face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
