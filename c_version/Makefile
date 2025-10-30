# コンパイラとオプション
CC = gcc
CFLAGS = -Wall -lm

# 出力ファイル名
TARGET = sim

# ソースファイル
SRCS = main.c agent.c

# ビルドルール
all: $(TARGET)

$(TARGET): $(SRCS)
    $(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# クリーンアップ（実行ファイルを削除）
clean:
    del $(TARGET).exe