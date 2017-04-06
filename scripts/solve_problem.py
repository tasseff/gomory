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


def main(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)
    results_store = {}
    pure = True
    for i in range(2,13):
        results_store = {}
        results_store["num_starting_vars"] = i
        j = 0
        results_file_path_naive = folder + "/results_naive_" + str(i) + ".csv"
        results_file_path_lex = folder + "/results_lex_" + str(i) + ".csv"
        results_file_path_rounds = folder + "/results_rounds_" + str(i) + ".csv"
        results_file_path_lex_rounds = folder + "/results_lexrounds_" + str(i) + ".csv"
        create_results_files([results_file_path_naive, results_file_path_rounds, 
            results_file_path_lex, results_file_path_lex_rounds])
        while(j < 3):
            new_folder_path = folder + "/ex_" + str(i) + "_" + str(j)
            if not os.path.exists(new_folder_path):
                os.makedirs(new_folder_path)            
            (feasible, obj) = make_mip.make_mip(i, i, pure, 
                new_folder_path + "/generated_problem.lp")
            if feasible:
                j += 1
                write_temp_input(new_folder_path)
                if is_trivial(new_folder_path, obj):
                    print("trivial")
                #   continue
                run_gomory(new_folder_path)
                output_intermediate_results(new_folder_path, results_file_path_naive,
                    results_file_path_rounds, results_file_path_lex,
                    results_file_path_lex_rounds, obj, j, results_store)
        write_results_store(results_store)
        if i == 4:
            sys.exit()
    return 0


def write_results_store(results_store):
    for solve_type in results_store:
        path = get_bar_graph_path(solve_type)
        write_bar_graph_data(path, results_store, solve_type)
    return 0


def is_trivial(folder, actual_objective):
    rc = subprocess.check_call(["./RUN_NAIVE.sh", folder])
    (num_cuts, _,_,_,_) = get_stats(folder + "/naive.txt", actual_objective)
    if num_cuts == 0:
        return True
    return False


def create_results_files(file_array):
    for fn in file_array:
        write_first_line(fn)
    return 0


def write_first_line(fn):
    f = open(fn, 'w')
    f.write('problem_num, num_cuts,num_constr,det,obj\n')
    f.close()


def output_intermediate_results(new_folder_path, results_file_path_naive, results_file_path_lex,
    results_file_path_rounds, results_file_path_lex_rounds, actual_objective, j, results_store):
    filepaths = [results_file_path_naive, results_file_path_lex, 
        results_file_path_rounds, results_file_path_lex_rounds]
    lex_last = get_stats(new_folder_path + "/lex.txt", actual_objective)
    rounds_lex_last = get_stats(new_folder_path + "/rounds_lex.txt", actual_objective)
    naive_last = get_stats(new_folder_path + "/naive.txt", actual_objective)
    rounds_last = get_stats(new_folder_path + "/rounds.txt", actual_objective)
    last_lines = [naive_last, lex_last, rounds_last, rounds_lex_last]
    methods = ["naive", "lex", "rounds", "lex_rounds"]
    for m in methods:
        if m not in results_store.keys():
            results_store[m] = 0
    if naive_last[-1] == True : results_store["naive"]  += 1
    if lex_last[-1] == True : results_store["lex"]  += 1
    if rounds_last[-1] == True : results_store["rounds"]  += 1
    if rounds_lex_last[-1] == True : results_store["lex_rounds"]  += 1
    for i, path in enumerate(filepaths):
        write_data_line(path, last_lines[i], j)
    return 0


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


def get_bar_graph_path(solve_method):
    return "bar_graph_" + solve_method + ".csv"


def write_data_line(filepath, line, j):
    line_to_write = str(str(j) + "," + line[0]) + ","  + str(line[1]) + "," + str(
            line[2]) + "," + str(line[3])+ "\n"
    f = open(filepath, 'a')
    f.write(line_to_write)
    f.close()


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
    if test < .0000001 and int(num_cuts) < 10000:
        return True
    return False


def read_last_line(filepath):
    with open(filepath, 'r') as f:
        lines = f.read().splitlines()
        last_line = lines[-1]
    return last_line


def run_gomory(folder):
    rc = subprocess.check_call(["./RUN.sh", folder])
    return 0


def write_temp_input(folder):
    d = {}
    d["parameters"] = {}
    d["parameters"]["model"] = folder + "/generated_problem.lp"
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
    parser.add_argument('-f', '--folder', type=str, nargs=1,
                        metavar= 'folder',
                        help = 'output folder path')
   
    args = parser.parse_args()

    folder = 'temp'
    if args.folder:
        folder = args.folder[0]
    main(folder)
