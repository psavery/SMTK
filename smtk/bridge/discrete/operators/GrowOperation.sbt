<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "grow" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="grow" BaseType="operation" Label="Face - Grow">
      <BriefDescription>
        Create a new face by a group of mesh faces selected by user or a group of mesh faces grown from
        a seed face.
      </BriefDescription>
      <DetailedDescription>
        Create a new face by a group of mesh faces selected by user or a group of mesh faces grown from
        a seed face.

        Mesh selection supports `grow selection`, `grow and append selection` and `grow and
        remove selection`. After a selection has been made, user can click the button on the
        righest to create face group.

        First mode:
        If only one cell is selected, grow operator would treat this cell as a seed face, picking
        the neighboring faces recursively untill the angle between the normals of adjacent faces exceeds
        the upper bound.

        Second mode:
        If multiple mesh faces are selected, grow operator would modify current face selection
        using the input selection(feature angle would be ignored and no grow operation would happen. It
        works as purely arithematic).
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1">
        <Accepts>
          <Resource Name="smtk::bridge::discrete::Resource" Filter="model"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <MeshSelection Name="selection" ModelEntityRef="model">
          <MembershipMask>face</MembershipMask>
        </MeshSelection>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <BriefDescription>
            Feature angle is the angle between the normals of two adjacent faces. Here feature
            angle is defind as an upper bound. To grow by feature angle, please select only one cell in the target area.
          </BriefDescription>
          <DetailedDescription>
            Feature angle is the angle between the normals of two adjacent faces. Here feature
            angle is defined as an upper bound. To grow by feature angle, please select only one cell in the target area.

            Feature angle also is the supplementary angle of the dihedral angle between
            two adjacent faces.
          </DetailedDescription>
          <DefaultValue>30.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">180.</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(grow)" BaseType="result">
      <ItemDefinitions>
        <MeshSelection Name="selection">
        </MeshSelection>
        <Void Name= "show selection" AdvanceLevel="11">
          <BriefDescription> Show selection when the operation is finished </BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
