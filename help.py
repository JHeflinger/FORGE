import sys
import os

if (len(sys.argv) <= 1):
    print("Available syntax:")
    print("\tall                            | lists all available commands and their variations extensively")
    print("\thelp audit                     | Performs basic project analysis and suggests any basic missed work")
    exit()

if sys.argv[1] == 'all':
    print("Extensive commands:")
    print("\thelp all")
    print("\thelp audit")
    exit()

def handle():
    precursor = sys.argv[1]
    if (precursor == "audit"):
        print(f"Performing audit in {os.getcwd()}...")
        vulnerabilities = 0

        if not os.path.isdir("src"):
            print("src not detected. Running from wrong location!")
            print("pass -- $(pwd) to test command")
            exit(1)

        # check if settings.fconf exists
        if not os.path.exists("settings.fconf"):
            print("No configuration file detected... implicitly creating one now")
            with open("settings.fconf", 'w') as file:
                file.write("\n")

        # excessive white space
        for root, dirs, files in os.walk("src"):
            for file in files:
                filepath = os.path.join(root, file)
                if ".h" in filepath or ".cpp" in filepath:
                    prev_line = ""
                    with open(filepath, 'r') as file:
                        linecount = 0
                        for line in file:
                            linecount += 1
                            if ((line.strip() == "\n") or (line.strip() == "") or (len(line.strip()) == 0)) and ((prev_line.strip() == "\n") or (prev_line.strip() == "") or (len(prev_line.strip()) == 0)):
                                print("Detected excessive whitespace in " + filepath + " on line " + str(linecount))
                                vulnerabilities += 1
                            prev_line = line
        
        # empty files
        for root, dirs, files in os.walk("src"):
            for file in files:
                filepath = os.path.join(root, file)
                if ".h" in filepath or ".cpp" in filepath:
                    with open(filepath, 'r') as file:
                        lines = file.readlines()
                        if len(lines) < 1:
                            vulnerabilities += 1
                            print("Empty file detected: " + filepath)
                            continue

        # header guards
        for root, dirs, files in os.walk("src"):
            for file in files:
                filepath = os.path.join(root, file)
                if ".h" in filepath:
                    with open(filepath, 'r') as file:
                        lines = file.readlines()
                        if len(lines) < 1:
                            continue
                        found = False
                        if not ("#pragma once" in lines[0]):
                            found = True
                            print("Missing or incorrect header guard (#pragma once) detected in " + filepath)
                        if found:
                            vulnerabilities += 1      

        # relative includes
        for root, dirs, files in os.walk("src"):
            for file in files:
                filepath = os.path.join(root, file)
                if ".h" in filepath or ".cpp" in filepath:
                    with open(filepath, 'r') as file:
                        lines = file.readlines()
                        linenum = 0
                        for line in lines:
                            linenum += 1
                            if "#include \".." in line:
                                print("Relative include path detected on line " + str(linenum) + " in " + filepath + ":")
                                print("    " + line.strip())
                                vulnerabilities += 1

        retval = True
        quality = "\033[32m"
        if (vulnerabilities > 10):
            quality = "\033[31m"
            retval = False
        elif (vulnerabilities > 0):
            quality = "\033[33m"
            retval = False
        print("Audit finished - detected " + quality + str(vulnerabilities) + "\033[0m vulnerabilities")
        return retval
    return False

if (handle() == False):
    print("Invalid command detected - please use help command with no args for a list of available commands")
