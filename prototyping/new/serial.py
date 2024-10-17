import json
import pickle

def globals(path="config.json"):
    results = None
    with open(path, "r") as file:
        results = json.loads(file.read())
        with open(results["particles"], "r") as pfile:
            results["particles"] = json.loads(pfile.read())
    return results

def tglobals(path="tconf.json"):
    results = None
    with open(path, "r") as file:
        results = json.loads(file.read())
        results["timing"]["config"] = globals(results["timing"]["config"])
    return results

def savestate(state, path):
    with open(path, 'wb') as f:
        pickle.dump(state, f)

def loadstate(path):
    with open(path, 'rb') as f:
        return pickle.load(f)
