#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <thread>
#include <cmath>
#include <vector>
#include <SDL2/SDL_image.h>

double distance(SDL_Rect one, SDL_Rect two) {
    double deltax = abs(one.x - two.x);
    double deltay = abs(one.y - two.y);
    double c = pow(deltax, 2) + pow(deltay, 2);
    return sqrt(c);
}

double distance(int x1, int y1, int x2, int y2) {
    double deltax = abs(x1 - x2);
    double deltay = abs(y1 - y2);
    double wah = pow(deltax, 2) + pow(deltay, 2);
    return sqrt(wah);
}

const int HEIGHT = 540;
const int WIDTH = 960; 
const int spriteSize = 100;
const int normalAttack = 100;
const int normalSpeed = 10;
class entity {
    public:
    SDL_Rect cam;
    bool isLeft;
    bool isRight;
    bool inBattle;
    bool alive;
    int velo;
    int currentHealth;
    int fullHealth;
    int damage;
    double defence;
    int centerx;
    int centery;
    SDL_Rect BattleRect;
    SDL_Rect healthRect;
    SDL_Rect hitbox;
    SDL_Rect cropRect;
    SDL_Texture *texture;
    entity* opponent;
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
        defence = def;
        centerx = hitbox.x + (hitbox.w / 2);
        centery = hitbox.y + (hitbox.h / 2);
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        if (alive) {
            cam.x = hitbox.x - camera.x;
            cam.y = hitbox.y - camera.y;
            cam.h = hitbox.h;
            cam.w = hitbox.w;
            healthRect.y = cam.y - healthRect.h;
            healthRect.x = cam.x;
            if (inBattle) {
                SDL_RenderCopy(target, texture, &cropRect, &BattleRect);
            } else {
                SDL_RenderCopy(target, texture, &cropRect, &cam);
                SDL_RenderDrawRect(target, &healthRect);
            }
        }
    }
    void battleRender(SDL_Renderer *target) {
        if (alive && inBattle) {
            SDL_RenderCopy(target, texture, &cropRect, &BattleRect);
        }
    }
    void attack(entity *op) {
        if (op->currentHealth <= 0 || currentHealth <= 0) {
            inBattle = false;
            op->inBattle = false;
        } else {
            op->currentHealth -= damage - damage * op->defence;
            op->healthRect.w = 100 * op->currentHealth / op->fullHealth;
            std::cout << "moo\n";
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
            p.currentHealth -= damage - damage * p.defence;
            bound.y = 0;
            p.healthRect.w = 100 * p.currentHealth / p.fullHealth;
        } 
    }
};

class bullet {
    public:
    int damage;
    int velo;
    SDL_Texture* texture;
    SDL_Rect hitbox;
    SDL_Rect cam;
    int dir;
    bullet(int dam, int x, int y, std::string filePath, entity p, SDL_Renderer* target) {
        damage = dam;
        hitbox.x = x;
        hitbox.y = y;
        hitbox.w = 50;
        hitbox.h = 50;
        velo = 10;
        texture = nullptr;
        if (p.isLeft) {
            dir = -1;
        } else {
            dir = 1;
        }
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
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        cam.x = hitbox.x - camera.x;
        cam.y = hitbox.y - camera.y;
        cam.w = hitbox.w;
        cam.h = hitbox.h;
        SDL_RenderCopy(target, texture, NULL, &cam);
    }
    void move() {
        hitbox.x += velo * dir;
    }
};

