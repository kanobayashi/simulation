import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import matplotlib.patches as patches
import matplotlib.lines as lines
import numpy as np # 線分の中点を計算するために追加

plt.rcParams["font.family"] = "Meiryo" 
# CSVファイルの読み込み
df = pd.read_csv("positions.csv")

# ステップごとにグループ化
grouped = df.groupby("step")


# 最終ステップを取得
max_step = df["step"].max()


# 描画領域の設定
fig, ax = plt.subplots(figsize=(10, 5))
scat = ax.scatter([], [], s=10, c='blue', zorder=5) # zorderを上げてエージェントを手前に

# 軸の範囲を設定
# Cコードで定義された障害物が(20.0, 1.0)まであるため、範囲を調整
max_x = max(df["x"].max() + 1, 22.0)
max_y = max(df["y"].max() + 1, 5.0) # Segmentがy=3.0まであるため
ax.set_xlim(0, max_x)
ax.set_ylim(0, max_y)
ax.set_xlabel("X座標")
ax.set_ylabel("Y座標")
ax.set_title("人流シミュレーション")

print("max_y=", max_y)
ax.set_ylim(0, max_y)

# ====================================================================
# --- 障害物描画の定義 ---
# Cコードの init_model の定義に基づいて障害物データを設定
# 0: 円 (中心: 10.0, 0.5, 半径: 0.3)
# 1: 矩形 (中心: 20.0, 1.0, 幅: 2.0, 高さ: 1.0 と仮定)
# 2: 線分 (始点: 5.0, 0.0, 終点: 5.0, 3.0, 厚み: 0.2)

obstacles = [
    # 0. 円
    patches.Circle((10.0, 0.5), 0.3, color='red', alpha=0.6, zorder=2),
    # 1. 矩形 (中心 20.0, 1.0, 幅 2.0, 高さ 1.0)
    # matplotlibでは左下隅を指定する必要があるため、中心から半分のサイズを引く
    patches.Rectangle((20.0 - 1.0, 1.0 - 0.5), 2.0, 1.0, 
                      color='darkgreen', alpha=0.6, zorder=2),
    # 2. 線分 (5.0, 0.0) -> (5.0, 3.0)
    lines.Line2D([5.0, 5.0], [0.0, 3.0], 
                 color='purple', linewidth=5, solid_capstyle='round', zorder=3)
]
# ====================================================================

# --- 障害物の描画実行 ---
# ====================================================================
# --- 障害物の描画実行 (修正版) ---

for obs in obstacles:
    # オブジェクトの型をチェック
    if isinstance(obs, lines.Line2D):
        # Line2D (線分) の場合は add_line を使用
        ax.add_line(obs)
    else:
        # Circle や Rectangle (Patch) の場合は add_patch を使用
        ax.add_patch(obs)

# ====================================================================

# 障害物描画部分のコードはアニメーションループの外で一度実行すればOKです。
# ====================================================================


def update(frame):
    if frame in grouped.groups:
        data = grouped.get_group(frame)
        scat.set_offsets(data[["x", "y"]].values)
        ax.set_title(f"人流シミュレーション - ステップ {frame}")
    return scat,

# アニメーション作成
ani = FuncAnimation(fig, update, frames=range(max_step + 1), interval=200, blit=True)

# 表示
plt.show()
# ...