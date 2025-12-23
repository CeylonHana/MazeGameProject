#include "maze_config.h"
#include "maze_textures.h"
#include "maze_data.h"
#include "maze_utils.h"
#include "maze_algorithms.h"
#include "maze_render.h"
#include <format> // 用于格式化字符串
#include <cmath> // 用于fabs、round等函数

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

    // ---------------------- 角色核心变量（重点重构） ----------------------
    // 精灵图参数（映射：下0/右1/左2/上3，4行3列）
    const int FRAME_COUNT_PER_DIR = 3;
    const int DIR_COUNT = 4;
    float character_width = (float)tex->character.width / FRAME_COUNT_PER_DIR;
    float character_height = (float)tex->character.height / DIR_COUNT;
    Rectangle frame_rect = { 0.0F, 0.0F, character_width, character_height };

    // 移动参数（纯像素级）
    Vector2 player_pos;          // 角色像素坐标（浮点型，支持任意位置）
    const float MOVE_SPEED = 150.0F; // 基础移动速度（像素/秒）
    float current_speed = MOVE_SPEED; // 当前移动速度（受草地影响）
    bool is_moving = false;      // 是否正在移动
    int curr_dir = 0;            // 当前方向：0下/1右/2左/3上
    int curr_frame = 0;          // 当前动画帧
    float anim_timer = 0.0F;     // 动画计时器
    const float FRAME_DURATION = 0.15F; // 动画帧间隔（更流畅）

    // 游戏状态
    bool game_start = false;
    bool game_win = false;
    bool game_over = false;
    int lava_step = 0;
    bool on_lava = false;        // 标记是否正踩在熔岩上（避免重复计数）

    // 初始化角色位置（起点像素坐标）
    player_pos = GetCellPixelPos(maze, maze->start.x, maze->start.y);

    // 主循环
    while (!WindowShouldClose()) {
        // 游戏开始逻辑（原有不变）
        if (IsKeyPressed(KEY_SPACE) && !game_start && !game_win && !game_over) {
            game_start = true;
            TraceLog(LOG_INFO, "游戏开始！");
        }

        // ---------------------- 纯像素级连续移动逻辑（核心） ----------------------
        if (game_start && !game_win && !game_over) {
            float delta_time = GetFrameTime();
            is_moving = false; // 默认未移动
            Vector2 move_delta = { 0.0F, 0.0F }; // 移动增量（x/y方向）

            // 1. 检测方向键，计算移动增量+更新方向
            if (IsKeyDown(KEY_UP)) {
                move_delta.y = -current_speed * delta_time;
                curr_dir = 3; // 上
                is_moving = true;
            }
            else if (IsKeyDown(KEY_DOWN)) {
                move_delta.y = current_speed * delta_time;
                curr_dir = 0; // 下
                is_moving = true;
            }
            if (IsKeyDown(KEY_LEFT)) {
                move_delta.x = -current_speed * delta_time;
                curr_dir = 1; // 左
                is_moving = true;
            }
            else if (IsKeyDown(KEY_RIGHT)) {
                move_delta.x = current_speed * delta_time;
                curr_dir = 2; // 右
                is_moving = true;
            }

            // 2. 碰撞检测：判断移动后的位置是否是墙（像素级）
            Vector2 new_pos = player_pos;
            new_pos.x += move_delta.x;
            new_pos.y += move_delta.y;

            // 计算角色中心对应的迷宫格子（用于判断地块类型）
            int grid_x = (new_pos.x - ((WINDOW_WIDTH - maze->cols * (CELL_SIZE + CELL_GAP)) / 2)) / (CELL_SIZE + CELL_GAP);
            int grid_y = (new_pos.y - ((WINDOW_HEIGHT - maze->rows * (CELL_SIZE + CELL_GAP)) / 2)) / (CELL_SIZE + CELL_GAP);

            // 校验：坐标在迷宫范围内 + 不是墙 → 才允许移动
            bool can_move = true;
            if (IsPointValid(maze, grid_x, grid_y)) {
                if (maze->grid[grid_y][grid_x] == CELL_WALL) {
                    can_move = false; // 墙，禁止移动
                }
            }
            else {
                can_move = false; // 超出迷宫范围，禁止移动
            }

            // 3. 执行移动（仅当可移动时）
            if (can_move) {
                player_pos = new_pos;
            }

            // 4. 检测当前地块类型（草地/熔岩/终点）
            if (IsPointValid(maze, grid_x, grid_y)) {
                // 草地：速度降为1/3
                if (maze->grid[grid_y][grid_x] == CELL_GRASS) {
                    current_speed = MOVE_SPEED / 3;
                }
                else {
                    current_speed = MOVE_SPEED; // 恢复基础速度
                }

                // 熔岩：第二次踩触发失败（仅当从非熔岩→熔岩时计数）
                bool curr_on_lava = (maze->grid[grid_y][grid_x] == CELL_LAVA);
                if (curr_on_lava && !on_lava) {
                    lava_step++;
                    if (lava_step >= 2) {
                        game_over = true;
                        TraceLog(LOG_INFO, "游戏失败！第二次踩到熔岩");
                    }
                }
                on_lava = curr_on_lava;

                // 终点：到达即胜利
                if (maze->grid[grid_y][grid_x] == CELL_END) {
                    game_win = true;
                    TraceLog(LOG_INFO, "游戏胜利！到达终点");
                }
            }

            // 5. 动画播放（移动时播放，停止则暂停在当前帧）
            frame_rect.y = curr_dir * character_height; // 更新方向行
            if (is_moving) {
                anim_timer += delta_time;
                if (anim_timer >= FRAME_DURATION) {
                    anim_timer = 0;
                    curr_frame = (curr_frame + 1) % FRAME_COUNT_PER_DIR;
                    frame_rect.x = curr_frame * character_width;
                }
            }
            // 停止移动时，动画停在当前帧（不重置）
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