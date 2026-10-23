import os
import pandas as pd
import matplotlib.pyplot as plt

# =============================================================================
# PARAMETERS
# =============================================================================
SUMMARY_CSV_PATH = "results/task_2/summary.csv"
OUTPUT_PLOT_DIR = "results/task_2/weight_plots"

# Dictionary mapping the exact string in your CSV to a (base_method, weight) tuple.
METHOD_MAPPING = {
    "regret_2_nn":              ("regret_2_nn", 1.0),
    "regret_2_nn_w05":          ("regret_2_nn", 0.5),
    "regret_2_nn_w01":              ("regret_2_nn", 0.1),
    "regret_2_nn_w02":          ("regret_2_nn", 0.2),
    "regret_2_nn_w03":          ("regret_2_nn", 0.3),
    "regret_2_nn_w04":          ("regret_2_nn", 0.4),
    "regret_2_nn_w06":          ("regret_2_nn", 0.6),
    "regret_2_nn_w07":          ("regret_2_nn", 0.7),
    "regret_2_nn_w08":          ("regret_2_nn", 0.8),
    "regret_2_nn_w09":          ("regret_2_nn", 0.9),
    "regret_2_greedy_cycle":     ("regret_2_greedy_cycle", 1.0),
    "regret_2_greedy_cycle_w05": ("regret_2_greedy_cycle", 0.5),
    "regret_2_greedy_cycle_w01": ("regret_2_greedy_cycle", 0.1),
    "regret_2_greedy_cycle_w02": ("regret_2_greedy_cycle", 0.2),
    "regret_2_greedy_cycle_w03": ("regret_2_greedy_cycle", 0.3),
    "regret_2_greedy_cycle_w04": ("regret_2_greedy_cycle", 0.4),
    "regret_2_greedy_cycle_w06": ("regret_2_greedy_cycle", 0.6),
    "regret_2_greedy_cycle_w07": ("regret_2_greedy_cycle", 0.7),
    "regret_2_greedy_cycle_w08": ("regret_2_greedy_cycle", 0.8),
    "regret_2_greedy_cycle_w09": ("regret_2_greedy_cycle", 0.9),
}
# =============================================================================

def main():
    # 1. Ensure the output directory exists
    os.makedirs(OUTPUT_PLOT_DIR, exist_ok=True)
    
    if not os.path.exists(SUMMARY_CSV_PATH):
        print(f"Error: Could not find {SUMMARY_CSV_PATH}")
        return

    # 2. Load the data 
    # (Using sep=r'\s+|,|;' to robustly handle spaces, commas, or semicolons)
    df = pd.read_csv(SUMMARY_CSV_PATH, sep=r'\s+|,|;', engine='python')
    
    # Standardize column names in case of case-sensitivity issues
    df.columns = [col.lower() for col in df.columns]

    # 3. Filter and map the methods
    # Keep only rows where the method exists in our mapping dictionary
    df = df[df['method'].isin(METHOD_MAPPING.keys())].copy()
    
    # Extract base method and weight from the mapping
    df['base_method'] = df['method'].apply(lambda x: METHOD_MAPPING[x][0])
    df['weight'] = df['method'].apply(lambda x: METHOD_MAPPING[x][1])

    # 4. Generate a plot for each Instance + Base Method combination
    instances = df['instance'].unique()
    base_methods = df['base_method'].unique()

    for inst in instances:
        for b_method in base_methods:
            # Isolate the data for this specific line plot
            subset = df[(df['instance'] == inst) & (df['base_method'] == b_method)]
            
            # Skip if we don't have enough data to draw a line
            if len(subset) < 2:
                continue
                
            # Sort by weight so the X-axis plots sequentially from 0.0 to 1.0
            subset = subset.sort_values(by='weight')

            # 5. Create the plot
            plt.figure(figsize=(8, 6))
            
            # Plot the three lines
            plt.plot(subset['weight'], subset['min'], marker='o', linewidth=2, label='Min Cost')
            plt.plot(subset['weight'], subset['avg'], marker='s', linewidth=2, label='Avg Cost')
            plt.plot(subset['weight'], subset['max'], marker='^', linewidth=2, label='Max Cost')

            # Formatting
            plt.title(f'Weight Comparison: {b_method} on {inst}', fontsize=14, fontweight='bold')
            plt.xlabel('Regret Weight (W)', fontsize=12)
            plt.ylabel('Tour Cost (Objective Value)', fontsize=12)
            plt.legend(loc='best')
            plt.grid(True, linestyle='--', alpha=0.7)
            
            # Ensure the X-axis ticks match our weights cleanly
            plt.xticks(subset['weight'].unique())

            # 6. Save the plot
            filename = f"weight_cmp_{inst}_{b_method}.png"
            filepath = os.path.join(OUTPUT_PLOT_DIR, filename)
            plt.savefig(filepath, dpi=300, bbox_inches='tight')
            plt.close()
            
            print(f"Saved plot: {filepath}")

if __name__ == "__main__":
    main()