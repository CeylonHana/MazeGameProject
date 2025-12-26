#include "maze_utils.h"

bool IsPointValid(Maze* maze, int x, int y) {
    return (x >= 0 && x < maze->cols && y >= 0 && y < maze->rows);
}

PathData* PathDataCreate(Maze* maze) {
    PathData* pd = (PathData*)malloc(sizeof(PathData));
    if (pd == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：路径数据结构体");
        return NULL;
    }

    // 初始化访问标记
    pd->visited = (bool**)malloc(maze->rows * sizeof(bool*));
    if (pd->visited == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：访问标记行");
        free(pd);
        return NULL;
    }
    for (int y = 0; y < maze->rows; y++) {
        pd->visited[y] = (bool*)calloc(maze->cols, sizeof(bool));
        if (pd->visited[y] == NULL) {
            TraceLog(LOG_ERROR, "内存分配失败：访问标记列");
            // 释放已分配内存
            for (int j = 0; j < y; j++) {
                free(pd->visited[j]);
            }
            free(pd->visited);
            free(pd);
            return NULL;
        }
    }

    // 初始化父节点
    pd->parent = (Point**)malloc(maze->rows * sizeof(Point*));
    if (pd->parent == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：父节点行");
        // 释放已分配内存
        for (int j = 0; j < maze->rows; j++) {
            free(pd->visited[j]);
        }
        free(pd->visited);
        free(pd);
        return NULL;
    }
    for (int y = 0; y < maze->rows; y++) {
        pd->parent[y] = (Point*)malloc(maze->cols * sizeof(Point));
        if (pd->parent[y] == NULL) {
            TraceLog(LOG_ERROR, "内存分配失败：父节点列");
            // 释放已分配内存
            for (int j = 0; j < maze->rows; j++) {
                free(pd->visited[j]);
            }
            free(pd->visited);
            for (int j = 0; j < y; j++) {
                free(pd->parent[j]);
            }
            free(pd->parent);
            free(pd);
            return NULL;
        }
        for (int x = 0; x < maze->cols; x++) {
            pd->parent[y][x].x = -1; // 父节点初始为无效坐标
            pd->parent[y][x].y = -1;
        }
    }

    // 初始化距离（Dijkstra用）
    pd->distance = (int**)malloc(maze->rows * sizeof(int*));
    if (pd->distance == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：距离行");
        // 释放已分配内存
        for (int j = 0; j < maze->rows; j++) {
            free(pd->visited[j]);
            free(pd->parent[j]);
        }
        free(pd->visited);
        free(pd->parent);
        free(pd);
        return NULL;
    }
    for (int y = 0; y < maze->rows; y++) {
        pd->distance[y] = (int*)malloc(maze->cols * sizeof(int));
        if (pd->distance[y] == NULL) {
            TraceLog(LOG_ERROR, "内存分配失败：距离列");
            // 释放已分配内存
            for (int j = 0; j < maze->rows; j++) {
                free(pd->visited[j]);
                free(pd->parent[j]);
            }
            free(pd->visited);
            free(pd->parent);
            for (int j = 0; j < y; j++) {
                free(pd->distance[j]);
            }
            free(pd->distance);
            free(pd);
            return NULL;
        }
        for (int x = 0; x < maze->cols; x++) {
            pd->distance[y][x] = INT_MAX; // 初始距离为无穷大
        }
    }

    return pd;
}

void PathDataDestroy(Maze* maze, PathData* pd) {
    if (pd == NULL) return;
    // 释放访问标记
    for (int y = 0; y < maze->rows; y++) {
        free(pd->visited[y]);
    }
    free(pd->visited);
    // 释放父节点
    for (int y = 0; y < maze->rows; y++) {
        free(pd->parent[y]);
    }
    free(pd->parent);
    // 释放距离
    for (int y = 0; y < maze->rows; y++) {
        free(pd->distance[y]);
    }
    free(pd->distance);
    free(pd);
}

int PathBacktrack(Maze* maze, PathData* pd, Point* path, int maxPathLen) {
    Point current = maze->end;
    int pathLen = 0;

    // 回溯直到起点
    while (current.x != -1 && current.y != -1 && pathLen < maxPathLen) {
        path[pathLen++] = current;
        current = pd->parent[current.y][current.x];

        // 防止死循环（比如路径未连通）
        if (pathLen >= maxPathLen) {
            TraceLog(LOG_WARNING, "路径回溯长度超过最大值%d", maxPathLen);
            break;
        }
    }

    // 检查是否回溯到起点
    if (pathLen > 0 && (path[pathLen - 1].x != maze->start.x || path[pathLen - 1].y != maze->start.y)) {
        TraceLog(LOG_WARNING, "路径未回溯到起点，可能路径不连通");
        pathLen = 0;
        return pathLen;
    }

    // 反转路径（转为起点到终点）
    for (int i = 0; i < pathLen / 2; i++) {
        Point temp = path[i];
        path[i] = path[pathLen - 1 - i];
        path[pathLen - 1 - i] = temp;
    }

    TraceLog(LOG_INFO, "路径回溯完成，路径长度：%d", pathLen);
    return pathLen;
}