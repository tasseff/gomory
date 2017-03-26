#!/usr/bin/env python

"""make_mip.py: Generate random feasible pure and mixed-integer programs."""

import argparse

__author__ = "Byron Tasseff"
__credits__ = ["Byron Tasseff"]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = "Byron Tasseff"
__email__ = "byron@tasseff.com"
__status__ = "Development"

#def make_mip(num_constraints, num_variables, pure, output_path):

def make_mips(num_problems, num_constraints, num_variables, pure, output_path):
    if num_problems == 1:
        print(num_problems, num_constraints, num_variables, pure, output_path)
        #make_mip(num_constraints, num_variables, output_path)
    else:
        print(num_problems, num_constraints, num_variables, pure, output_path)
        #for i in range(0, num_problems):
        #    output_file_path = os.path.join(output_path,

if __name__ == "__main__":
    description = 'Generate random feasible pure and mixed-integer programs.'
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

    num_variables = 100
    if args.num_variables:
        num_variables = args.num_variables[0]

    pure = False
    if args.pure:
        pure = args.pure

    make_mips(num_problems, num_constraints, num_variables, pure, output_path)
