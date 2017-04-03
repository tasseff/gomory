#!/usr/bin/env python

"""make_mip.py: Generate random feasible mixed-integer programs."""

import argparse
import gurobipy as grb
import numpy as np
import os
import random
import sys
import scipy.sparse

__author__ = "Byron Tasseff"
__credits__ = ["Byron Tasseff"]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = "Byron Tasseff"
__email__ = "byron@tasseff.com"
__status__ = "Development"

def make_mip(num_constraints, num_variables, pure, output_path):
    # Create a feasible maximization problem with constraints A' y <= c.
    A = np.random.randint(-5, 5, size = (num_variables, num_constraints))
    x = np.random.randint(0, 10, size = (num_constraints, 1))
    b = np.matmul(A, x)
    c = np.random.randint(0, num_variables, size = (num_constraints, 1))
    A_T = A.transpose()

    model = grb.Model(os.path.basename(output_path))
    model.setParam('OutputFlag', False)

    # Set the possible variable types.
    var_types = [grb.GRB.INTEGER]
    if not pure:
       var_types.append(grb.GRB.CONTINUOUS)

    var_list = []
    obj = grb.LinExpr()
    for j in range(0, num_variables):
        var_type = random.choice(var_types)
        var = model.addVar(vtype = var_type, lb = -grb.GRB.INFINITY, ub = grb.GRB.INFINITY)
        var_list.append(var)
        obj += var * b[j]

    model.update()
    model.setObjective(obj, sense = grb.GRB.MAXIMIZE)

    for i in range(0, num_constraints):
        lhs = grb.LinExpr()
        for j, var in enumerate(var_list):
            lhs += A_T[i, j] * var
        model.addConstr(lhs, grb.GRB.LESS_EQUAL, c[i])

    model.update()
    model.optimize()
    status = model.Status

    if status != grb.GRB.OPTIMAL:
        print('Generated problem is infeasible.')
        sys.exit(1)

    model.write(output_path)

def make_mips(num_problems, num_constraints, num_variables, pure, output_path):
    if num_problems == 1:
        make_mip(num_constraints, num_variables, pure, output_path)
    else:
        for i in range(0, num_problems):
            basename = ('ip' if pure else 'mip') + '-' + str(i) + '.mps'
            output_path_ = os.path.join(output_path, basename)
            num_variables_ = random.randint(1, num_variables)
            num_constraints_ = random.randint(1, num_constraints)
            make_mip(num_constraints_, num_variables_, pure, output_path_)

if __name__ == "__main__":
    description = 'Generate random feasible mixed-integer programs.'
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument('output_path', type = str, nargs = 1,
                        metavar = 'outputPath',
                        help = 'path to output folder or file')
    parser.add_argument('-np', '--num_problems', type = int, nargs = 1,
                        metavar = 'numProblems',
                        help = 'number of problems to generate (1)')
    parser.add_argument('-nc', '--num_constraints', type = int, nargs = 1,
                        metavar = 'numConstraints',
                        help = 'maximum number of constraints (100')
    parser.add_argument('-nv', '--num_variables', type = int, nargs = 1,
                        metavar = 'numVariables',
                        help = 'maximum number of variables (100)')
    parser.add_argument('-p', '--pure', action = 'store_true',
                        help = 'generate pure integer programs (False)')

    args = parser.parse_args()
    output_path = args.output_path[0]

    num_problems = 1
    if args.num_problems:
        num_problems = args.num_problems[0]

    num_constraints = 100
    if args.num_constraints:
        num_constraints = args.num_constraints[0]

    num_variables = 10
    if args.num_variables:
        num_variables = args.num_variables[0]

    pure = False
    if args.pure:
        pure = args.pure

    make_mips(num_problems, num_constraints, num_variables, pure, output_path)
