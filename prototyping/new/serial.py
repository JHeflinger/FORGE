import json

def globals():
    results = None
    with open("config.json", "r") as file:
        results = json.loads(file.read())
        with open(results["particles"], "r") as pfile:
            results["particles"] = json.loads(pfile.read())
    return results
