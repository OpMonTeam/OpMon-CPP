﻿#include "MainFrame.hpp"
#include <iostream>
#include "../start/main.hpp"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <cstring>
#include "../start/StringKeys.hpp"
#include "../start/main.hpp"

#define rLog rlog
#define charLineDialog 33
#define kget StringKeys::get

using namespace std;


namespace MainFrame {
	SDL_Window *frame = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Event events;
	TTF_Font *font = NULL;
	SDL_Rect dialogP = {0,0,0,0};
    SDL_Texture *dialogT = NULL;
    SDL_Rect textPlace = {0,0,0,0};
    SDL_Color noir = {0,0,0};
	bool init = false;

	int printText(SDL_Renderer *renderer, string txt, string line2S){
            SDL_Texture *textUre = NULL;
            SDL_Surface *sfce = NULL;
            SDL_Texture *line2 = NULL;
            SDL_Surface *sfce2 = NULL;
            SDL_Rect posLineTwo;

            SDL_RenderCopy(renderer, dialogT, NULL, &dialogP);

            sfce = TTF_RenderText_Blended(font, txt.c_str(), noir);
            textPlace.h = sfce->h;
            textPlace.w = sfce->w;
            textUre = SDL_CreateTextureFromSurface(renderer, sfce);


            sfce2 = TTF_RenderText_Blended(font, line2S.c_str(), noir);
            posLineTwo.h = sfce2->h;
            posLineTwo.w = sfce2->w;
            posLineTwo.x = textPlace.x;
            posLineTwo.y = textPlace.y + 42;
            line2 = SDL_CreateTextureFromSurface(renderer, sfce2);

            SDL_RenderCopy(renderer, line2, NULL, &posLineTwo);
            SDL_RenderCopy(renderer, textUre, NULL, &textPlace);
            SDL_DestroyTexture(textUre);
            SDL_DestroyTexture(line2);
            SDL_FreeSurface(sfce);
	}

