#include "maze_algorithms.h"

bool DFS(Maze* maze, PathData* pd, int x, int y) {
    // 终止条件：到达终点
    if (x == maze->end.x && y == maze->end.y) return true;
    // 终止条件：坐标无效/已访问/是墙
    if (!IsPointValid(maze, x, y) || pd->visited[y][x] || maze->grid[y][x] == CELL_WALL) return false;

    // 标记为已访问
    pd->visited[y][x] = true;

    // 遍历四个方向（上下左右）
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (DFS(maze, pd, nx, ny)) {
            // 记录父节点
            pd->parent[ny][nx].x = x;
            pd->parent[ny][nx].y = y;
            return true;
        }
    }

    return false;
}

bool BFS(Maze* maze, PathData* pd) {
    // 初始化队列（20x20迷宫最大400个节点）
    Point queue[400];
    int front = 0, rear = 0;

    // 起点入队，标记为已访问
    queue[rear++] = maze->start;
    pd->visited[maze->start.y][maze->start.x] = true;

    // 四个方向
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };

    while (front < rear) {
        Point curr = queue[front++];

        // 到达终点
        if (curr.x == maze->end.x && curr.y == maze->end.y) return true;

        // 遍历四个方向
        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            if (IsPointValid(maze, nx, ny) && !pd->visited[ny][nx] && maze->grid[ny][nx] != CELL_WALL) {
                pd->visited[ny][nx] = true;
                pd->parent[ny][nx] = curr; // 记录父节点
                Point temp = { nx, ny };
                queue[rear++] = temp;

                // 防止队列溢出
                if (rear >= 400) {
                    TraceLog(LOG_WARNING, "BFS队列溢出");
                    break;
                }
            }
        }
    }

    return false; // 无路径
}

bool Dijkstra(Maze* maze, PathData* pd) {
    // 起点距离设为0
    pd->distance[maze->start.y][maze->start.x] = 0;

    // 遍历所有节点
    for (int count = 0; count < maze->rows * maze->cols - 1; count++) {
        // 找到未访问的距离最小的节点
        int minDist = INT_MAX;
        Point u = { -1, -1 };
        for (int y = 0; y < maze->rows; y++) {
            for (int x = 0; x < maze->cols; x++) {
                if (!pd->visited[y][x] && pd->distance[y][x] <= minDist) {
                    minDist = pd->distance[y][x];
                    u.x = x;
                    u.y = y;
                }
            }
        }

        // 所有节点已处理或无可达节点
        if (u.x == -1) break;

        // 标记为已访问
        pd->visited[u.y][u.x] = true;

        // 到达终点可提前退出
        if (u.x == maze->end.x && u.y == maze->end.y) break;

        // 更新邻接节点的距离
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; i++) {
            int nx = u.x + dx[i];
            int ny = u.y + dy[i];
            if (IsPointValid(maze, nx, ny) && !pd->visited[ny][nx] && maze->grid[ny][nx] != CELL_WALL) {
                // 基础版：普通地面成本1，草地/熔岩暂按1处理（进阶可修改）
                int cost = 1;
                if (maze->grid[ny][nx] == CELL_GRASS) cost = 3;
                if (maze->grid[ny][nx] == CELL_LAVA) cost = 1000;

                if (pd->distance[u.y][u.x] != INT_MAX && pd->distance[u.y][u.x] + cost < pd->distance[ny][nx]) {
                    pd->distance[ny][nx] = pd->distance[u.y][u.x] + cost;
                    pd->parent[ny][nx] = u; // 记录父节点
                }
            }
        }
    }

    // 检查终点是否可达
    bool reachable = (pd->distance[maze->end.y][maze->end.x] != INT_MAX);
    if (reachable) {
        TraceLog(LOG_INFO, "Dijkstra最短路径成本：%d", pd->distance[maze->end.y][maze->end.x]);
    }
    else {
        TraceLog(LOG_WARNING, "Dijkstra算法未找到可达路径");
    }
    return reachable;
}


