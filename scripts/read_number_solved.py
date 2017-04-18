import sys

def main(folder):
    file_list = ["bar_graph_naive.csv",                  
        "bar_graph_rounds.csv",                 
        "bar_graph_purging.csv",
        "bar_graph_lex.csv",                      
        "bar_graph_rounds_purging.csv",
        "bar_graph_lex_rounds.csv",             
        "bar_graph_lex_purging.csv",     
        "bar_graph_lex_rounds_purging.csv"]      
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
    main(sys.argv[1])

