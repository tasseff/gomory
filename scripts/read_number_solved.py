import sys

def main(folder, mixed):
    types = ["naive", "rounds", "purging", "lex", "rounds_purging",
        "lex_rounds", "lex_purging", "lex_rounds_purging"]
    file_list = ["bar_graph_naive.csv",                  
        "bar_graph_rounds.csv",                 
        "bar_graph_purging.csv",
        "bar_graph_lex.csv",                      
        "bar_graph_rounds_purging.csv",
        "bar_graph_lex_rounds.csv",             
        "bar_graph_lex_purging.csv",     
        "bar_graph_lex_rounds_purging.csv"]
    print_lines(folder, file_list)
    
    if mixed:
        file_list = ["bar_graph_naive_mixed.csv",                  
        "bar_graph_rounds_mixed.csv",                 
        "bar_graph_purging_mixed.csv",
        "bar_graph_lex_mixed.csv",                      
        "bar_graph_rounds_purging_mixed.csv",
        "bar_graph_lex_rounds_mixed.csv",             
        "bar_graph_lex_purging_mixed.csv",     
        "bar_graph_lex_rounds_purging_mixed.csv"]
        print_lines(folder, file_list)
    
    cuts_file = folder + "/" + "avg_cuts.csv"
    with open(cuts_file) as f:
        content = f.readlines()
    d = {}
    for i, line in enumerate(content):
        if i == 0:
            continue
        linesplit = line.split(",")
        d[linesplit[0]] = float(linesplit[1].rstrip())
    print_avg_cuts(d, types)
    
    if mixed:
        types = ["naive_mixed", "rounds_mixed", "purging_mixed", "lex_mixed", 
        "rounds_purging_mixed", "lex_rounds_mixed", "lex_purging_mixed", 
        "lex_rounds_purging_mixed"]
    print_avg_cuts(d, types)
    return 0


def print_avg_cuts(d, types):
    avg_cuts = []
    for t in types:
        avg_cuts.append(d[t])
    print(avg_cuts)
    return 0


def print_lines(folder, file_list):     
    solved_list = []
    for name in file_list:
	fname = str(folder) + "/" + str(name)
        with open(fname) as f:
            content = f.readlines()
        num = content[1].split(",")[1]
	num = num.rstrip()
	num = int(num)
	solved_list.append(num)
    print(solved_list)
    return 0


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])

