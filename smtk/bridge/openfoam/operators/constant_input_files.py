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

""" constant_input_files.py:

These functions generate input files necessary for adding an obstacle to an
OpenFOAM background mesh. It is used for demo purposes only; in the future,
these files will be (at least partly) configurable by the user.

"""

import os


def write_surface_feature_extract_dict(systemDirectory, filename):

    with open(systemDirectory + '/surfaceFeatureExtractDict', 'w') as f:
        f.write(
            "/*--------------------------------*- C++ -*----------------------------------*\\\n"
            "| =========                 |                                                 |\n"
            "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n"
            "|  \\\\    /   O peration     | Version:  plus                                  |\n"
            "|   \\\\  /    A nd           | Web:      www.OpenFOAM.com                      |\n"
            "|    \\\\/     M anipulation  |                                                 |\n"
            "\*---------------------------------------------------------------------------*/\n"
            "\n"
            "/* Generated by SMTK (http://www.computationalmodelbuilder.org/smtk/) */\n"
            "\n"
            "FoamFile\n"
            "{\n"
            "    version     2.0;\n"
            "    format      ascii;\n"
            "    class       dictionary;\n"
            "    object      surfaceFeatureExtractDict;\n"
            "}\n"
            "\n"
            "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n"
            "\n"
        )

        f.write('%s\n{\n' % (filename))
        f.write('    extractionMethod extractFromSurface;\n')
        f.write('    extractFromSurfaceCoeffs\n    {\n')
        f.write(' ' * 8 + 'includedAngle 150;\n    }\n')
        f.write('    subsetFeatures\n    {\n')
        f.write(' ' * 8 + 'nonManifoldEdges no;\n')
        f.write(' ' * 8 + 'openEdges yes;\n    }\n')
        f.write('    writeObj yes;\n}\n')
        f.write('\n')
        f.write(
            '// ************************************************************************* //\n')


def write_fv_schemes_dict(systemDirectory):

    with open(systemDirectory + '/fvSchemes', 'w') as f:
        f.write(
            "/*--------------------------------*- C++ -*----------------------------------*\\\n"
            "| =========                 |                                                 |\n"
            "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n"
            "|  \\\\    /   O peration     | Version:  plus                                  |\n"
            "|   \\\\  /    A nd           | Web:      www.OpenFOAM.com                      |\n"
            "|    \\\\/     M anipulation  |                                                 |\n"
            "\*---------------------------------------------------------------------------*/\n"
            "\n"
            "/* Generated by SMTK (http://www.computationalmodelbuilder.org/smtk/) */\n"
            "\n"
            "FoamFile\n"
            "{\n"
            "    version     2.0;\n"
            "    format      ascii;\n"
            "    class       dictionary;\n"
            "    object      fvSchemes;\n"
            "}\n"
            "\n"
            "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n"
            "\n"
        )

        f.write('ddtSchemes\n')
        f.write('{\n')
        f.write('    default         steadyState;\n')
        f.write('}\n')
        f.write('\n')
        f.write('gradSchemes\n')
        f.write('{\n')
        f.write('    default         Gauss linear;\n')
        f.write('    grad(U)         cellLimited Gauss linear 1;\n')
        f.write('}\n')
        f.write('\n')
        f.write('divSchemes\n')
        f.write('{\n')
        f.write('    default         none;\n')
        f.write('    div(phi,U)      bounded Gauss linearUpwindV grad(U);\n')
        f.write('    div(phi,k)      bounded Gauss upwind;\n')
        f.write('    div(phi,omega)  bounded Gauss upwind;\n')
        f.write('    div((nuEff*dev2(T(grad(U))))) Gauss linear;\n')
        f.write('}\n')
        f.write('\n')
        f.write('laplacianSchemes\n')
        f.write('{\n')
        f.write('    default         Gauss linear corrected;\n')
        f.write('}\n')
        f.write('\n')
        f.write('interpolationSchemes\n')
        f.write('{\n')
        f.write('    default         linear;\n')
        f.write('}\n')
        f.write('\n')
        f.write('snGradSchemes\n')
        f.write('{\n')
        f.write('    default         corrected;\n')
        f.write('}\n')
        f.write('\n')
        f.write('wallDist\n')
        f.write('{\n')
        f.write('    method meshWave;\n')
        f.write('}\n')
        f.write('\n')
        f.write(
            '// ************************************************************************* //\n')


