import json
import pickle

def globals():
    results = None
    with open("config.json", "r") as file:
        results = json.loads(file.read())
        with open(results["particles"], "r") as pfile:
            results["particles"] = json.loads(pfile.read())
    return results

def savestate(state, path):
    with open(path, 'wb') as f:
        pickle.dump(state, f)

def loadstate(path):
    with open(path, 'rb') as f:
        return pickle.load(f)