<?xml version="1.0" encoding="utf-8" ?>
<!--
Generated by hand. See David for hand.
-->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="parameter test" BaseType="operation">
      <ItemDefinitions>
        <Int Name="shouldSucceedInteger" NumberOfRequiredValues="1">
          <DefaultValue>1</DefaultValue>
        </Int>
        <Double Name="shouldSucceedFloat" NumberOfRequiredValues="3">
          <DefaultValue>0., 1., 2.</DefaultValue>
        </Double>
        <String Name="shouldSucceedString" NumberOfRequiredValues="1" Extensible="1" MaxNumberOfValues="2">
          <DefaultValue>zero</DefaultValue>
        </String>
        <Component Name="shouldSucceedUUID" NumberOfRequiredValues="1" Extensible="1">
          <!-- Accept all components -->
        </Component>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(parameter test)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>