import sys
import argparse
import os
import json
import subprocess
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7')
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7/')
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7/gurobipy')
import gurobipy as grb

import make_mip


__author__ = "Byron Tasseff, Connor Riley"
__credits__ = ["Byron Tasseff", "Connor Riley"]
__license__ = "MIT"
__version__ = "0.0.2"
__maintainer__ = "Connor Riley"
__email__ = ""
__status__ = "Development"


def main(folder, num_vars, pure, use_mixed_on_pure):
    result_files = initialize_results_files(folder, num_vars, use_mixed_on_pure)
    results_store = {}
    results_store["num_starting_vars"] = num_vars
    j = 0
    while(j < 50):
    	new_folder_path = folder + "/ex_" + str(num_vars) + "_" + str(j)
        if not os.path.exists(new_folder_path):
            os.makedirs(new_folder_path)
        print("generating problem")            
        obj = make_problem(num_vars,pure,new_folder_path)
        j += 1
        print("running gomory cuts")
        run_gomory(new_folder_path, use_mixed_on_pure)
        print("outputing results")
        output_intermediate_results(new_folder_path, result_files, obj, j, 
            results_store, use_mixed_on_pure)
    print("writing results")
    write_results_store(results_store, folder, num)
    return 0


def make_problem(num_vars, pure, new_folder_path):
    feasible = False
    trivial = True
    while not feasible and trivial:
        (feasible, obj) = make_mip.make_mip(num_vars, num_vars, pure, 
            new_folder_path + "/generated_problem.lp")
        if feasible:
            write_temp_input(new_folder_path)
            trivial = is_trivial(new_folder_path, obj)
    return obj


def is_trivial(folder, actual_objective):
    rc = subprocess.check_call(["./RUN_NAIVE.sh", folder])
    (num_cuts, _,_,_,_) = get_stats(folder + "/naive.txt", actual_objective)
    if num_cuts == 0:
        return True
    return False


def write_results_store(results_store, folder, num):
    for solve_type in results_store:
        path = get_bar_graph_path(solve_type, folder, num)
        write_bar_graph_data(path, results_store, solve_type)
    return 0


def output_intermediate_results(new_folder_path, result_files, actual_objective, 
    j, results_store, use_mixed_on_pure):
    methods = ["naive", "lex", "rounds", "rounds_lex"]
    last_lines = []
    if use_mixed_on_pure:
        methods.extend["naive_mixed", "lex_mixed", "rounds_mixed", "rounds_lex_mixed"]
    for method in methods:
        stats = get_stats(new_folder_path + "/" + method + ".txt", actual_objective)
        if method not in results_store.keys():
            results_store[method] = 0
        if stats[-1] == True : results_store[method] += 1
        last_lines.append(stats)
    for i, path in enumerate(result_files):
        write_data_line(path, last_lines[i],  j)
    return 0


def get_stats(filepath, actual_objective):
    with open(filepath, "r") as f:
        lines = f.read().splitlines()
    max_det = 0
    for i,line in enumerate(lines):
        if i == 0:
            continue
        split_line = line.split(",")
        det = split_line[2]
        if abs(float(det)) > max_det:
            max_det = abs(float(det)) 
    last_line_split = lines[-1].split(",")
    obj = last_line_split[3]
    num_cuts = last_line_split[0]
    num_constr = last_line_split[1]
    achieved_solution = test_for_solution(num_cuts, obj, actual_objective)
    return(num_cuts, num_constr, max_det, obj, achieved_solution) 


def test_for_solution(num_cuts, obj, actual_objective):
    test = abs(float(obj) - float(actual_objective))
    if test < .0000001 and int(num_cuts) < 2500:
        return True
    return False


def run_gomory(folder, use_mixed_on_pure):
    rc = subprocess.check_call(["./RUN.sh", folder])
    if use_mixed_on_pure:
        rc = subprocess.check_call(["./RUN_MIXED.sh", folder])
    return 0


