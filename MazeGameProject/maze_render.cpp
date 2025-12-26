#include "maze_render.h"

Vector2 GetCellPixelPos(Maze* maze, int x, int y) {
    // 计算迷宫整体偏移（居中显示）
    int totalGridWidth = maze->cols * (CELL_SIZE + CELL_GAP);
    int totalGridHeight = maze->rows * (CELL_SIZE + CELL_GAP);
    float offsetX = (WINDOW_WIDTH - totalGridWidth) / 2.0f;
    float offsetY = (WINDOW_HEIGHT - totalGridHeight) / 2.0f;

    // 计算单个方块的像素坐标（C++兼容写法）
    Vector2 pixelPos;
    pixelPos.x = offsetX + x * (CELL_SIZE + CELL_GAP);
    pixelPos.y = offsetY + y * (CELL_SIZE + CELL_GAP);
    return pixelPos;
}

void DrawCellWithTexture(Maze* maze, MazeTextures* tex, int x, int y, CellType type) {
    Vector2 pos = GetCellPixelPos(maze, x, y);

    // 根据地块类型选择纹理
    Texture2D* useTex = &tex->floor; // 默认普通地面
    if (type == CELL_START) useTex = &tex->start;
    else if (type == CELL_END) useTex = &tex->end;
    else if (type == CELL_WALL) useTex = &tex->wall;
    else if (type == CELL_GRASS) useTex = &tex->grass;
    else if (type == CELL_LAVA) useTex = &tex->lava;

    // 绘制纹理（缩放至CELL_SIZE大小）- C++兼容写法
    // 1. 初始化原纹理区域
    Rectangle sourceRec;
    sourceRec.x = 0.0f;
    sourceRec.y = 0.0f;
    sourceRec.width = (float)useTex->width;
    sourceRec.height = (float)useTex->height;

    // 2. 初始化目标区域
    Rectangle destRec;
    destRec.x = pos.x;
    destRec.y = pos.y;
    destRec.width = (float)CELL_SIZE;
    destRec.height = (float)CELL_SIZE;

    // 3. 初始化旋转中心
    Vector2 origin;
    origin.x = 0.0f;
    origin.y = 0.0f;

    // 4. 调用DrawTexturePro（参数全部改为C++兼容的临时变量）
    DrawTexturePro(
        *useTex,
        sourceRec,       // 原纹理区域
        destRec,         // 目标区域
        origin,          // 旋转中心
        0.0f,            // 旋转角度
        WHITE            // 颜色（直接用Raylib预定义常量，避免手动初始化）
    );
}

void DrawMazeGridWithTexture(Maze* maze, MazeTextures* tex) {
    for (int y = 0; y < maze->rows; y++) {
        for (int x = 0; x < maze->cols; x++) {
            DrawCellWithTexture(maze, tex, x, y, maze->grid[y][x]);
        }
    }
}

void DrawPathOnTexture(Maze* maze, Point* path, int pathLen) {
    for (int i = 0; i < pathLen; i++) {
        int x = path[i].x;
        int y = path[i].y;
        // 跳过起点和终点（保留原有纹理）
        if (maze->grid[y][x] == CELL_START || maze->grid[y][x] == CELL_END) continue;

        Vector2 pos = GetCellPixelPos(maze, x, y);
        // 绘制半透明黄色路径（覆盖在纹理上）
        DrawRectangle(
            (int)pos.x, (int)pos.y,
            CELL_SIZE, CELL_SIZE,
            COLOR_PATH  // 直接用宏定义，避免手动初始化Color
        );
    }
}