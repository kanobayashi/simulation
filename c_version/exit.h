#ifndef EXIT_H
#define EXIT_H

typedef struct {
    double pos[2];   // 出口の中心座標 (x, y)
    double width;    // 出口の幅（矩形的に扱う）
    double height;
} Exit;

#endif
