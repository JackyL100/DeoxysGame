#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <SDL2/SDL_image.h>

const int HEIGHT = 540;
const int WIDTH = 960; 
const int spriteSize = 100;
const int normalAttack = 100;
const int normalSpeed = 10;
class entity {
    protected:
    SDL_Rect cam;
    public:
    bool isLeft;
    bool isRight;
    bool inBattle;
    bool alive;
    int velo;
    int currentHealth;
    int fullHealth;
    int damage;
    double defense;
    SDL_Rect BattleRect;
    SDL_Rect healthRect;
    SDL_Rect hitbox;
    SDL_Rect cropRect;
    SDL_Texture *texture;
    int textureWidth;
    int textureHeight;
    entity(int x, int y, int w, int h, int vel, int hlth, int dam, int def, bool l, bool r, SDL_Renderer *target, std::string filePath) {
        texture = nullptr;
        SDL_Surface *surface = IMG_Load(filePath.c_str());
        if (surface == NULL) {
            std::cout << "Error loading player image to surface" << SDL_GetError() << std::endl;
        } else {
            texture = SDL_CreateTextureFromSurface(target, surface);
            if (texture == NULL) {
                std::cout << "Error creating player texture" << SDL_GetError() << std::endl;
            }
        }
        SDL_FreeSurface(surface);
        SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
        hitbox.x = x;
        hitbox.w = w;
        hitbox.h = h;
        hitbox.y = y;
        cropRect.x = 0;
        cropRect.y = 0;
        isLeft = l;
        isLeft = r;
        inBattle = false;
        alive = true;
        fullHealth = hlth;
        currentHealth = hlth;
        damage = dam;
        velo = vel;
        defense = def;
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        if (alive) {
            cam.x = hitbox.x - camera.x;
            cam.y = hitbox.y - camera.y;
            cam.h = hitbox.h;
            cam.w = hitbox.w;
            if (inBattle) {
                SDL_RenderCopy(target, texture, &cropRect, &BattleRect);
            } else {
                SDL_RenderCopy(target, texture, &cropRect, &cam);
            }
        }
    }
    void battleRender(SDL_Renderer *target) {
        if (alive && inBattle) {
            SDL_RenderCopy(target, texture, &cropRect, &BattleRect);
        }
    }
};

class map {
    public:
    SDL_Texture *bg;
    int height;
    int width;
    SDL_Rect cam;
    int door;
    map(std::string filePath, SDL_Renderer *target) {
        bg = nullptr;
        SDL_Surface *surface = IMG_Load(filePath.c_str());
        if (surface == NULL) {
            std::cout << "Error loading enemy image to surface" << SDL_GetError() << std::endl;
        } else {
            bg = SDL_CreateTextureFromSurface(target, surface);
            if (bg == NULL) {
                std::cout << "Error creating enemy texture" << SDL_GetError() << std::endl;
            }
        }
        SDL_FreeSurface(surface);
        SDL_QueryTexture(bg, NULL, NULL, &width, &height);
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        cam = camera;
        SDL_RenderCopy(target, bg, &cam, NULL);
    }
    void render(SDL_Renderer *target) {
        SDL_RenderCopy(target, bg, NULL, NULL);
    }
    bool checkEnd(entity p) {
        if (p.hitbox.x + p.velo + p.hitbox.h >= width && p.hitbox.y < height && p.isRight) {
            return true;
        } else {
            return false;
        }
    }
    bool checkBeginning(entity p) {
        if (p.hitbox.x - p.velo <= 0 && p.hitbox.y < height && p.isLeft) {
            return true;
        } else {
            return false;
        }
    }
    static void changeWorld(map *current, map newWorld) {
        current = &newWorld;
    }
};

