#include "maze_textures.h"

MazeTextures* TexturesLoad() {
    MazeTextures* tex = (MazeTextures*)malloc(sizeof(MazeTextures));
    if (tex == NULL) {
        TraceLog(LOG_ERROR, "内存分配失败：纹理结构体");
        return NULL;
    }

    // 加载纹理（请确保图片文件与程序同目录）
    tex->start = LoadTexture("start.png");
    tex->end = LoadTexture("end.png");
    tex->floor = LoadTexture("floor.png");
    tex->wall = LoadTexture("wall.png");
    tex->grass = LoadTexture("grass.png");
    tex->lava = LoadTexture("lava.png");
    tex->character = LoadTexture("character.png");
    tex->slime = LoadTexture("slime.png");

    // 校验加载结果
    Texture2D* texList[] = {&tex->start, &tex->end, &tex->floor, &tex->wall, 
                           &tex->grass, &tex->lava, &tex->character, &tex->slime};
    const char* names[] = {"start", "end", "floor", "wall", "grass", "lava", "character", "slime"};
    bool loadFailed = false;
    for (int i = 0; i < 8; i++) {
        if (texList[i]->id == 0) {
            TraceLog(LOG_ERROR, "纹理加载失败: %s.png", names[i]);
            loadFailed = true;
        }
    }

    // 加载失败则释放已加载的纹理
    if (loadFailed) {
        for (int i = 0; i < 8; i++) {
            if (texList[i]->id != 0) {
                UnloadTexture(*texList[i]);
            }
        }
        free(tex);
        return NULL;
    }

    TraceLog(LOG_INFO, "所有纹理加载成功");
    return tex;
}

void TexturesUnload(MazeTextures* tex) {
    if (tex == NULL) return;
    UnloadTexture(tex->start);
    UnloadTexture(tex->end);
    UnloadTexture(tex->floor);
    UnloadTexture(tex->wall);
    UnloadTexture(tex->grass);
    UnloadTexture(tex->lava);
    UnloadTexture(tex->character);
    UnloadTexture(tex->slime);
    free(tex);
}