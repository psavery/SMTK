#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
from __future__ import print_function
import os
import sys
import smtk
import smtk.bridge.discrete
import smtk.testing


class TestDiscreteSession(smtk.testing.TestCase):

    def read(self, filename):
        readOp = smtk.bridge.discrete.ReadOperation.create()
        readOp.parameters().find('filename').setValue(filename)
        result = readOp.operate()
        self.assertEqual(
            result.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'discrete read operation failed')

        # store the resource so it doesn't go out of scope
        self.resource = smtk.model.Resource.CastTo(
            result.find('resource').value(0))
        return self.resource.findEntitiesOfType(int(smtk.model.MODEL_ENTITY))

    def setEntityProperty(self, ents, propName, **kwargs):
        """Set a property value (or vector of values) on an entity (or vector of entities).

        You may pass any combination of "as_int", "as_float", or "as_string" as named
        arguments specifying the property values. The values of these named arguments may
        be a single value or a list of values. Values will be coerced to the named type.

        Example:

          SetEntityProperty(face, 'color', as_float=(1., 0., 0.))
          SetEntityProperty(edge, 'name', as_string='edge 20')
          SetEntityProperty(body, 'visited', as_int='edge 20')
        """
        spr = smtk.model.SetProperty.create()
        if hasattr(ents, '__iter__'):
            [spr.parameters().associate(ent.component()) for ent in ents]
        else:
            spr.parameters().associate(ents.component())
        spr.parameters().find('name').setValue(propName)
        if 'as_int' in kwargs:
            vlist = kwargs['as_int']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            intVal = spr.parameters().find('integer value')
            intVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                intVal.setValue(i, vlist[i])
        if 'as_float' in kwargs:
            vlist = kwargs['as_float']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            floatVal = spr.parameters().find('float value')
            floatVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                floatVal.setValue(i, vlist[i])
        if 'as_string' in kwargs:
            vlist = kwargs['as_string']
            if not hasattr(vlist, '__iter__'):
                vlist = [vlist, ]
            stringVal = spr.parameters().find('string value')
            stringVal.setNumberOfValues(len(vlist))
            for i in xrange(len(vlist)):
                stringVal.setValue(i, vlist[i])
        res = spr.operate()
        self.assertEqual(
            res.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'set property failed')
        return res.findInt('outcome').value(0)

    def resetTestFiles(self):
        self.filesToTest = []

    def addExternalFile(self, pathStr, numCells, numGroups, validator=None):
        self.filesToTest += [{'filename': pathStr, 'numCells': numCells,
                              'numGroups': numGroups, 'validator': validator}]

    def addTestFile(self, pathList, numCells, numGroups, validator=None):
        self.addExternalFile(
            os.path.join(*([smtk.testing.DATA_DIR, ] + pathList)),
            numCells, numGroups, validator)

    def validateTest2D(self, model):
        "Verify that the test2D model is imported correctly."
        faces = [smtk.model.Face(x) for x in model.cells()]
        f4l = [f for f in faces if f.name() == 'Face4']
        self.assertEqual(len(f4l), 1, 'Could not find test2D "Face4"')
        face4 = f4l[0]
        outer = face4.positiveUse().loops()
        self.assertEqual(len(outer), 1, 'Face4 should have 1 outer loop')
        inner = outer[0].containedLoops()
        self.assertEqual(
            len(inner), 1, 'Face4\'s outer loop should have 1 inner loop')
        self.assertEqual(len(inner[0].edgeUses()), 1,
                         'Face4\'s inner loop should have 1 edge use')
        innerEdge = inner[0].edgeUses()[0].edge()
        self.assertEqual(innerEdge.name(), 'Edge10',
                         'Face4\'s inner loop should one edge named "Edge10"')

    def validateHybrid(self, model):
        "Verify that the hybridModelOneCube model is imported correctly."
        faces = [smtk.model.Face(x) for x in model.cells()]
        f7l = [f for f in faces if f.name() == 'Face7']
        self.assertEqual(
            len(f7l), 1, 'Could not find floating "Face7" in hybridModelOneCube')
        face7 = f7l[0]
        outer = face7.positiveUse().loops()
        self.assertEqual(len(outer), 0, 'Face7 should have 0 outer loop')

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            mbs = self.addModelToScene(model)

            self.renderer.SetBackground(0.5, 0.5, 1)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(15, -10, -20)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatch(
                ['baseline', 'smtk', 'discrete', 'hybridModelOneCube.png'])
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))

    def validatePMDC(self, model):
        "Verify that the PMDC model is imported correctly."
        # Assign a color to each face in the groups.
        groupColors = {
            'Coil1':             '#875d4f',
            'Coil2':             '#896f59',
            'Coil3':             '#a99b86',

            'Core':              '#5c3935',

            'Magnet1Caps':       '#093020',
            'Magnet1RotorFace':  '#0e433b',
            'Magnet2Caps':       '#104c57',
            'Magnet2RotorFace':  '#0e5168',
            'MagnetHousingFace': '#125b6e',

            'OuterSurf':         '#90ab77',
            'OuterUpperLower':   '#a7b894',

            'Rotor':             '#c4d6b3',
            'RotorMagnetFace':   '#5a8559',
        }

        for grp in model.groups():
            if grp.name() in groupColors:
                color = groupColors[grp.name()]
                self.setEntityProperty(grp.members(), 'color',
                                       as_float=self.hex2rgb(color))

        # TODO: Should run grow operator on some of the faces here.
        #       Especially if we test group membership afterwards.

        if self.haveVTK() and self.haveVTKExtension():

            self.startRenderTest()

            mbs = self.addModelToScene(model)

            self.renderer.SetBackground(1, 1, 1)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(10, 15, 20)
            cam.SetViewUp(0, 1, 0)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatch(['baseline', 'smtk', 'discrete', 'pmdc.png'])
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))

    def setUp(self):
        import os
        self.resetTestFiles()
        self.addTestFile(['model', '2d', 'cmb', 'test2D.cmb'],
                         4, 0, self.validateTest2D)
        self.addTestFile(['model', '3d', 'cmb', 'RectangleBox.cmb'], 1, 2)
        self.addTestFile(['model', '3d', 'cmb', 'smooth_surface.cmb'], 6, 0)
        self.addTestFile(['model', '3d', 'cmb', 'pmdc.cmb'],
                         7, 13, self.validatePMDC)
        self.addTestFile(
            ['model', '3d', 'cmb', 'hybridModelOneCube.cmb'], 2, 1, self.validateHybrid)
        self.shouldSave = False

    def verifyRead(self, filename, numCells, numGroups, validator):
        """Read a single file and validate that the reader worked.
        This is done by checking the number of free cells and groups
        reported by the output model as well as running an optional
        function on the model to do further model-specific testing."""

        print('\n\nFile: {fname}'.format(fname=filename))

        mod = smtk.model.Model(self.read(filename)[0])

        print('  {mt} model'.format(
            mt=smtk.model.ModelGeometryStyleName(mod.geometryStyle())))
        print('\nFree cells:\n  %s' %
              '\n  '.join([x.name() for x in mod.cells()]))
        print('\nGroups:\n  %s\n' %
              '\n  '.join([x.name() for x in mod.groups()]))
        if (numCells >= 0 and len(mod.cells()) != numCells) or (numGroups >= 0 and len(mod.groups()) != numGroups):
            print(smtk.io.SaveJSON.fromModelResource(self.resource))

        self.assertEqual(
            mod.geometryStyle(), smtk.model.DISCRETE,
            'Expected a discrete model, got a {mt} model'.format(
                mt=smtk.model.ModelGeometryStyleName(mod.geometryStyle())))
        if numCells >= 0:
            self.assertEqual(len(mod.cells()), numCells,
                             'Expected {nc} free cells'.format(nc=numCells))
        if numGroups >= 0:
            self.assertEqual(len(mod.groups()), numGroups,
                             'Expected {ng} groups'.format(ng=numGroups))
        if validator:
            validator(mod)

    def testRead(self):
        "Read each file named in setUp and validate the reader worked."
        for test in self.filesToTest:
            self.verifyRead(test['filename'], test['numCells'],
                            test['numGroups'], test['validator'])

        if self.shouldSave:
            out = file('test.json', 'w')
            print(smtk.io.SaveJSON.fromModelResource(self.resource), file=out)
            out.close()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
