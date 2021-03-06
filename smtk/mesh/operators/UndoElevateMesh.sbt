<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "undo elevate mesh" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="undo elevate mesh"
            Label="Mesh - Undo Elevate" BaseType="operation">
      <BriefDescription>
        Restore a mesh to its unelevated state.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Restore a mesh to its unelevated state.
        &lt;p&gt;Some operators can distort a mesh's coordinates
        (e.g. Mesh - Elevate). This operator removes the elevateing
        applied by these operators.
      </DetailedDescription>
      <ItemDefinitions>
        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh to restore.</BriefDescription>
        </MeshEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(undo elevate mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <Component Name="tess_changed" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
          <Accepts><Resource Name="smtk::model::Resource" Filter=""/></Accepts>
        </Component>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