class projectile {
    public:
    int damage;
    SDL_Rect bound;
    SDL_Rect cam;
    int grav;
    bool active;
    SDL_Texture *texture;
    projectile(int dam, int gr, int x, int y, int w, int h, std::string filePath, SDL_Renderer *target) {
        texture = nullptr;
        SDL_Surface *surface = IMG_Load(filePath.c_str());
        if (surface == NULL) {
            std::cout << "Error loading player image to surface" << SDL_GetError() << std::endl;
        } else {
            texture = SDL_CreateTextureFromSurface(target, surface);
            if (texture == NULL) {
                std::cout << "Error creating player texture" << SDL_GetError() << std::endl;
            }
        }
        SDL_FreeSurface(surface);
        damage = dam;
        bound.x = x;
        bound.y = y;
        bound.w = w;
        bound.h = h;
        active = false;
        grav = gr;
    }
    void drop(entity e, entity p, map world) {
        if (p.hitbox.x > e.hitbox.x - 300 && p.hitbox.x < e.hitbox.x + 300 && !p.inBattle) {
            active = true;
            bound.y += grav;
            if (bound.y >= world.height - bound.h) {
                bound.y = 0;
            }
            if ((bound.x > e.hitbox.x + 100 || bound.x < e.hitbox.x - 100) && bound.y == 0) {
                bound.x = e.hitbox.x;
            }
        } else {
            active = false;
            bound.y = 0;
        }
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        if (active) {
            cam.x = bound.x - camera.x;
            cam.y = bound.y - camera.y;
            cam.w = bound.w;
            cam.h = bound.h;
            SDL_RenderCopy(target, texture, NULL, &cam);
        }
    }
    void checkhit(entity &p) {
        if (bound.x < p.hitbox.x + p.hitbox.w && bound.x + bound.w > p.hitbox.x &&
        bound.y < p.hitbox.y + p.hitbox.h && bound.y + bound.h > p.hitbox.y) {
            p.currentHealth -= damage - damage * p.defense;
            bound.y = 0;
            p.healthRect.w = 100 * p.currentHealth / p.fullHealth;
        } 
    }
};

class player : public entity{
    public:
    SDL_Scancode keys[5];
    map *currentWorld;
    player(int x, int w, int h, std::string filePath, int vel, int hlth, int dam, double def, SDL_Renderer *target):entity(x, w, h, 100, vel, hlth, dam, def, false, true, target, filePath) {
        hitbox.y = HEIGHT;
        BattleRect.x = WIDTH / 5;
        BattleRect.y = HEIGHT / 2;
        BattleRect.h = 200;
        BattleRect.w = 200;
        healthRect.x = WIDTH / 5;
        healthRect.y = HEIGHT / 2;
        healthRect.h = 10;
        healthRect.w = 100;
        cropRect.w = 66;
        cropRect.h = 68;
        keys[0] = SDL_SCANCODE_W;
        keys[1] = SDL_SCANCODE_S;
        keys[2] = SDL_SCANCODE_A;
        keys[3] = SDL_SCANCODE_D;
        keys[4] = SDL_SCANCODE_SPACE;
    }
    void move(const Uint8 *keystate, map *world) {
        if (!inBattle) {
            if (keystate[keys[0]]) {
                if (hitbox.y - velo >= 0) {
                    hitbox.y -= velo;
                }
            } else if (keystate[keys[1]]) {
                if (hitbox.y + velo + spriteSize <= world->height) {
                    hitbox.y += velo;
                }
            } if (keystate[keys[2]] && hitbox.x - velo >= 0) {
                isLeft = true;
                isRight = false;
                cropRect.x = 66;
                hitbox.x -= velo;
            } else if (keystate[keys[3]] && hitbox.x + velo + hitbox.h <= world->width) {
                isLeft = false;
                isRight = true;
                cropRect.x = 0;
                hitbox.x += velo;
            } 
        }
    }
    void attack(entity &op) {
        if (op.currentHealth <= 0 || currentHealth <= 0) {
            inBattle = false;
            op.inBattle = false;
        } else {
            op.currentHealth -= damage - damage * op.defense;
            op.healthRect.w = 100 * op.currentHealth / op.fullHealth;
        }
    }
    void changeToAttack() {
        cropRect.y = 0;
        defense = 0;
        damage = normalAttack * 3;
        velo = normalSpeed * 1.2;
    }
    void changeToDefense() {
        cropRect.y = 68;
        defense = 0.8;
        damage = normalAttack / 2;
        velo = normalSpeed * 0.9;
    }
    void changeToSpeed() {
        cropRect.y = 204;
        damage = normalAttack * 0.9;
        defense = 0.3;
        velo = normalSpeed * 2;
    }
    void changeToNormal() {
        cropRect.y = 136;
        damage = normalAttack;
        defense = 0.5;
        velo = normalSpeed;
    }
};