def run_gurobi(folder):
    rc = subprocess.check_call(["gurobi_cl", "ResultFile=" + folder + "/gurobi_solution.sol", folder+"/generated_problem.lp"])
    with open(folder + "/gurobi_solution.sol") as f:
        content = f.readlines()
    obj = content[0].split("=")[1]
    return float(obj)


def write_bar_graph_data(bar_path, results_store, solve_method):
    num_starting_vars = results_store["num_starting_vars"]
    data_to_write = str(num_starting_vars) + "," + str(results_store[solve_method]) + "\n"
    if not os.path.exists(bar_path):
        f = open(bar_path, 'w')
        f.write('num_starting_vars,num_finished\n')
        f.close()
    f = open(bar_path, "a")
    f.write(data_to_write)
    f.close()


def get_bar_graph_path(solve_method, folder, i):
    return folder + "/" + "bar_graph_" + solve_method + "_" + str(i) + ".csv"


def initialize_results_files(folder, num_vars, use_mixed_on_pure):
    if not os.path.exists(folder):
        os.makedirs(folder)
    results_file_path_naive = folder + "/results_naive_" + str(num_vars) + ".csv"
    results_file_path_lex = folder + "/results_lex_" + str(num_vars) + ".csv"
    results_file_path_rounds = folder + "/results_rounds_" + str(num_vars) + ".csv"
    results_file_path_lex_rounds = folder + "/results_rounds_lex_" + str(num_vars) + ".csv"
    filepaths = [results_file_path_naive, results_file_path_rounds, 
        results_file_path_lex, results_file_path_lex_rounds]
    if use_mixed_on_pure:
        path1 = folder + "/results_naive_mixed_" + str(num_vars) + ".csv"
        path2 = folder + "/results_lex_mixed_" + str(num_vars) + ".csv"
        path3 = folder + "/results_rounds_mixed_" + str(num_vars) + ".csv"
        path4 = folder + "/results_rounds_lex_mixed_" + str(num_vars) + ".csv"
        filepaths.extend([path1, path2, path3, path4])
    create_results_files(filepaths)
    return filepaths

def create_results_files(file_array):
    for fn in file_array:
        f = open(fn, 'w')
        f.write('problem_num, num_cuts,num_constr,det,obj\n')
        f.close()
    return 0


def write_data_line(filepath, line, j):
    line_to_write = str(str(j) + "," + line[0]) + ","  + str(line[1]) + "," + str(
            line[2]) + "," + str(line[3])+ "\n"
    f = open(filepath, 'a')
    f.write(line_to_write)
    f.close()


def write_temp_input(folder):
    d = {}
    d["parameters"] = {}
    d["parameters"]["model"] = folder + "/generated_problem.lp"
    d["parameters"]["solution"] = "solution.sol"
    with open(folder + '/temp.json', 'w') as outfile:
        json.dump(d, outfile)
    return 0


def read_last_line(filepath):
    with open(filepath, 'r') as f:
        lines = f.read().splitlines()
        last_line = lines[-1]
    return last_line


if __name__ == "__main__":
    description = 'Generate random feasible problems, solve them and output results.'
    parser = argparse.ArgumentParser(description = description)
    parser.add_argument('folder', type=str, nargs=1,
                        metavar = 'folder',
                        help = 'folder to put statistics in')
    parser.add_argument('num', type=int, nargs=1,
                        metavar = 'num',
                        help = 'number of variables/constraints')
    parser.add_argument('-p', '--p', action="store_true", default=False,
            help = 'use this option to make pure problems')
    parser.add_argument('-m', '--m', action="store_true", default=False,
			help = 'use this option to use mixed cuts on pure problems')

    args = parser.parse_args()
    num = args.num[0]
    folder = args.folder[0]
    pure = args.p
    use_mixed_on_pure = args.m
    if use_mixed_on_pure and not pure:
        print("You are generating mixed problems, and do not need the flag to do mixed cuts")
    main(folder, num, pure, use_mixed_on_pure)
