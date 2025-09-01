import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

MEASUREMENTS_PATH = os.path.join(os.getcwd(), "..", "power_model", "measurements")
OUTPUT_PATH = os.path.join(os.getcwd(), "out")    

def plotPowerDrawAll(files, out):    
    data_dict = {}    
    col_names = ["power:device=" + str(i) for i in range(4)]
    colors = ["tab:blue", "tab:orange", "tab:green", "tab:red"]

    for file in files:
        df = pd.read_csv(file)
        df["timestamp"] = (df["timestamp"] - df["timestamp"][0]) / 1000000000
        for col in col_names:
            df[col] = df[col] / 1000000
        data_dict[file] = df

    fig, axs = plt.subplots(figsize=(20,6), dpi=300)
    for key, df in data_dict.items():
        for col, c in zip(col_names, colors):
            axs.plot(df["timestamp"], df[col], color=c, linewidth=0.5, alpha=0.2)

    average_df_length = min([len(df.index) for df in data_dict.values()])
    num_dfs = len(data_dict.keys())
    df_cols = ["timestamp"] + col_names
    average_df = pd.DataFrame(columns=df_cols)

    for i in range(average_df_length):
        averages = [(sum([df[col][i] for df in data_dict.values()]) / num_dfs) for col in col_names]
        time = sum([df["timestamp"][i] for df in data_dict.values()]) / num_dfs
        row = [time] + averages
        
        average_df.loc[-1] = row           
        average_df.index = average_df.index + 1
        average_df = average_df.sort_index()
    
    for col, c in zip(col_names, colors):
        axs.plot(average_df["timestamp"], average_df[col], color=c, linewidth=3, alpha=1, label=col)        

    axs.set_xlabel('Time [s]')
    axs.set_ylabel('Power [W]')
    axs.legend(loc='lower center', ncols=4) 
    axs.grid(True)

    out_path = os.path.join(OUTPUT_PATH, out)
    plt.savefig(out_path)


def plotPowerDrawAllBox(files, out, boxes = 20):
    # Read and process data
    data_dict = {}    
    devices = ["power:device=" + str(i) for i in range(4)]    

    for file in files:        
        df = pd.read_csv(file)  
        df["timestamp"] = (df["timestamp"] - df["timestamp"][0]) / 1000000000
        for col in devices:
            df[col] = df[col] / 1000000
        data_dict[file] = df

    # Get furthest timestamp    
    max_length = max([df["timestamp"].iloc[-1] for df in data_dict.values()])
    segment_length = max_length / boxes
    segments = [(segment_length * i, segment_length * (i + 1)) for i in range(boxes)]
    
    # values
    x_vals = []      
    y_vals = {}
    for device in devices:
        y_vals[device] = [[] for _ in range(boxes)]    
        
    for i, (start, stop) in enumerate(segments):
        x_vals.append(round(start + (stop - start) / 2, 2))

        for df in data_dict.values():            
            df_d = df[(df["timestamp"] >= start) & (df["timestamp"] <= stop)]
            
            for device in devices:
                y_vals[device][i] += list(df_d[device])

    # Plots

    fig, axs = plt.subplots(nrows=len(devices), ncols=1, figsize=(15, 15), dpi=300)    
    flierprops = dict(marker='x', markerfacecolor='gray', markersize=4, linestyle='none')

    for i, device in enumerate(devices):
        axs[i].boxplot(y_vals[device], flierprops=flierprops)
        #axs[i].violinplot(y_vals[device], showmeans=True, showmedians=False, showextrema=False)
        axs[i].set_xticks([y + 1 for y in range(len(x_vals))], labels=x_vals)
        axs[i].yaxis.grid(True)
        axs[i].set_ylabel(device)

    
    out_path = os.path.join(OUTPUT_PATH, out)
    plt.savefig(out_path)

def getPowerFiles(name, count):
    base_path = os.path.join(MEASUREMENTS_PATH, name)
    power_files = [os.path.join(base_path, f"power_{i}.csv") for i in range(count)]
    counter_file = os.path.join(base_path, f"counter.csv")
    return power_files, counter_file


if __name__ == "__main__":
    power_files, counter_file = getPowerFiles("model/Add/run_0", 10)    
    plotPowerDrawAll(power_files, "power_draw_all.png")
    plotPowerDrawAllBox(power_files, "power_draw_box_plot.png", 20)    
    