class Enemy : public entity{
    public:
    int end;
    int start;
    int attackCounter;
    map *home;
    Enemy(int x, int y, int w, int h, std::string filePath, int endPath, int startPath, int vel, int dam, double def, bool l, bool r, int hlth, SDL_Renderer *target, map *world):entity(x, y, w, h, vel, hlth, dam, def, false, true, target, filePath) {
        texture = nullptr;
        SDL_Surface *surface = IMG_Load(filePath.c_str());
        if (surface == NULL) {
            std::cout << "Error loading enemy image to surface" << SDL_GetError() << std::endl;
        } else {
            texture = SDL_CreateTextureFromSurface(target, surface);
            if (texture == NULL) {
                std::cout << "Error creating enemy texture" << SDL_GetError() << std::endl;
            }
        }
        SDL_FreeSurface(surface);
        end = endPath;
        start = startPath;
        BattleRect.w = w;
        BattleRect.h = h;
        BattleRect.x = WIDTH * 7 / 10 - BattleRect.w / 2 + 25;
        BattleRect.y = HEIGHT / 5;
        cropRect.w = textureWidth / 2;
        cropRect.h = textureHeight / 2;
        attackCounter = 0;
        healthRect.x = WIDTH * 3 / 10;
        healthRect.y = HEIGHT / 5;
        healthRect.w = 100;
        healthRect.h = 10;
        alive = true;
        home = world;
    }
    void render(map *w, SDL_Renderer *target, SDL_Rect camera) {
        if (w == home) {
            if (alive) {
                cam.x = hitbox.x - camera.x;
                cam.y = hitbox.y - camera.y;
                cam.h = hitbox.h;
                cam.w = hitbox.w;
                if (inBattle) {
                    SDL_RenderCopy(target, texture, &cropRect, &BattleRect);
                } else {
                    SDL_RenderCopy(target, texture, &cropRect, &cam);
                }
            }
        }
    }
    void move(map *world) {
        if (world == home) {
            cropRect.y = 0;
            if (velo > 0) {
                if (hitbox.x < end + velo) {
                    isLeft = false;
                    isRight = true;
                    cropRect.x = 0;
                    hitbox.x += velo;
                } else {
                    velo *= -1;
                    hitbox.x += velo;
                }
            } else {
                if (hitbox.x > start - velo) {
                    isLeft = true;
                    isRight = false;
                    hitbox.x += velo;
                    cropRect.x = textureWidth / 2;
                } else {
                    velo *= -1;
                    hitbox.x += velo;
                }
            }
        }
    }
    void checkhit(player &p, map *world) {
        if (world == home) {
            if (hitbox.x < p.hitbox.x + p.hitbox.w && hitbox.x + hitbox.w > p.hitbox.x &&
            hitbox.y < p.hitbox.y + p.hitbox.h && hitbox.y + p.hitbox.h > p.hitbox.y && alive) {
                inBattle = true;
                p.inBattle = true;
            } 
        }
        
    }
    void attack(player &p) {
        if (inBattle) {
            if (attackCounter > 300) {
                attackCounter = 50;
            }
            if (attackCounter % 75 == 0) {
                p.currentHealth -= damage - damage * p.defense;
                p.healthRect.w = 100 * p.currentHealth / p.fullHealth;
            }
            attackCounter += 1;
        } 
    }
    void revive() {
        alive = true;
        currentHealth = fullHealth;
        healthRect.w = 100;
    }
};