class player : public entity{
    public:
    SDL_Scancode keys[5];
    map *currentWorld;
    bool offense;
    bool defense;
    bool speed;
    bool normal;
    std::vector<bullet> projectiles;
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
        offense = true;
        defense = false;
        speed = false;
        normal = false;
    }
    void move(const Uint8 *keystate, map *world, SDL_Renderer *target) {
        if (!inBattle) {
            if (keystate[keys[0]]) {
                if (hitbox.y - velo >= 0) {
                    hitbox.y -= velo;
                    centerx = hitbox.x + (hitbox.w / 2);
                    centery = hitbox.y + (hitbox.h / 2);
                }
            } else if (keystate[keys[1]]) {
                if (hitbox.y + velo + spriteSize <= world->height) {
                    hitbox.y += velo;
                    centerx = hitbox.x + (hitbox.w / 2);
                    centery = hitbox.y + (hitbox.h / 2);
                }
            } if (keystate[keys[2]] && hitbox.x - velo >= 0) {
                isLeft = true;
                isRight = false;
                cropRect.x = 66;
                hitbox.x -= velo;
                centerx = hitbox.x + (hitbox.w / 2);
                centery = hitbox.y + (hitbox.h / 2);
            } else if (keystate[keys[3]] && hitbox.x + velo + hitbox.h <= world->width) {
                isLeft = false;
                isRight = true;
                cropRect.x = 0;
                hitbox.x += velo;
                centerx = hitbox.x + (hitbox.w / 2);
                centery = hitbox.y + (hitbox.h / 2);
            } 
            healthRect.x = hitbox.x;
            healthRect.y = hitbox.y - healthRect.h;
            SDL_RenderFillRect(target, &healthRect);
        }
    }
    void changeToAttack() {
        offense = true;
        defense = false;
        speed = false;
        normal = false;
        cropRect.y = 0;
        defence = 0;
        damage = normalAttack * 3;
        velo = normalSpeed * 1.2;
    }
    void changeToDefense() {
        defense = true;
        normal = false;
        offense = false;
        speed = false;
        cropRect.y = 68;
        defence = 0.8;
        damage = normalAttack / 2;
        velo = normalSpeed * 0.9;
    }
    void changeToSpeed() {
        speed = true;
        defense = false;
        offense = false;
        normal = false;
        cropRect.y = 204;
        damage = normalAttack * 0.5;
        defence = 0.3;
        velo = normalSpeed * 3;
    }
    void changeToNormal() {
        normal = true;
        defense = false;
        offense = false;
        speed = false;
        cropRect.y = 136;
        damage = normalAttack;
        defence = 0.5;
        velo = normalSpeed;
    }
};

class Enemy : public entity{
    public:
    int end;
    int start;
    int attackCounter;
    int inity;
    int initx;
    int velo2;
    map *home;
    Enemy(int x, int y, int w, int h, std::string filePath, int endPath, int startPath, int vel, int dam, double def, bool l, bool r, int hlth, SDL_Renderer *target, map *world):entity(x, y, w, h, vel, hlth, dam, def, false, true, target, filePath) {
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
        inity = y;
        initx = x;
        velo2 = velo;
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
                    SDL_RenderDrawRect(target, &healthRect);
                }
            }
        }
    }
    bool goBack() {
        if (hitbox.y != inity){
            return true;
        } else {
            return false;
        }
    }
    void move(map *world, entity p, SDL_Rect camera) {
        if (world == home && alive) {
            cropRect.y = 0;
            bool goback = goBack();
            if (distance(cam, p.cam) > 400) {
                if (!goback) {
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
                } else {
                    if (hitbox.y > inity) {
                        hitbox.y -= velo2;
                    } else if (hitbox.y < inity) {
                        hitbox.y += velo2;
                    }
                    if (hitbox.x > initx) {
                        isLeft = true;
                        isRight = false;
                        hitbox.x -= velo2;
                        cropRect.x = textureWidth / 2;
                    } else if (hitbox.x < initx) {
                        isLeft = false;
                        isRight = true;
                        hitbox.x += velo2;
                        cropRect.x = 0;
                    }
                } 
            } else {
                if (p.hitbox.x > hitbox.x) {
                    isLeft = true;
                    isRight = false;
                    hitbox.x += velo2;
                    cropRect.x = 0;
                } else if (p.hitbox.x < hitbox.x) {
                    isLeft = false;
                    isRight = true;
                    hitbox.x -= velo2;
                    cropRect.x = textureWidth / 2;
                }
                if (p.hitbox.y > hitbox.y) {
                    hitbox.y += velo2;
                } else if (p.hitbox.y < hitbox.y) {
                    hitbox.y -= velo2;
                }
            }
            cam.x = hitbox.x - camera.x;
            cam.y = hitbox.y - camera.y;
            cam.h = hitbox.h;
            cam.w = hitbox.w;
            centerx = cam.x + (cam.w / 2);
            centery = cam.y + (cam.h / 2);
            healthRect.x = cam.x;
            healthRect.y = cam.y - healthRect.h;
        }
    }
    void checkhit(player &p, map *world) {
        if (world == home && alive) {
            int i = 0;
            while (i < p.projectiles.size()) {
                if (!(cam.x < p.projectiles[i].cam.x + p.projectiles[i].cam.w && cam.x + cam.w > p.projectiles[i].cam.x &&
                cam.y < p.projectiles[i].cam.y + p.projectiles[i].cam.h && cam.y + cam.h > p.projectiles[i].cam.y && alive)) {
                    i++;
                } else {
                    currentHealth -= p.projectiles[i].damage;
                    healthRect.w = 100 * currentHealth / fullHealth;
                    p.projectiles.erase(p.projectiles.begin() + i);
                    SDL_DestroyTexture(p.projectiles[i].texture);
                }
            }
            if (cam.x < p.cam.x + p.cam.w && cam.x + cam.w > p.cam.x && cam.y < p.cam.y + p.cam.h && cam.y + cam.h > p.cam.y && alive) {
                p.currentHealth -= damage - damage * p.defence;
                p.healthRect.w = 100 * p.currentHealth / p.fullHealth;
            }
        }
    }
    void revive() {
        alive = true;
        currentHealth = fullHealth;
        healthRect.w = 100;
    }
};