def write_fv_solution_dict(systemDirectory):

    with open(systemDirectory + '/fvSolution', 'w') as f:
        f.write(
            "/*--------------------------------*- C++ -*----------------------------------*\\\n"
            "| =========                 |                                                 |\n"
            "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n"
            "|  \\\\    /   O peration     | Version:  plus                                  |\n"
            "|   \\\\  /    A nd           | Web:      www.OpenFOAM.com                      |\n"
            "|    \\\\/     M anipulation  |                                                 |\n"
            "\*---------------------------------------------------------------------------*/\n"
            "\n"
            "/* Generated by SMTK (http://www.computationalmodelbuilder.org/smtk/) */\n"
            "\n"
            "FoamFile\n"
            "{\n"
            "    version     2.0;\n"
            "    format      ascii;\n"
            "    class       dictionary;\n"
            "    object      fvSolution;\n"
            "}\n"
            "\n"
            "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n"
            "\n"
        )

        f.write('solvers\n')
        f.write('{\n')
        f.write('    p\n')
        f.write('    {\n')
        f.write('        solver          GAMG;\n')
        f.write('        smoother        GaussSeidel;\n')
        f.write('        tolerance       1e-7;\n')
        f.write('        relTol          0.01;\n')
        f.write('    }\n')
        f.write('\n')
        f.write('    Phi\n')
        f.write('    {\n')
        f.write('        $p;\n')
        f.write('    }\n')
        f.write('\n')
        f.write('    U\n')
        f.write('    {\n')
        f.write('        solver          smoothSolver;\n')
        f.write('        smoother        GaussSeidel;\n')
        f.write('        tolerance       1e-8;\n')
        f.write('        relTol          0.1;\n')
        f.write('        nSweeps         1;\n')
        f.write('    }\n')
        f.write('\n')
        f.write('    k\n')
        f.write('    {\n')
        f.write('        solver          smoothSolver;\n')
        f.write('        smoother        GaussSeidel;\n')
        f.write('        tolerance       1e-8;\n')
        f.write('        relTol          0.1;\n')
        f.write('        nSweeps         1;\n')
        f.write('    }\n')
        f.write('\n')
        f.write('    omega\n')
        f.write('    {\n')
        f.write('        solver          smoothSolver;\n')
        f.write('        smoother        GaussSeidel;\n')
        f.write('        tolerance       1e-8;\n')
        f.write('        relTol          0.1;\n')
        f.write('        nSweeps         1;\n')
        f.write('    }\n')
        f.write('}\n')
        f.write('\n')
        f.write('SIMPLE\n')
        f.write('{\n')
        f.write('    nNonOrthogonalCorrectors 0;\n')
        f.write('    consistent yes;\n')
        f.write('}\n')
        f.write('\n')
        f.write('potentialFlow\n')
        f.write('{\n')
        f.write('    nNonOrthogonalCorrectors 10;\n')
        f.write('}\n')
        f.write('\n')
        f.write('relaxationFactors\n')
        f.write('{\n')
        f.write('    equations\n')
        f.write('    {\n')
        f.write('        U               0.9;\n')
        f.write('        k               0.7;\n')
        f.write('        omega           0.7;\n')
        f.write('    }\n')
        f.write('}\n')
        f.write('\n')
        f.write('cache\n')
        f.write('{\n')
        f.write('    grad(U);\n')
        f.write('}\n')
        f.write('\n')
        f.write(
            '// ************************************************************************* //\n')


def write_mesh_quality_dict(systemDirectory):

    with open(systemDirectory + '/meshQualityDict', 'w') as f:
        f.write(
            "/*--------------------------------*- C++ -*----------------------------------*\\\n"
            "| =========                 |                                                 |\n"
            "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n"
            "|  \\\\    /   O peration     | Version:  plus                                  |\n"
            "|   \\\\  /    A nd           | Web:      www.OpenFOAM.com                      |\n"
            "|    \\\\/     M anipulation  |                                                 |\n"
            "\*---------------------------------------------------------------------------*/\n"
            "\n"
            "/* Generated by SMTK (http://www.computationalmodelbuilder.org/smtk/) */\n"
            "\n"
            "FoamFile\n"
            "{\n"
            "    version     2.0;\n"
            "    format      ascii;\n"
            "    class       dictionary;\n"
            "    object      meshQualityDict;\n"
            "}\n"
            "\n"
            "// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n"
            "\n"
        )

        f.write('#includeEtc "caseDicts/meshQualityDict"\n')
        f.write('minFaceWeight 0.02;\n')
        f.write('\n')
        f.write(
            '// ************************************************************************* //\n')