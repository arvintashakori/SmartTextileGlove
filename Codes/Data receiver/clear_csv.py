import os

# Usage: deletes (non-recursively) all .csv files in the folder it is run

dir_name = os.getcwd()
files = os.listdir(dir_name)

for file in files:
    if file.endswith(".csv"):
        os.remove(os.path.join(dir_name, file))