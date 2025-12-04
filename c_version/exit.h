#ifndef EXIT_H
#define EXIT_H

// 出口構造体：位置 (x, y) と幅を持つ
typedef struct {
    double pos[2];   // 出口の中心座標 (x, y)
    double width;    // 出口の幅（矩形的に扱う）
    double height;
} Exit;

#endif
