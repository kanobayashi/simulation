# import pandas as pd
# import matplotlib.pyplot as plt
# from matplotlib.animation import FuncAnimation
# import matplotlib.patches as patches
# import matplotlib.lines as lines
# import numpy as np # 線分の中点を計算するために追加
# import matplotlib.animation as animation
# from matplotlib.animation import FuncAnimation, FFMpegWriter

# plt.rcParams["font.family"] = "Meiryo" 
# # CSVファイルの読み込み
# df = pd.read_csv("positions.csv")

# # ステップごとにグループ化
# grouped = df.groupby("step")


# # 最終ステップを取得
# max_step = df["step"].max()


# # 描画領域の設定
# fig, ax = plt.subplots(figsize=(10, 5))
# scat = ax.scatter([], [], s=10, c='blue', zorder=5) # zorderを上げてエージェントを手前に

# # 軸の範囲を設定
# # Cコードで定義された障害物が(20.0, 1.0)まであるため、範囲を調整
# max_x = 38
# max_y = 25 # Segmentがy=3.0まであるため
# ax.set_xlim(0, max_x)
# ax.set_ylim(0, max_y)
# ax.set_xlabel("X座標")
# ax.set_ylabel("Y座標")
# ax.set_title("人流シミュレーション")

# ax.set_ylim(0, max_y)

# # ====================================================================
# # --- 障害物描画の定義 ---
# # Cコードの init_model の定義に基づいて障害物データを設定
# # 0: 円 (中心: 10.0, 0.5, 半径: 0.3)
# # 1: 矩形 (中心: 20.0, 1.0, 幅: 2.0, 高さ: 1.0 と仮定)
# # 2: 線分 (始点: 5.0, 0.0, 終点: 5.0, 3.0, 厚み: 0.2)

# # obstacles = [
# #     # 0. 円
# #     patches.Circle((10.0, 0.5), 0.3, color='red', alpha=0.6, zorder=2),
# #     # 1. 矩形 (中心 20.0, 1.0, 幅 2.0, 高さ 1.0)
# #     # matplotlibでは左下隅を指定する必要があるため、中心から半分のサイズを引く
# #     patches.Rectangle((20.0 - 1.0, 1.0 - 0.5), 2.0, 1.0, 
# #                       color='darkgreen', alpha=0.6, zorder=2),
# #     # 2. 線分 (5.0, 0.0) -> (5.0, 3.0)
# #     lines.Line2D([5.0, 5.0], [0.0, 3.0], 
# #                  color='purple', linewidth=5, solid_capstyle='round', zorder=3)
# # ]

# # ====================================================================

# # --- 障害物の描画実行 ---
# # ====================================================================
# # --- 障害物の描画実行 (修正版) ---

# # for obs in obstacles:
# #     # オブジェクトの型をチェック
# #     if isinstance(obs, lines.Line2D):
# #         # Line2D (線分) の場合は add_line を使用
# #         ax.add_line(obs)
# #     else:
# #         # Circle や Rectangle (Patch) の場合は add_patch を使用
# #         ax.add_patch(obs)


# def update(frame):
#     if frame in grouped.groups:
#         data = grouped.get_group(frame)
#         scat.set_offsets(data[["x", "y"]].values)
#         ax.set_title(f"人流シミュレーション")
#     return scat,

# #gifアニメーション作成
# ani = FuncAnimation(fig, update, frames=range(max_step + 1), interval=500, blit=True)





# # CSV読み込み時に、数値変換できないデータを NaN にしておく
# df = pd.read_csv("positions.csv", names=["t", "x", "y"], dtype=str)

# # 数値に変換（変換できない部分は自動で NaN に）
# df["t"] = pd.to_numeric(df["t"], errors="coerce")
# df["x"] = pd.to_numeric(df["x"], errors="coerce")
# df["y"] = pd.to_numeric(df["y"], errors="coerce")

# # NaN 行は削除
# df = df.dropna(subset=["t", "x", "y"]).reset_index(drop=True)

# # ====== 設定 ======
# max_x = 38
# max_y = 25
# # 時間ごとにグループ化
# grouped = df.groupby("t")
# frames = sorted(grouped.groups.keys())  

