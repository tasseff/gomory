import sys
import argparse
import os
import json
import subprocess

import gurobipy as grb

import make_mip


__author__ = "Byron Tasseff, Connor Riley"
__credits__ = ["Byron Tasseff", "Connor Riley"]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = "Connor Riley"
__email__ = ""
__status__ = "Development"


def main(num_problems, num_constraints, num_variables, pure, folder):
    if not os.path.exists(folder):
        os.makedirs(folder)
    make_mip.make_mips(num_problems, num_constraints, num_variables, pure, folder + "/generated_problem.lp")
    obj = run_gurobi(folder)
    if (obj >= 0 - .0000000001 and obj <= 0 + .0000000001):
        print("The objective was 0, terminating")
        sys.exit()
    write_temp_input(folder)
    run_gomory()


def run_gomory():
    rc = subprocess.check_call(["./RUN.sh", folder])
    return 0


def write_temp_input(folder):
    d = {}
    d["parameters"] = {}
    d["parameters"]["model"] = "generated_problem.lp"
    d["parameters"]["solution"] = "solution.sol"
    with open(folder + '/temp.json', 'w') as outfile:
        json.dump(d, outfile)
    return 0


def run_gurobi(folder):
    rc = subprocess.check_call(["gurobi_cl", "ResultFile=" + folder + "/gurobi_solution.sol", folder+"/generated_problem.lp"])
    with open(folder + "/gurobi_solution.sol") as f:
        content = f.readlines()
    obj = content[0].split("=")[1]
    return float(obj)


if __name__ == "__main__":
    description = 'Generate random feasible mixed-integer programs.'
    parser = argparse.ArgumentParser(description = description)
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
    parser.add_argument('-f', '--folder', type=str, nargs=1,
                        metavar= 'folder',
                        help = 'output folder path')
   
    args = parser.parse_args()
   
    num_problems = 1
   
    if args.num_problems:
        num_problems = args.num_problems[0]

    num_constraints = 100
    if args.num_constraints:
        num_constraints = args.num_constraints[0]

    num_variables = 10
    if args.num_variables:
        num_variables = args.num_variables[0]

    folder = 'temp'
    if args.folder:
        folder = args.folder[0]

    pure = False
    if args.pure:
        pure = args.pure
    main(num_problems, num_constraints, num_variables, pure, folder)