	void open(){

		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) == -1){
			rerrLog << "Erreur d'initialisation de SDL." << endl;
			rlog << "Une erreur fatale s'est produite. Merci de consulter errLog.txt" << endl;
			gererErreur(SDL_GetError(), true);
		}

		if (IMG_Init(IMG_INIT_PNG) < 0){
            rerrLog << "Erreur d'initialisation de SDL_Image" << endl;
            rlog << "Une erreur fatale s'est produite. Merci de consulter errLog.txt" << endl;
            gererErreur(IMG_GetError(), true);

        }

        if(TTF_Init() == -1){
           rerrLog << "Erreur d'initialisation de SDL_ttf" << endl;
           rlog << "Une erreur fatale s'est produite. Merci de consulter errLog.txt" << endl;
           gererErreur(TTF_GetError(), true);
        }

        #ifdef WINDOWS
        font = TTF_OpenFont("ressources\\fonts\\arial.ttf", 32);
        #else
        font = TTF_OpenFont("ressources/fonts/arial.ttf", 32);
        #endif

        if(font == NULL){
            rerrLog << "Erreur d'initialisation de la police d'écriture." << endl;
            rlog << "Une erreur fatale s'est produite. Merci de consulter errLog.txt" << endl;
            gererErreur(TTF_GetError(), true);
        }
		init = true;
		frame = SDL_CreateWindow("Pokémon Regimys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(frame, -1, SDL_RENDERER_ACCELERATED);
		if(frame == NULL || renderer == NULL){
            rerrLog << "Erreur d'ouverture de la fenetre ou du renderer." << endl;
            rlog << "Une erreur fatale s'est produite. Merci de consulter errLog.txt" << endl;
            gererErreur(SDL_GetError(), true);
		}
		SDL_Rect fondP;
        SDL_Rect profP;
        SDL_Texture *fondT;
        SDL_Texture *profT;



        rlog << "[T = " << time(NULL) - Main::startTime << "] - Initialisation des SDL_Rect" << endl;
        fondP.h = 512;
        fondP.w = 512;
        fondP.x = 0;
        fondP.y = 0;
        dialogP.h = 150;
        dialogP.w = 512;
        dialogP.x = 0;
        dialogP.y = 362;
        profP.h = 300;
        profP.w = 180;
        profP.x = (fondP.w / 2) - (profP.w / 2) + 10;
        profP.y = ((fondP.h - dialogP.h) / 2) - (profP.h / 2);
        textPlace.x = 10;
        textPlace.y = 372;

        noir.r = 0;
        noir.g = 0;
        noir.b = 0;
        rlog << "[T = " << time(NULL) - Main::startTime << "] - Initialisation des sprites..." << endl;
        #ifdef WINDOWS
        fondT = IMG_LoadTexture(renderer, "ressources\\backgrounds\\start\\startscene.png");
        profT = IMG_LoadTexture(renderer, "ressources\\sprites\\chara\\jlppc\\jlppc.png");
        dialogT = IMG_LoadTexture(renderer, "ressources\\backgrounds\\dialog\\dialog.png");
        #else
        fondT = IMG_LoadTexture(renderer, "ressources/backgrounds/start/startscene.png");
        profT = IMG_LoadTexture(renderer, "ressources/sprites/chara/jlppc/jlppc.png");
        dialogT = IMG_LoadTexture(renderer, "ressources/backgrounds/dialog/dialog.png");
        #endif
        if(fondT == NULL || profT == NULL || dialogT == NULL){
            gererErreur(string("Erreur lors de l'initialisation d'une image.") + string(SDL_GetError()), false);
        }

        if(SDL_RenderCopy(renderer, fondT, NULL, &fondP) == -1){
            rerrLog << "Erreur lors de l'affichage d'un élément" << endl;
            gererErreur(SDL_GetError(), false);
        }
        if(SDL_RenderCopy(renderer, profT, NULL, &profP) == -1){
            rerrLog << "Erreur lors de l'affichage d'un élément" << endl;
        }
        if(SDL_RenderCopy(renderer, dialogT, NULL, &dialogP) == -1){
            rerrLog << "Erreur lors de l'affichage d'un élément" << endl;
        }
        //TTF_RenderText
        SDL_RenderPresent(renderer);
        rlog << "[T = " << time(NULL) - Main::startTime << "] - Fin des initialisations" << endl;
        rlog << "[T = " << time(NULL) - Main::startTime << "] - Creation des variables utilitaires" << endl;
        bool continuer = true;
        long ancientTick = 0;
        //Phase 0
        string txtP0[] = {kget("jlppc.dialog.start.1"), kget("jlppc.dialog.start.2"), kget("jlppc.dialog.start.3"), kget("jlppc.dialog.start.4"), kget("jlppc.dialog.start.5"), kget("jlppc.dialog.start.6"), kget("jlppc.dialog.start.7"), kget("jlppc.dialog.start.8")};//Deux cases == Deux lignes. 3 cases == Deux lignes + un nouveau dialogue
        //Phase 1
        //TODO
        int sizeOfTxt = 8;
        string txtEnCours[] = {string(" "), string(" ")};
        SDL_Texture *textUre = NULL;
        SDL_Surface *sfce = NULL;
        int line = 0, i = 0, dialog = 0;
        bool changeDialog = false;
        int phase = 0;
        rlog << "[T = " << time(NULL) - Main::startTime << "] - Début de la boucle principale." << endl;
        while(continuer){
            if((SDL_GetTicks() - ancientTick) >= 100){
                ancientTick = SDL_GetTicks();
                SDL_PollEvent(&events);
                switch(events.type){
                case SDL_QUIT:
                    continuer = false;
                    break;
                case SDL_KEYDOWN:
                    switch(events.key.keysym.sym){
                    case SDLK_SPACE:
                        if(changeDialog == false){
                            printText(renderer, txt[dialog], txt[dialog + 1]);
                            txtEnCours[0] = string(" ");
                            txtEnCours[1] = string(" ");
                            line = 0;
                            dialog++;
                            dialog++;
                            i = 0;
                            changeDialog = true;
                            SDL_Delay(50);
                        }else if(dialog != sizeOfTxt){
                            changeDialog = false;
                        }

                        break;
                    case SDLK_ESCAPE:
                        continuer = false;
                        break;
                    }
                    break;
                    case SDL_WINDOWEVENT:
                        switch(events.window.event){
                        case SDL_WINDOWEVENT_RESIZED:
                            break;
                        }
                        break;
                }
                if(phase == 0){
                    if(!changeDialog){
                    if(!(i >= txtP0[line].size())){
                        txtEnCours[line]+=txtP0[line + dialog].c_str()[i];
                        printText(renderer, txtEnCours[0], txtEnCours[1]);
                        i++;
                    }else{
                        if(line == 1){
                            txtEnCours[0] = string(" ");
                            txtEnCours[1] = string(" ");
                            line = 0;
                            dialog++;
                            dialog++;
                            i = 0;
                            changeDialog = true;
                        }else{
                            line++;
                            i = 0;
                        }
                    }
                }
                }

                SDL_RenderPresent(renderer);


            }else{
                SDL_Delay(100 - (SDL_GetTicks() - ancientTick));
            }

        }
        rlog << "[T = " << time(NULL) - Main::startTime << "] - Fin de la boucle." << endl;
        SDL_DestroyTexture(textUre);
        SDL_DestroyTexture(dialogT);
        SDL_DestroyTexture(profT);
        SDL_DestroyTexture(fondT);
        SDL_DestroyWindow(frame);
        TTF_CloseFont(font);
		TTF_Quit();
        atexit(IMG_Quit);
		SDL_Quit();
	}

	void loop(){

	}
}