class barrier {
    public:
    SDL_Rect hitbox;
    int currentHp;
    int fullHp;
    double defence;
    SDL_Rect cam;
    SDL_Rect healthRect;
    bool active;
    SDL_Texture *texture;
    void checkhit(player &p, map *world) {
        if (currentHp <= 0) {
            active = false;
        }
        if (active) {
            int i = 0;
            while (i < p.projectiles.size()) {
                if (!(cam.x < p.projectiles[i].cam.x + p.projectiles[i].cam.w && cam.x + cam.w > p.projectiles[i].cam.x &&
                cam.y < p.projectiles[i].cam.y + p.projectiles[i].cam.h && cam.y + cam.h > p.projectiles[i].cam.y)) {
                    i++;
                } else {
                    currentHp -= p.projectiles[i].damage * p.damage * 0.1 * defence;
                    healthRect.w = 100 * currentHp / fullHp;
                    p.projectiles.erase(p.projectiles.begin() + i);
                    SDL_DestroyTexture(p.projectiles[i].texture);
                }
            }
            if (hitbox.x < p.cam.x + p.cam.w && hitbox.x + hitbox.w > p.cam.x && hitbox.y < p.cam.y + p.cam.h && hitbox.y + hitbox.h > p.cam.y) {
                if (p.cam.x < hitbox.x) {
                    p.hitbox.x -= p.velo;
                } else if (p.cam.x > hitbox.x) {
                    p.hitbox.x += p.velo;
                }
                if (p.cam.y > hitbox.y) {
                    p.hitbox.y -= p.velo;
                } else if (p.cam.y < hitbox.y) {
                    p.hitbox.y += p.velo;
                }
                p.currentHealth -= 10;
                p.healthRect.w = 100 * p.currentHealth / p.fullHealth;
            }
        }
    }
    void spawn(int x, int y, int fullhp, double def, std::string filePath, SDL_Renderer *target) {
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
        cam.x = x;
        cam.y = y;
        fullHp = fullhp;
        currentHp = fullHp;
        defence = def;
        cam.w = 30;
        cam.h = 200;
        healthRect.x = cam.x - cam.w;
        healthRect.y = cam.y;
        healthRect.w = 100;
        healthRect.h = 10;
        active = true;
    }
    void render(SDL_Renderer *target, SDL_Rect camera) {
        if (active) {
            hitbox.x = cam.x - camera.x;
            hitbox.y = cam.y - camera.y;
            hitbox.w = cam.w;
            hitbox.h = cam.h;
            SDL_RenderCopy(target, texture, NULL, &hitbox);
        }
    }
};

class regigigas : public Enemy {
    public:
    barrier wall;
    regigigas(int x, int y, int w, int h, std::string filePath, int endPath, int startPath, int vel, int dam, double def, bool l, bool r, int hlth, SDL_Renderer *target, map *world) : Enemy(x,y,w,h,filePath, endPath, startPath, vel, dam,def,l,r, hlth, target, world){
        wall.spawn(cam.x - 30, cam.y, 300, 0.5, "rockwall.jpg", target);
        wall.active = false;
    }
    void spawnBarrier() {
        wall.active = true;
    }
    void render(map *w, SDL_Renderer *target, SDL_Rect camera) {
        if (w == home) {
            if (alive) {
                cam.x = hitbox.x - camera.x;
                cam.y = hitbox.y - camera.y;
                cam.h = hitbox.h;
                cam.w = hitbox.w;
                SDL_RenderCopy(target, texture, &cropRect, &cam);
                SDL_RenderDrawRect(target, &healthRect);
                wall.cam.y = this->hitbox.y;
                wall.render(target, camera);
            }
        }
    }
    void move(map *world, player &p, SDL_Rect camera) {
        Enemy::move(world, p, camera);
        if (isLeft) {
            wall.cam.x = hitbox.x + hitbox.w + wall.cam.w;
        } else if (isRight) {
            wall.cam.x = hitbox.x - wall.cam.w;
        }
        wall.cam.y = this->cam.y;
        wall.checkhit(p, home);
    }
    void checkhit(player &p, map *world) {
        Enemy::checkhit(p, world);
        if (currentHealth != fullHealth) {
            spawnBarrier();
        }
    }
};