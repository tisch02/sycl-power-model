import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

MEASUREMENTS_PATH = os.path.join(os.getcwd(), "..", "power_model", "measurements")
OUTPUT_PATH = os.path.join(os.getcwd(), "out")    

def integratePower(df, device, start, stop):    
    df = df[(df["timestamp"] >= start) & (df["timestamp"] <= stop)]
    df = df.sort_values(by=['timestamp'])

    vals = np.array([df["timestamp"], df[device]]).T
    i = 0
    energy = 0
    while i < len(vals) - 1:

        t1, p1 = tuple(vals[i])
        t2, p2 = tuple(vals[i + 1])

        dt = abs(t1 - t2)
        pmax = max(p1, p2)
        pmin = max(p1, p2)

        energy += pmin * dt + (pmax - pmin) * dt / 2

        i += 1    

    return energy

def calculateIdle(df, device, before, after):
    df = df[(df["timestamp"] < before) | (df["timestamp"] > after)]
    return np.mean(df[device])

def plotPowerDrawAll(files, out):
    threshold = 1
    measure_device = 0
    measure_device_name = f"power:device={measure_device}" 


    start_wait = 1000
    stop_wait = 1000

    dfs_power = {}    
    devices = ["power:device=" + str(i) for i in range(4)]
    colors = ["tab:red", "gray", "gray", "gray"]

    # Read data from files and cleanup
    for file in files:
        df = pd.read_csv(file)
        df["timestamp"] = (df["timestamp"] - df["timestamp"][0]) / 1000000000
        for col in devices:
            df[col] = df[col] / 1000000
        dfs_power[file] = df

    fix, ax = plt.subplots(nrows=2, ncols=1, figsize=(15,10), dpi=300)

    for key, df in dfs_power.items():
        for device, c in zip(devices, colors):
            ax[0].plot(df["timestamp"], df[device], color=c, linewidth=0.5, alpha=0.2)

    average_df_length = min([len(df.index) for df in dfs_power.values()])
    num_dfs = len(dfs_power.keys())
    df_cols = ["timestamp"] + devices
    average_df = pd.DataFrame(columns=df_cols)
    df_derivative = pd.DataFrame(columns=df_cols)

    for i in range(average_df_length):
        averages = [(sum([df[col][i] for df in dfs_power.values()]) / num_dfs) for col in devices]
        time = sum([df["timestamp"][i] for df in dfs_power.values()]) / num_dfs
        row = [time] + averages
        
        average_df.loc[-1] = row           
        average_df.index = average_df.index + 1
        average_df = average_df.sort_index()
    
    df_derivative["timestamp"] = average_df["timestamp"]
    for device, c in zip(devices, colors):
        ax[0].plot(average_df["timestamp"], average_df[device], color=c, linewidth=3, alpha=1, label=device)

        derivative = abs(np.gradient(average_df[device]))
        kernel = int(len(derivative)/100)  
        derivative = np.convolve(derivative, np.ones(kernel)/kernel, mode='same')   
        df_derivative[device] = derivative
        ax[1].plot(average_df["timestamp"], derivative, color=c, linewidth=3, alpha=1, label=device)

    # Calculate derivative
    df_filter = df_derivative[(df_derivative[measure_device_name] > threshold)]
    stop = df_filter.iloc[0]["timestamp"]    
    start = df_filter.iloc[-1]["timestamp"]
    
    # Calcualte start and stop lines
    start_x = start_wait / 1000
    average_end = np.average([max(df["timestamp"]) for df in dfs_power.values()])
    max_end = max([max(df["timestamp"]) for df in dfs_power.values()])
    stop_x = average_end - (stop_wait / 1000)

    # Integrate Power
    idle = calculateIdle(average_df, measure_device_name, start, stop)
    idle_box = abs(stop - stop_x) * idle
    power1 = (integratePower(average_df, measure_device_name, start, stop) - idle_box) / (stop_x - start_x)    
    power2 = integratePower(average_df, measure_device_name, start_x, stop_x) / (stop_x - start_x)
    
    # LINES
    ax[0].axvline(x = start, linewidth = 3, color = 'tab:green', label = 'threshold start', linestyle="dotted")
    ax[0].axvline(x = stop, linewidth = 3, color = 'tab:green', label = 'threshold stop', linestyle="dotted")
    ax[1].axvline(x = start, linewidth = 3, color = 'tab:green', label = 'threshold start', linestyle="dotted")
    ax[1].axvline(x = stop, linewidth = 3, color = 'tab:green', label = 'threshold stop', linestyle="dotted")

    ax[0].axvline(x = start_x, color = 'tab:blue', linewidth = 3, label = 'kernel start', linestyle="dotted")
    # ax[0].axvline(x = average_end, color = 'black', label = 'start')
    ax[0].axvline(x = stop_x, color = 'tab:blue', linewidth = 3, label = 'kernel stop', linestyle="dotted")

    ax[0].axhline(y = power1, color = 'tab:green', label = 'energy/s advanced', linewidth=2)
    ax[0].axhline(y = power2, color = 'tab:blue', label = 'energy/s simple', linewidth=2)
    ax[0].axhline(y = idle, color = 'tab:orange', label = 'idle power', linewidth=2)
    
    ax[1].axhline(y = threshold, color='black', linestyle='-')

    ax[0].set_xlabel('Time [s]')
    ax[0].set_ylabel('Power [W]')
    ax[0].legend(loc='lower center', ncols=4) 
    ax[0].grid(True)    
    ax[0].set_xlim(0, max_end)

    ax[1].set_xlabel('Time [s]')
    ax[1].set_ylabel('Change of Power')
    ax[1].legend(loc='upper center', ncols=2) 
    ax[1].grid(True)
    ax[1].set_xlim(0, max_end)

    out_path = os.path.join(OUTPUT_PATH, out)
    plt.savefig(out_path)

