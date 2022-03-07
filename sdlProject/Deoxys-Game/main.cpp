#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <SDL2/SDL_image.h>
#include "classes.h"

void battle(Enemy &e, player &p, SDL_Renderer *target, map battle) {
    SDL_SetRenderTarget(target, battle.bg);
    e.cropRect.x = 0;
    e.cropRect.y = e.textureHeight / 2;
    p.cropRect.x = 134;
    battle.render(target);
    e.battleRender(target);
    p.battleRender(target);
    SDL_RenderFillRect(target, &e.healthRect);
    SDL_RenderFillRect(target,&p.healthRect);
}

int main() {
    SDL_Init( SDL_INIT_EVERYTHING );
    SDL_Window *window = SDL_CreateWindow( "Hello World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI );
    SDL_Renderer *renderTarget = nullptr;
    SDL_Rect cameraRect;
    cameraRect.x = 0;
    cameraRect.y = 0;
    cameraRect.w = WIDTH;
    cameraRect.h = HEIGHT;
    int worldIndex = 0;
    if ( NULL == window ) {
        std::cout << "Could not create window: " << SDL_GetError( ) << std::endl;
        return 1;
    }
    SDL_Event windowEvent;
    renderTarget = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    map overWorld("bigBg.jpg", renderTarget);
    map Cave("temporaryCave.jpg", renderTarget);
    map Ocean("tempOcean.jpg", renderTarget);
    map battleBg("battleBg.bmp", renderTarget);
    map *listOfWorlds[3] = {&overWorld, &Cave, &Ocean};
    map *currentWorld = listOfWorlds[worldIndex];
    Enemy Rayquaza(500, 150, 100, 100,"RayquazaSheet.png", 900, 400, 7, 50, 0.1, true, false, 1000, renderTarget, &overWorld);
    Enemy Regigigas(400, overWorld.height - 180, 180, 180, "RegigigasSheet.png", WIDTH, 400, 5, 200, 0.5, true, false, 2000, renderTarget, &overWorld);
    Enemy Garchomp(450, Cave.height - spriteSize - 50, 170, 135, "GarchompSheet.png", 900, 450, 15, 150, 0.2, true, false, 750, renderTarget, &Cave);
    Enemy Kyogre(400, Ocean.height - 300, 140, 140, "kyogreSheet.png", 900, 400, 5, 200, 0.3, true, false, 1000, renderTarget, &Ocean);
    player Deoxys(10, 80, 80, "DeoxysSheet.png", 10, 500, 100, 0.5, renderTarget);
    projectile meteor1(50, 6, Rayquaza.hitbox.x, 0, 30, 90, "dracoMeteor2.png",renderTarget);
    projectile meteor2(50, 14, Rayquaza.hitbox.x, 0, 30, 90, "dracoMeteor2.png", renderTarget);
    projectile meteor3(50, 18, Rayquaza.hitbox.x, 0, 30, 90, "dracoMeteor2.png", renderTarget);
    SDL_SetRenderTarget(renderTarget, overWorld.bg);
    const Uint8* keyState;
    bool isRunning = true;
    SDL_SetRenderDrawColor(renderTarget, 255, 0, 0, 255);
    while ( isRunning ) {
        SDL_Delay(30);
        while ( SDL_PollEvent( &windowEvent ) != 0 ) {
            if ( SDL_QUIT == windowEvent.type ) {
                isRunning = false;
            } else if (SDL_KEYDOWN == windowEvent.type) {
                switch(windowEvent.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (Deoxys.inBattle && Rayquaza.inBattle) {
                            Deoxys.attack(Rayquaza);
                        } else if (Deoxys.inBattle && Regigigas.inBattle) {
                            Deoxys.attack(Regigigas);
                        } else if (Deoxys.inBattle && Garchomp.inBattle) {
                            Deoxys.attack(Garchomp);
                        } else if (Deoxys.inBattle && Kyogre.inBattle) {
                            Deoxys.attack(Kyogre);
                        }
                        break;
                    case SDLK_0:
                        Rayquaza.revive();
                        break;
                    case SDLK_9:
                        Regigigas.revive();
                        break;
                    case SDLK_8:
                        Garchomp.revive();
                        break;
                    case SDLK_7:
                        Kyogre.revive();
                        break;
                    case SDLK_r:
                        Deoxys.currentHealth = Deoxys.fullHealth;
                        Deoxys.healthRect.w = 100;
                        break;
                    case SDLK_j:
                        Deoxys.changeToAttack();
                        break;
                    case SDLK_i:
                        Deoxys.changeToDefense();
                        break;
                    case SDLK_l:
                        Deoxys.changeToSpeed();
                        break;
                    case SDLK_k:
                        Deoxys.changeToNormal();
                        break;
                }
            } 
        }
        keyState = SDL_GetKeyboardState(NULL);
        Deoxys.move(keyState, currentWorld);
        if (overWorld.checkEnd(Deoxys)) {
            if (worldIndex < 2) {
                worldIndex += 1;
                currentWorld = listOfWorlds[worldIndex];
                Deoxys.hitbox.x = 10;
                Deoxys.hitbox.y = currentWorld->height / 2;
            }
        } else if (overWorld.checkBeginning(Deoxys)) {
            if (worldIndex > 0) {
                worldIndex -= 1;
                currentWorld = listOfWorlds[worldIndex];
                Deoxys.hitbox.x = currentWorld->width - Deoxys.hitbox.h;
                Deoxys.hitbox.y = currentWorld->height / 2;
            }
        }
        Rayquaza.checkhit(Deoxys, currentWorld);
        Regigigas.checkhit(Deoxys, currentWorld);
        Garchomp.checkhit(Deoxys, currentWorld);
        Kyogre.checkhit(Deoxys, currentWorld);
        Rayquaza.attack(Deoxys);
        Regigigas.attack(Deoxys);
        Garchomp.attack(Deoxys);
        Kyogre.attack(Deoxys);
        cameraRect.x = Deoxys.hitbox.x + 50 - WIDTH / 2;
        cameraRect.y = Deoxys.hitbox.y + 50 - HEIGHT / 2;

        if (cameraRect.x < 0) {
            cameraRect.x = 0;
        }
        if (cameraRect.y < 0) {
            cameraRect.y = 0;
        }
        if (cameraRect.x + cameraRect.w >= currentWorld->width) {
            cameraRect.x = currentWorld->width - WIDTH;
        }
        if (cameraRect.y + cameraRect.h >= currentWorld->height) {
            cameraRect.y = currentWorld->height - HEIGHT;
        }
        SDL_RenderClear(renderTarget);
        if (Rayquaza.alive && Rayquaza.home == currentWorld) {
            meteor1.drop(Rayquaza,Deoxys, overWorld);
            meteor2.drop(Rayquaza,Deoxys, overWorld);
            meteor3.drop(Rayquaza,Deoxys, overWorld);
            meteor1.checkhit(Deoxys);
            meteor2.checkhit(Deoxys);
            meteor3.checkhit(Deoxys);
        }
        if (Rayquaza.inBattle && Deoxys.inBattle) {
            battle(Rayquaza,Deoxys,renderTarget,battleBg);
        } else if (Regigigas.inBattle && Deoxys.inBattle) {
            battle(Regigigas,Deoxys,renderTarget,battleBg);
        } else if (Garchomp.inBattle && Deoxys.inBattle) {
            battle(Garchomp,Deoxys,renderTarget,battleBg);
        } else if (Kyogre.inBattle && Deoxys.inBattle) {
            battle(Kyogre,Deoxys,renderTarget,battleBg);
        } else {
            SDL_SetRenderTarget(renderTarget, currentWorld->bg);
            Rayquaza.move(currentWorld);
            Regigigas.move(currentWorld);
            Garchomp.move(currentWorld);
            Kyogre.move(currentWorld);
            currentWorld->render(renderTarget, cameraRect);
            Deoxys.render(renderTarget, cameraRect);
            Rayquaza.render(currentWorld, renderTarget, cameraRect);
            Regigigas.render(currentWorld, renderTarget, cameraRect);
            Garchomp.render(currentWorld, renderTarget, cameraRect);
            Kyogre.render(currentWorld, renderTarget, cameraRect);
            if (Rayquaza.alive && Rayquaza.home == currentWorld) {
                meteor1.render(renderTarget,cameraRect);
                meteor2.render(renderTarget,cameraRect);
                meteor3.render(renderTarget,cameraRect);
            }
        }
        SDL_RenderPresent(renderTarget);
        if (Deoxys.currentHealth <= 0) {
            std::cout << "Game Over :T\n";
            isRunning = false;
        }
        if (Rayquaza.currentHealth <= 0) {Rayquaza.alive = false;}
        if (Regigigas.currentHealth <= 0) {Regigigas.alive = false;}
        if (Garchomp.currentHealth <= 0) {Garchomp.alive = false;}
        if (Kyogre.currentHealth <= 0) {Kyogre.alive = false;}
    }
    SDL_DestroyTexture(overWorld.bg);
    SDL_DestroyTexture(battleBg.bg);
    SDL_DestroyTexture(Cave.bg);
    SDL_DestroyTexture(Deoxys.texture);
    SDL_DestroyTexture(Rayquaza.texture);
    SDL_DestroyTexture(Regigigas.texture);
    SDL_DestroyTexture(Garchomp.texture);
    SDL_DestroyTexture(meteor1.texture);
    SDL_DestroyTexture(meteor2.texture);
    SDL_DestroyTexture(meteor3.texture);
    SDL_DestroyRenderer(renderTarget);
    SDL_DestroyWindow( window );
    Cave.bg = nullptr;
    overWorld.bg = nullptr;
    Deoxys.texture = nullptr;
    Rayquaza.texture = nullptr;
    Regigigas.texture = nullptr;
    Garchomp.texture = nullptr;
    battleBg.bg = nullptr;
    meteor1.texture = nullptr;
    meteor2.texture = nullptr;
    meteor3.texture = nullptr;
    window = nullptr;
    renderTarget = nullptr;
    SDL_Quit();
    return EXIT_SUCCESS;
}