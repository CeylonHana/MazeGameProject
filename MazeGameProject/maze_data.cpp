#include "maze_data.h"

Maze* MazeCreate() {
    Maze* maze = (Maze*)malloc(sizeof(Maze));
    if (maze == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：迷宫结构体");
        return NULL;
    }
    maze->rows = 0;
    maze->cols = 0;
    maze->grid = NULL;
    // C++兼容的结构体初始化
    maze->start.x = -1;
    maze->start.y = -1;
    maze->end.x = -1;
    maze->end.y = -1;
    return maze;
}

void MazeDestroy(Maze* maze) {
    if (maze == NULL) return;
    if (maze->grid != NULL) {
        for (int i = 0; i < maze->rows; i++) {
            free(maze->grid[i]);
        }
        free(maze->grid);
    }
    free(maze);
}

bool MazeLoadFromFile(Maze* maze, const char* filename) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, filename, "r"); // fopen_s返回错误码，而非直接返回指针
    if (err != 0 || file == NULL) { // 先判断错误码，再判断指针
        TraceLog(LOG_ERROR, "无法打开迷宫文件: %s", filename);
        return false;
    }

    // 读取行列数
    if (fscanf_s(file, "%d %d", &maze->rows, &maze->cols) != 2) {
        TraceLog(LOG_ERROR, "迷宫文件格式错误（首行需为行列数）");
        fclose(file);
        return false;
    }

    // 检查行列数合法性（基础版限制20x20）
    if (maze->rows <= 0 || maze->cols <= 0 || maze->rows > 20 || maze->cols > 20) {
        TraceLog(LOG_ERROR, "迷宫行列数需为1-20");
        fclose(file);
        return false;
    }

    // 分配网格内存
    maze->grid = (CellType**)malloc(maze->rows * sizeof(CellType*));
    if (maze->grid == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：迷宫网格行");
        fclose(file);
        return false;
    }
    for (int i = 0; i < maze->rows; i++) {
        maze->grid[i] = (CellType*)malloc(maze->cols * sizeof(CellType));
        if (maze->grid[i] == NULL) {
            TraceLog(LOG_ERROR, "内存分配失败：迷宫网格列");
            // 释放已分配的内存
            for (int j = 0; j < i; j++) {
                free(maze->grid[j]);
            }
            free(maze->grid);
            fclose(file);
            return false;
        }
    }

    // 读取网格数据，记录起点/终点
    int startCount = 0, endCount = 0;
    for (int y = 0; y < maze->rows; y++) {
        for (int x = 0; x < maze->cols; x++) {
            int val;
            if (fscanf_s(file, "%d", &val) != 1) {
                TraceLog(LOG_ERROR, "迷宫文件网格数据不完整（行%d列%d）", y + 1, x + 1);
                // 释放内存
                for (int j = 0; j < maze->rows; j++) {
                    free(maze->grid[j]);
                }
                free(maze->grid);
                fclose(file);
                return false;
            }
            maze->grid[y][x] = (CellType)val;

            // 记录起点/终点
            if (maze->grid[y][x] == CELL_START) {
                maze->start.x = x;
                maze->start.y = y;
                startCount++;
            }
            else if (maze->grid[y][x] == CELL_END) {
                maze->end.x = x;
                maze->end.y = y;
                endCount++;
            }
        }
    }
    fclose(file);

    // 校验起点/终点数量
    if (startCount != 1) {
        TraceLog(LOG_ERROR, "迷宫需且仅需1个起点（当前%d个）", startCount);
        return false;
    }
    if (endCount != 1) {
        TraceLog(LOG_ERROR, "迷宫需且仅需1个终点（当前%d个）", endCount);
        return false;
    }

    TraceLog(LOG_INFO, "迷宫文件加载成功: %d行 %d列", maze->rows, maze->cols);
    TraceLog(LOG_INFO, "起点坐标：(%d, %d)  终点坐标：(%d, %d)",
        maze->start.x, maze->start.y, maze->end.x, maze->end.y);
    return true;
}