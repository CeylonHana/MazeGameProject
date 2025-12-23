#include "maze_config.h"
#include "maze_textures.h"
#include "maze_data.h"
#include "maze_utils.h"
#include "maze_algorithms.h"
#include "maze_render.h"
#include <format> // 用于格式化字符串

int main() {
    // 初始化Raylib窗口
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60); // 设置帧率

    // 加载纹理资源
    MazeTextures* tex = TexturesLoad();
    if (tex == NULL) {
        TraceLog(LOG_ERROR, "纹理加载失败，程序退出");
        CloseWindow();
        return 1;
    }

    // 加载迷宫文件（使用资源文件中的maze0.txt）
    Maze* maze = MazeCreate();
    if (maze == NULL) {
        TraceLog(LOG_ERROR, "迷宫初始化失败，程序退出");
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }
    if (!MazeLoadFromFile(maze, "maze20x20.txt")) { // 替换为资源文件中的maze0.txt
        TraceLog(LOG_ERROR, "迷宫文件加载失败，程序退出");
        TexturesUnload(tex);
        MazeDestroy(maze);
        CloseWindow();
        return 1;
    }

    // 路径相关变量
    PathData* pd = NULL;
    Point path[400]; // 存储路径（最大400个节点）
    int pathLen = 0;
    int algoType = 0; // 0=无路径, 1=DFS, 2=BFS, 3=Dijkstra
    // ---------------------- 角色动画+移动核心变量（重点修改） ----------------------
       // 精灵图参数（4行3列：下0/右1/左2/上3）
    const int FRAME_COUNT_PER_DIR = 3;
    const int DIR_COUNT = 4;
    float character_width = (float)tex->character.width / FRAME_COUNT_PER_DIR;
    float character_height = (float)tex->character.height / DIR_COUNT;
    Rectangle frame_rect = { 0.0F, 0.0F, character_width, character_height };

    // 移动相关（核心修改：像素级移动）
    Vector2 player_pos;          // 角色当前像素坐标（浮点型，支持平滑移动）
    Point current_grid_pos;      // 角色当前所在格子（行列）
    Point target_grid_pos;       // 角色目标格子（行列）
    const float MOVE_SPEED_PX = 200.0F; // 固定移动速度：200像素/秒（可调整）
    bool is_moving = false;      // 是否正在像素级移动
    int curr_dir = 0;            // 当前方向：0下/1右/2左/3上
    int curr_frame = 0;          // 当前动画帧
    float anim_timer = 0.0F;     // 动画计时器
    const float FRAME_DURATION = 0.2F; // 每帧动画时长

    // 游戏状态变量
    bool game_start = false;
    bool game_win = false;
    bool game_over = false;
    int lava_step = 0;

    // 初始化角色位置
    current_grid_pos = maze->start;
    target_grid_pos = current_grid_pos;
    player_pos = GetCellPixelPos(maze, current_grid_pos.x, current_grid_pos.y);

    // 主循环
    while (!WindowShouldClose()) {
        // 游戏开始逻辑（原有不变）
        if (IsKeyPressed(KEY_SPACE) && !game_start && !game_win && !game_over) {
            game_start = true;
            TraceLog(LOG_INFO, "游戏开始！");
        }

        // ---------------------- 角色平滑移动+动画逻辑（核心修改） ----------------------
        if (game_start && !game_win && !game_over) {
            float delta_time = GetFrameTime(); // 每帧耗时

            // 1. 只有不在移动时，才响应方向键（避免多指令叠加）
            if (!is_moving) {
                Point new_target = current_grid_pos;
                int new_dir = curr_dir;

                // 检测方向键（你的方向映射：下0/右1/左2/上3）
                if (IsKeyDown(KEY_UP)) {
                    new_target.y -= 1;
                    new_dir = 3;
                }
                else if (IsKeyDown(KEY_DOWN)) {
                    new_target.y += 1;
                    new_dir = 0;
                }
                else if (IsKeyDown(KEY_LEFT)) {
                    new_target.x -= 1;
                    new_dir = 2;
                }
                else if (IsKeyDown(KEY_RIGHT)) {
                    new_target.x += 1;
                    new_dir = 1;
                }

                // 校验目标格子是否有效（非墙+在迷宫范围内）
                if (new_target.x != current_grid_pos.x || new_target.y != current_grid_pos.y) {
                    if (IsPointValid(maze, new_target.x, new_target.y) && maze->grid[new_target.y][new_target.x] != CELL_WALL) {
                        target_grid_pos = new_target; // 设置目标格子
                        curr_dir = new_dir;           // 更新方向
                        is_moving = true;             // 开始移动
                        // 切换方向时重置动画帧
                        curr_frame = 0;
                        frame_rect.y = curr_dir * character_height;
                        frame_rect.x = 0;
                    }
                }
            }

            // 2. 像素级平滑移动（核心）
            if (is_moving) {
                // 计算目标格子的像素坐标
                Vector2 target_pos = GetCellPixelPos(maze, target_grid_pos.x, target_grid_pos.y);

                // 计算x/y方向的移动增量（线性插值）
                float move_step = MOVE_SPEED_PX * delta_time;

                // X轴移动
                if (fabs(player_pos.x - target_pos.x) > 1.0F) { // 误差小于1像素则视为到达
                    if (player_pos.x < target_pos.x) player_pos.x += move_step;
                    else player_pos.x -= move_step;
                }
                else {
                    player_pos.x = target_pos.x; // 校准位置
                }

                // Y轴移动
                if (fabs(player_pos.y - target_pos.y) > 1.0F) {
                    if (player_pos.y < target_pos.y) player_pos.y += move_step;
                    else player_pos.y -= move_step;
                }
                else {
                    player_pos.y = target_pos.y; // 校准位置
                }

                // 检查是否到达目标格子
                if (player_pos.x == target_pos.x && player_pos.y == target_pos.y) {
                    current_grid_pos = target_grid_pos; // 更新当前格子
                    is_moving = false;                  // 停止移动

                    // 到达新格子后检测地块类型（草地/熔岩/终点）
                    if (maze->grid[current_grid_pos.y][current_grid_pos.x] == CELL_GRASS) {
                        // 草地不改变移动速度（因为是固定像素速度，如需减速可调整MOVE_SPEED_PX）
                        // 如需草地减速：MOVE_SPEED_PX = 200.0F / 3;
                        // 离开草地恢复：可在非草地时重置MOVE_SPEED_PX
                    }
                    if (maze->grid[current_grid_pos.y][current_grid_pos.x] == CELL_LAVA) {
                        lava_step++;
                        if (lava_step >= 2) {
                            game_over = true;
                            TraceLog(LOG_INFO, "游戏失败！第二次踩到熔岩");
                        }
                    }
                    if (maze->grid[current_grid_pos.y][current_grid_pos.x] == CELL_END) {
                        game_win = true;
                        TraceLog(LOG_INFO, "游戏胜利！到达终点");
                    }
                }

                // 3. 移动时播放动画（流畅无卡顿）
                anim_timer += delta_time;
                if (anim_timer >= FRAME_DURATION) {
                    anim_timer = 0;
                    curr_frame = (curr_frame + 1) % FRAME_COUNT_PER_DIR;
                    frame_rect.x = curr_frame * character_width;
                }
            }
            else {
                // 停止移动时，动画重置为第0帧
                curr_frame = 0;
                frame_rect.x = 0;
            }
        }


        // ---------------------- 算法路径交互逻辑 ----------------------
        if (IsKeyPressed(KEY_ONE)) {
            TraceLog(LOG_INFO, "执行DFS算法");
            if (pd != NULL) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            if (pd && DFS(maze, pd, maze->start.x, maze->start.y)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            } else {
                pathLen = 0;
            }
            algoType = 1;
        }

        if (IsKeyPressed(KEY_TWO)) {
            TraceLog(LOG_INFO, "执行BFS算法");
            if (pd != NULL) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            if (pd && BFS(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            } else {
                pathLen = 0;
            }
            algoType = 2;
        }

        if (IsKeyPressed(KEY_THREE)) {
            TraceLog(LOG_INFO, "执行Dijkstra算法");
            if (pd != NULL) PathDataDestroy(maze, pd);
            pd = PathDataCreate(maze);
            if (pd && Dijkstra(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            } else {
                pathLen = 0;
            }
            algoType = 3;
        }

        if (IsKeyPressed(KEY_ZERO)) {
            TraceLog(LOG_INFO, "清空路径");
            if (pd != NULL) PathDataDestroy(maze, pd);
            pd = NULL;
            pathLen = 0;
            algoType = 0;
        }


        // ---------------------- 渲染逻辑 ----------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 绘制迷宫（纹理版）
        DrawMazeGridWithTexture(maze, tex);

        // 绘制路径（如果有）
        if (pathLen > 0) {
            DrawPathOnTexture(maze, path, pathLen);
        }

        // 绘制角色（仅游戏开始后）
        if (game_start && !game_win && !game_over) {
            // 绘制精灵图的当前关键帧
            DrawTextureRec(tex->character, frame_rect, player_pos, WHITE);
        }

        // 绘制提示文字
        DrawText("MazeGame", 20, 20, 24, BLACK);
        DrawText("ButtonInstruction-", 20, 60, 20, BLACK);
        DrawText("1-DFS  2-BFS  3-Dijkstra  0-ClearPath", 20, 90, 20, BLACK);
        const char* algoName[] = {"NONE", "DFS", "BFS", "Dijkstra"};
        DrawText(std::format("Current: {} | PathLen: {}", algoName[algoType], pathLen).c_str(), 20, 120, 20, RED);

        // 绘制游戏状态文字
        if (!game_start) {
            DrawText("Press SPACE to Start Game!", WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2, 24, BLUE);
        }
        if (game_win) {
            DrawText("Game Win! You reached the end!", WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2, 30, GREEN);
        }
        if (game_over) {
            DrawText("Game Over! Step on lava twice!", WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2, 30, RED);
        }

        EndDrawing();
    }


    // ---------------------- 资源释放 ----------------------
    if (pd != NULL) PathDataDestroy(maze, pd);
    MazeDestroy(maze);
    TexturesUnload(tex);
    CloseWindow();

    return 0;
}


/*// MazeGameProject.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "maze_config.h"
#include "maze_textures.h"
#include "maze_data.h"
#include "maze_utils.h"
#include "maze_algorithms.h"
#include "maze_render.h"

int main() {
    // 初始化Raylib窗口
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60); // 设置帧率

    // 加载纹理资源
    MazeTextures* tex = TexturesLoad();
    if (tex == NULL) {
        TraceLog(LOG_ERROR, "纹理加载失败，程序退出");
        CloseWindow();
        return 1;
    }

    // 加载迷宫文件（请确保maze20x20.txt在同目录）
    Maze* maze = MazeCreate();
    if (maze == NULL) {
        TraceLog(LOG_ERROR, "迷宫初始化失败，程序退出");
        TexturesUnload(tex);
        CloseWindow();
        return 1;
    }
    if (!MazeLoadFromFile(maze, "maze20x20.txt")) {
        TraceLog(LOG_ERROR, "迷宫文件加载失败，程序退出");
        TexturesUnload(tex);
        MazeDestroy(maze);
        CloseWindow();
        return 1;
    }

    // 路径相关变量
    PathData* pd = NULL;
    Point path[400]; // 存储路径（最大400个节点）
    int pathLen = 0;
    int algoType = 0; // 0=无路径, 1=DFS, 2=BFS, 3=Dijkstra

    // 主循环
    while (!WindowShouldClose()) {
        // 按键交互逻辑
        if (IsKeyPressed(KEY_ONE)) {
            TraceLog(LOG_INFO, "执行DFS算法");
            // 释放旧路径数据
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行DFS（从起点开始）
            if (DFS(maze, pd, maze->start.x, maze->start.y)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "DFS未找到有效路径");
            }
            algoType = 1;
        }

        if (IsKeyPressed(KEY_TWO)) {
            TraceLog(LOG_INFO, "执行BFS算法");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行BFS
            if (BFS(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "BFS未找到有效路径");
            }
            algoType = 2;
        }

        if (IsKeyPressed(KEY_THREE)) {
            TraceLog(LOG_INFO, "执行Dijkstra算法");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pd = PathDataCreate(maze);
            if (pd == NULL) {
                TraceLog(LOG_ERROR, "路径数据初始化失败");
                pathLen = 0;
                algoType = 0;
                continue;
            }
            // 执行Dijkstra
            if (Dijkstra(maze, pd)) {
                pathLen = PathBacktrack(maze, pd, path, 400);
            }
            else {
                pathLen = 0;
                TraceLog(LOG_WARNING, "Dijkstra未找到有效路径");
            }
            algoType = 3;
        }

        if (IsKeyPressed(KEY_ZERO)) {
            TraceLog(LOG_INFO, "清空路径");
            if (pd != NULL) {
                PathDataDestroy(maze, pd);
                pd = NULL;
            }
            pathLen = 0;
            algoType = 0;
        }

        // 渲染逻辑
        BeginDrawing();
        ClearBackground(RAYWHITE); // 清空背景

        // 绘制纹理版迷宫
        DrawMazeGridWithTexture(maze, tex);

        // 绘制路径（如果有）
        if (pathLen > 0) {
            DrawPathOnTexture(maze, path, pathLen);
        }

        // 绘制提示文字
        DrawText("MazeGame", 20, 20, 24, BLACK);
        DrawText("ButtonInstruction-", 20, 60, 20, BLACK);
        DrawText("1 - DFS_path   2 - BFS_path   3 - Dijkstra_path   0 - ClearPath", 20, 90, 20, BLACK);

        // 显示当前算法
        const char* algoName[] = { "NONE", "DFS", "BFS", "Dijkstra" };
        DrawText(TextFormat("CurrentAlgorithms-%s | PathLength-%d", algoName[algoType], pathLen), 20, 120, 20, RED);

        EndDrawing();
    }

    // 释放所有资源
    if (pd != NULL) PathDataDestroy(maze, pd);
    MazeDestroy(maze);
    TexturesUnload(tex);
    CloseWindow();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件*/