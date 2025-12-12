import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.animation import FuncAnimation, PillowWriter, FFMpegWriter

WIDTH, HEIGHT = 38, 25

# --- CSV読み込み ---
df = pd.read_csv("positions.csv")  # step,id,x,y

# --- ユニークな値を取得 ---
steps = sorted(df['step'].unique())
steps = steps[::100]
agent_ids = sorted(df['id'].unique())

# --- アニメーション用 ---
history = []
for step in steps:
    pos_step = np.full((len(agent_ids), 2), np.nan)
    df_step = df[df['step'] == step]
    for idx, agent_id in enumerate(agent_ids):
        agent_row = df_step[df_step['id']==agent_id]
        if not agent_row.empty:
            pos_step[idx] = agent_row[['x','y']].values[0]
    history.append(pos_step)
history = np.array(history)  # shape: (steps, num_agents, 2)

#CIRCLE
# obstacles = [
#     {"pos": (18.2, 0.8), "radius": 0.4},
#     {"pos": (19.8, 0.8), "radius": 0.4}
# ]

#RECTANGLE
obstacles = [
    {"type": "OBSTACLE_RECTANGLE","pos": (18.2, 0.8), "width": 0.8, "height": 0.8},
    {"type": "OBSTACLE_RECTANGLE","pos": (19.8, 0.8), "width": 0.8, "height": 0.8}
]

# --- 描画準備 ---
fig, ax = plt.subplots()
scat = ax.scatter(history[0,:,0], history[0,:,1])
lines = [ax.plot([], [], lw=1)[0] for _ in agent_ids]
ax.set_xlim(0, WIDTH)
ax.set_ylim(0, HEIGHT)  
ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_title("Agent Movement Over Time")
ax.set_aspect('equal')  

# 障害物を描画
for obs in obstacles:
    if obs["type"] == "OBSTACLE_RECTANGLE":
        x = obs["pos"][0] - obs["width"] / 2
        y = obs["pos"][1] - obs["height"] / 2
        rect = plt.Rectangle((x, y), obs["width"], obs["height"], color='red', alpha=0.5)
        ax.add_patch(rect)
    elif obs["type"] == "OBSTACLE_CIRCLE":
        circle = plt.Circle(obs["pos"], obs["radius"], color='red', alpha=0.5)
        ax.add_patch(circle)



# --- アニメーション関数 軌跡を見たいときはこっち ---
# def animate(i):
#     scat.set_offsets(history[i])
#     for j, line in enumerate(lines):
#         line.set_data(history[:i+1,j,0], history[:i+1,j,1])  # 軌跡
#     return scat, *lines

# --- エージェントの動きだけでいいときはこっち ---
def animate(i):
    scat.set_offsets(history[i])
    return scat,  

ani = FuncAnimation(fig, animate, frames=len(steps), interval=500, blit=True)

ani.save("simulation.gif", writer=PillowWriter(fps=20))
print("✅ GIFを simulation.gif として保存しました！")

# ====== MP4として保存 ======
writer = FFMpegWriter(fps=1, bitrate=1800)
ani.save("simulation.mp4", writer=writer)
print("✅ 動画をanimation.mp4 として保存したよ～～～")


# --- グラフ作成 ---
agent_counts = df.groupby('step')['id'].nunique().reset_index()
agent_counts.columns = ['step', 'num_agents']

initial_agents = agent_counts['num_agents'].iloc[0]     
final_step = agent_counts.loc[agent_counts['num_agents'] == 0, 'step']
evacuation_done_step = int(final_step.iloc[0]) if not final_step.empty else None


plt.figure(figsize=(8, 4))
plt.plot(agent_counts['step'], agent_counts['num_agents'], color='blue', lw=2)
plt.xlabel("Step")
plt.ylabel("Number of Agents")
plt.title("Number of Agents Over Time")
plt.grid(True)
plt.tight_layout()

info_text = f"Initial agents: {initial_agents}"
if evacuation_done_step is not None:
    info_text += f"\nAll evacuated at step: {evacuation_done_step}"
else:
    info_text += "\nAll agents not yet evacuated"

plt.text(0.7, 0.85, info_text, transform=plt.gca().transAxes,
         fontsize=10, bbox=dict(facecolor='white', alpha=0.7, edgecolor='gray'))

plt.savefig("agent_count_over_time.png", dpi=300) 
print("避難時間の折れ線グラフまで出力できたよ")


df = pd.read_csv("results.csv", usecols=["y_position", "avg_steps"])
df["y_position"] = pd.to_numeric(df["y_position"], errors="coerce")
df["avg_steps"] = pd.to_numeric(df["avg_steps"], errors="coerce")
# --- NaN を除外 ---
df = df.dropna(subset=["y_position", "avg_steps"])
df = df.sort_values("y_position")

# --- 折れ線グラフ描画 ---
plt.figure(figsize=(8,5))

plt.plot(df["y_position"], df["avg_steps"], marker="o", color="blue", lw=2)
plt.xlabel("Obstacle Y-coordinate")
plt.ylabel("Average evacuation time (steps)")
plt.title("Relationship between obstacle Y-coordinate and average evacuation time")
plt.grid(True)
plt.tight_layout()

# --- 保存と表示 ---
plt.savefig("results_plot.png", dpi=300)
plt.show()
