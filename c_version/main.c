#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "model.h"

int main() {
    // 乱数初期化
    srand((unsigned int)time(NULL));
    printf("=== シミュレーション開始 ===\n");
    printf("NUM_AGENTS = %d, WIDTH = %.2f, HEIGHT = %.2f\n", NUM_AGENTS, WIDTH, HEIGHT);

    HumanSimulationModel model;
    init_model(&model, NUM_AGENTS, WIDTH, HEIGHT);

    // 初期状態の確認
    printf("=== 初期化後のエージェント位置 ===\n");
    for (int i = 0; i < model.num_agents; i++) {
        printf("Agent %d: x = %.2f, y = %.2f -> target_x = %.2f, target_y = %.2f\n",
               model.agents[i].id,
               model.agents[i].pos[0],
               model.agents[i].pos[1],
               model.agents[i].target_pos[0],
               model.agents[i].target_pos[1]);
    }

    // --- 通常シミュレーションの出力 (positions.csv) ---
    FILE *fp_pos = fopen("positions.csv", "w");
    if (!fp_pos) {
        perror("Failed to open positions.csv");
        return 1;
    }
    fprintf(fp_pos, "step,id,x,y\n");

    for (int step = 0; step < SIMULATION_STEPS; step++) {
        step_model(&model);

        // デバッグ用: 各ステップの先頭エージェントだけ表示
        if (step % 10 == 0 || step == SIMULATION_STEPS - 1) {
            printf("=== Step %d ===\n", step);
            for (int i = 0; i < model.num_agents; i++) {
                printf("Agent %d: x = %.2f, y = %.2f\n",
                       model.agents[i].id,
                       model.agents[i].pos[0],
                       model.agents[i].pos[1]);
            }
        }

        for (int i = 0; i < model.num_agents; i++) {
            fprintf(fp_pos, "%d,%d,%.2f,%.2f\n",
                    step,
                    model.agents[i].id,
                    model.agents[i].pos[0],
                    model.agents[i].pos[1]);
        }
    }

    fclose(fp_pos);
    free_model(&model);

    printf("=== シミュレーション終了 ===\n");
    printf("positions.csv に出力しました\n");

    // --- 障害物位置を変えた平均避難時間の出力 (results.csv) ---
    int num_positions = 10;
    double y_start = 0;
    double y_step = 0.1;
    double results[num_positions];

    FILE *fp_res = fopen("./results.csv", "w");
    if (!fp_res) {
        perror("Failed to open results.csv");
        return 1;
    }
    fprintf(fp_res, "y_position,avg_steps\n");

    for (int i = 0; i < num_positions; i++) {
        double y = y_start + i * y_step;
        double sum = 0;

        for (int trial = 0; trial < 50; trial++) {
            init_model(&model, NUM_AGENTS, WIDTH, HEIGHT);

            // 障害物位置を設定（y座標だけ変える）
            model.obstacles[0] = (Obstacle){OBSTACLE_CIRCLE, {18.2, y}, 0.4514,
                                            0.0, 0.0, {0.0,0.0}, {0.0,0.0}, 0.0};
            model.obstacles[1] = (Obstacle){OBSTACLE_CIRCLE, {19.8, y}, 0.4514,
                                            0.0, 0.0, {0.0,0.0}, {0.0,0.0}, 0.0};

            int steps = run_simulation(&model);
            sum += steps;

            free_model(&model);
        }

        results[i] = sum / 50.0;
        fprintf(fp_res, "%f,%f\n", y, results[i]);
    }

    fclose(fp_res);
    printf("results.csv に保存しました\n");

    return 0;
}

// gcc main.c model.c agent.c obstacle.c -o sim.exe -lm
// ./sim.exe