def plotPowerEasy(files, out):      
    measure_device = 0
    measure_device_name = f"power:device={measure_device}" 


    start_wait = 1000
    stop_wait = 1000

    dfs_power = {}    
    devices = ["power:device=" + str(i) for i in range(4)]
    colors = ["tab:red", "gray", "gray", "gray"]

    # Read data from files and cleanup
    for file in files:
        df = pd.read_csv(file)
        df["timestamp"] = (df["timestamp"] - df["timestamp"][0]) / 1000000000
        for col in devices:
            df[col] = df[col] / 1000000
        dfs_power[file] = df

    fix, ax = plt.subplots(nrows=2, ncols=1, figsize=(15,10), dpi=300)
    
    for key, df in dfs_power.items():
        for device, c in zip(devices, colors):
            ax[0].plot(df["timestamp"], df[device], color=c, linewidth=0.5, alpha=0.2)

    average_df_length = min([len(df.index) for df in dfs_power.values()])
    num_dfs = len(dfs_power.keys())
    df_cols = ["timestamp"] + devices
    average_df = pd.DataFrame(columns=df_cols)
    df_derivative = pd.DataFrame(columns=df_cols)

    for i in range(average_df_length):
        averages = [(sum([df[col][i] for df in dfs_power.values()]) / num_dfs) for col in devices]
        time = sum([df["timestamp"][i] for df in dfs_power.values()]) / num_dfs
        row = [time] + averages
        
        average_df.loc[-1] = row           
        average_df.index = average_df.index + 1
        average_df = average_df.sort_index()
    
    df_derivative["timestamp"] = average_df["timestamp"]
    for device, c in zip(devices, colors):
        ax[0].plot(average_df["timestamp"], average_df[device], color=c, linewidth=3, alpha=1, label=device)

        derivative = abs(np.gradient(average_df[device]))
        kernel = int(len(derivative)/100)  
        derivative = np.convolve(derivative, np.ones(kernel)/kernel, mode='same')   
        df_derivative[device] = derivative
        ax[1].plot(average_df["timestamp"], derivative, color=c, linewidth=3, alpha=1, label=device)

     
    # Calcualte start and stop lines
    start_x = start_wait / 1000
    average_end = np.average([max(df["timestamp"]) for df in dfs_power.values()])
    max_end = max([max(df["timestamp"]) for df in dfs_power.values()])
    stop_x = average_end - (stop_wait / 1000)

    power2 = integratePower(average_df, measure_device_name, start_x, stop_x) / (stop_x - start_x)
    
    ax[0].axhline(y = power2, color = 'tab:blue', label = 'energy/s', linewidth=2)

    ax[0].axvline(x = start_x, color = 'tab:blue', linewidth = 3, label = 'kernel start', linestyle="dotted")
    # ax[0].axvline(x = average_end, color = 'black', label = 'start')
    ax[0].axvline(x = stop_x, color = 'tab:blue', linewidth = 3, label = 'kernel stop', linestyle="dotted")


    ax[0].set_xlabel('Time [s]')
    ax[0].set_ylabel('Power [W]')
    ax[0].legend(loc='lower center', ncols=4) 
    ax[0].grid(True)
    ax[0].set_xlim(0, max_end)

    ax[1].grid(True)
    ax[1].set_xlim(0, max_end)

    out_path = os.path.join(OUTPUT_PATH, out)
    plt.savefig(out_path)

def getPowerFiles(name, count):
    base_path = os.path.join(MEASUREMENTS_PATH, name)
    power_files = [os.path.join(base_path, f"power_{i}.csv") for i in range(count)]
    counter_file = os.path.join(base_path, f"counter.csv")
    return power_files, counter_file


if __name__ == "__main__":
    power_files, counter_file = getPowerFiles("model/Add/run_0", 10)
    plotPowerDrawAll(power_files, "plot_power_draw_all.png")
    plotPowerEasy(power_files, "plot_power_easy.png")
    