# # ====== プロット設定 ======
# fig, ax = plt.subplots(figsize=(10, 5))
# scat = ax.scatter([], [], s=10, c='blue', zorder=5)  # エージェントを手前に描画

# ax.set_xlim(0, max_x)
# ax.set_ylim(0, max_y)
# ax.set_xlabel("X座標")
# ax.set_ylabel("Y座標")
# ax.set_title("人流シミュレーション")

# # ====== アニメーション関数 ======
# def init():
#     scat.set_offsets(np.empty((0, 2)))  # 空でも2次元配列にする
#     return scat,

# def update(frame):
#     data = grouped.get_group(frame)
#     points = np.column_stack((data["x"], data["y"]))
#     scat.set_offsets(points)
#     ax.set_title(f"人流シミュレーション（t={frame:.2f}）")
#     return scat,

# # ====== フレーム生成 ======
# frames = sorted(grouped.groups.keys())

# # ====== アニメーション生成 ======
# ani = FuncAnimation(fig, update, frames=frames, init_func=init, blit=True, interval=50)

# # ====== 動画として保存 ======
# writer = FFMpegWriter(fps=20, bitrate=1800)
# ani.save("simulation.mp4", writer=writer)
# print("✅ 動画を simulation.mp4 として保存しました！")
# # 表示
# plt.show()
# # ...







# import pandas as pd
# import matplotlib.pyplot as plt
# from matplotlib.animation import FuncAnimation, PillowWriter, FFMpegWriter
# import matplotlib.patches as patches
# import matplotlib.lines as lines
# import numpy as np

# plt.rcParams["font.family"] = "Meiryo"

# # ====== CSV読み込みと前処理 ======
# df = pd.read_csv("positions.csv", names=["t", "x", "y"], dtype=str)

# # 数値に変換（変換できない部分は NaN に）
# df["t"] = pd.to_numeric(df["t"], errors="coerce")
# df["x"] = pd.to_numeric(df["x"], errors="coerce")
# df["y"] = pd.to_numeric(df["y"], errors="coerce")

# # NaN 行を削除
# df = df.dropna(subset=["t", "x", "y"]).reset_index(drop=True)

# # 時間ごとにグループ化
# grouped = df.groupby("t")
# frames = sorted(grouped.groups.keys())

# # ====== プロット設定 ======
# max_x, max_y = 38, 25
# fig, ax = plt.subplots(figsize=(10, 5))
# scat = ax.scatter([], [], s=10, c='blue', zorder=5)

# ax.set_xlim(0, max_x)
# ax.set_ylim(0, max_y)
# ax.set_xlabel("X座標")
# ax.set_ylabel("Y座標")
# ax.set_title("人流シミュレーション")

# # ====== 障害物の描画（必要ならここで追加） ======
# obstacles = [
#     patches.Circle((10.0, 0.5), 0.3, color='red', alpha=0.6, zorder=2),
#     patches.Rectangle((20.0 - 1.0, 1.0 - 0.5), 2.0, 1.0, color='darkgreen', alpha=0.6, zorder=2),
#     lines.Line2D([5.0, 5.0], [0.0, 3.0], color='purple', linewidth=5, solid_capstyle='round', zorder=3)
# ]

# for obs in obstacles:
#     if isinstance(obs, lines.Line2D):
#         ax.add_line(obs)
#     else:
#         ax.add_patch(obs)

# # ====== アニメーション関数 ======
# def init():
#     scat.set_offsets(np.empty((0, 2)))  # 空でも2次元配列
#     return scat,

# def update(frame):
#     data = grouped.get_group(frame)
#     points = np.column_stack((data["x"], data["y"]))
#     scat.set_offsets(points)
#     ax.set_title(f"人流シミュレーション（t={frame:.2f}）")
#     return scat,

# ani = FuncAnimation(fig, update, frames=frames, init_func=init, blit=True, interval=200)

# # ====== GIFとして保存 ======
# ani.save("simulation.gif", writer=PillowWriter(fps=20))
# print("✅ GIFを simulation.gif として保存しました！")

# # ====== MP4として保存 ======
# writer = FFMpegWriter(fps=20, bitrate=1800)
# ani.save("simulation.mp4", writer=writer)
# print("✅ 動画を simulation.mp4 として保存しました！")

# plt.show()
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

# --- アニメーション用にデータ整形 ---
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

plt.show()

