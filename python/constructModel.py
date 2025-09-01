import datetime
import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

BASE_PATH = os.path.join(os.getcwd(), "..", "power_model", "measurements")
DEVICE_ID = 0
DEVICE_COUNT = 4

def handleRun(path):
    counter_path = os.path.join(path, "counter.csv")
    df_counter = pd.read_csv(counter_path)

    power_paths = [os.path.join(path, fn) for fn in os.listdir(path) if "power" in fn]
    dfs_power = {}    
    devices = [":device=" + str(i) for i in range(DEVICE_COUNT)]

    for file in power_paths:
        df = pd.read_csv(file)
        df["timestamp"] = (df["timestamp"] - df["timestamp"][0]) / 1000000000
        for device in devices:
            df["power" + device] = df["power" + device] / 1000000
        dfs_power[file] = df

    benchmark = df_counter["benchmark"][0]
    arr = df_counter["arr"][0]
    n = df_counter["n"][0]    
    duration = np.mean(df_counter["duration"])
    sq_insts = np.mean(df_counter[f"rocm:::SQ_INSTS:device={DEVICE_ID}"])
    sq_insts_valu = np.mean(df_counter[f"rocm:::SQ_INSTS_VALU:device={DEVICE_ID}"])
    sq_insts_mfma = np.mean(df_counter[f"rocm:::SQ_INSTS_MFMA:device={DEVICE_ID}"])
    sq_insts_salu = np.mean(df_counter[f"rocm:::SQ_INSTS_SALU:device={DEVICE_ID}"])

    # Multi Val
    energy = [np.average(df_counter["ENERGY" + device]) for device in devices]    
    standard_deviations = [np.average([np.std(df["power" + device]) for df in dfs_power.values()]) for device in devices]

    # Add to results
    return [benchmark, arr, n, duration] + energy + standard_deviations + [sq_insts, sq_insts_valu, sq_insts_mfma, sq_insts_salu]

def handleBenchmark(path):
    benchmarks = [os.path.join(path, dir) for dir in os.listdir(path)]
    return [handleRun(benchmark) for benchmark in benchmarks]    
        

if __name__ == "__main__":
    model_name = "model"
    model_path = os.path.join(BASE_PATH, model_name)

    df_result_cols = ["benchmark", "arr", "n", "duration", "e_d0", "e_d1", "e_d2","e_d3", "p_std_d0", "p_std_d1", "p_std_d2", "p_std_d3", "sq_insts", "sq_insts_valu", "sq_insts_mfma", "sq_insts_salu"]
    df_result = pd.DataFrame(columns=df_result_cols)

    dirs = [os.path.join(model_path, dir) for dir in os.listdir(model_path)]    
    for dir in dirs:        
        df_new = pd.DataFrame(handleBenchmark(dir), columns=df_result_cols)
        df_result = pd.concat([df_result, df_new], ignore_index=True)

    df_result = df_result.sort_values(by=["benchmark", "arr"]).reset_index(drop=True)

    ts = str(datetime.datetime.now()).split(".")[0].replace(":", "-").replace(" ", "_")
    result_path = os.path.join(BASE_PATH, "model_" + ts + ".csv")
    df_result.to_csv(result_path, index=False)    
    print(df_result)