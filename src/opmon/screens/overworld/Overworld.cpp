/*
  Overworld.cpp
  Author : Cyrielle
  Contributors : BAKFR, torq, Navet56
  File under GNU GPL v3.0 license
*/

#define FPS_TICKS 33

#include "Overworld.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "OverworldData.hpp"
#include "src/opmon/core/GameData.hpp"
#include "src/opmon/core/GameStatus.hpp"
#include "src/opmon/core/Player.hpp"
#include "src/opmon/model/Enums.hpp"
#include "src/opmon/view/elements/Map.hpp"
#include "src/opmon/view/elements/Position.hpp"
#include "src/opmon/view/ui/Dialog.hpp"
#include "src/opmon/view/ui/Elements.hpp"
#include "src/opmon/view/ui/Jukebox.hpp"
#include "src/utils/defines.hpp"
#include "src/utils/i18n/Translator.hpp"
#include "src/utils/log.hpp"
#include "src/utils/time.hpp"

namespace OpMon {

    void Overworld::setMusic(std::string const &mus) {
        data.getGameDataPtr()->getJukebox().play(mus);
    }

    bool Overworld::isCameraLocked() { return cameraLock; }

    void Overworld::setCameraLock(bool locked) { cameraLock = locked; }

    void Overworld::moveCamera(Side dir) {
        switch(dir) {
            case Side::TO_UP:
                camera.move(0, -4);
                break;
            case Side::TO_DOWN:
                camera.move(0, 4);
                break;
            case Side::TO_LEFT:
                camera.move(-4, 0);
                break;
            case Side::TO_RIGHT:
                camera.move(4, 0);
                break;
            default:
                break;
        }
    }

    void Overworld::updateCamera() {
        // Note: character is already center on itself:
        // character.getPosition() returns the center of the player's sprite
        const sf::Vector2f &playerPos = character.getPosition();
        const sf::Vector2f &cameraSize = camera.getSize();
        const sf::Vector2i mapSize = current->getDimensions() * 32;

        sf::Vector2f center = camera.getCenter();

        // Only move the camera when the player moves away from the center.
        // "coef" determines the liberty given to the player. a coefficient of 0
        // means the camera is always centered on the player. a coefficient of 1
        // allows the player to reach the screen borders.
        float coef = 0.25;

        // Prevent the camera to display too much void outside the map
        // dimensions. 0 has no restriction; 1 means the camera never display
        // any black zone. A value of 0.7 means the camera can't display more
        // than 30% of out-of-map zone.
        float nearBorderCoef = current->isIndoor() ? 0.7f : 1;

        if(cameraSize.x * 0.9 > mapSize.x) {
            center.x = (float)mapSize.x / 2;
        } else {
            // center around the player
            center.x =
                std::max(center.x, playerPos.x - (cameraSize.x * coef / 2.f));
            center.x =
                std::min(center.x, playerPos.x + (cameraSize.x * coef / 2.f));

            // avoid displaying too much out-of-map zone.
            center.x = std::max(center.x, cameraSize.x * nearBorderCoef / 2.f);
            center.x =
                std::min(center.x, (float)mapSize.x -
                                       (cameraSize.x * nearBorderCoef / 2.f));
        }
        if(cameraSize.y * 0.9 > mapSize.y) {
            center.y = (float)mapSize.y / 2;
        } else {
            // center around the player
            center.y =
                std::max(center.y, playerPos.y - (cameraSize.y * coef / 2.f));
            center.y =
                std::min(center.y, playerPos.y + (cameraSize.y * coef / 2.f));

            // avoid displaying too much out-of-map zone.
            center.y = std::max(center.y, cameraSize.y * nearBorderCoef / 2.f);
            center.y =
                std::min(center.y, (float)mapSize.y -
                                       (cameraSize.y * nearBorderCoef / 2.f));
        }
        camera.setCenter(center);
    }

    void Overworld::resetCamera() { camera.setCenter(character.getPosition()); }

    void Overworld::updateElements() {
        //"i" is the element's id
        for(std::string const &i : current->getAnimatedElements()) {
            data.incrementElementCounter(i);
            if(data.getElementCounter(i) >= data.getElementTextures(i).size()) {
                data.resetElementCounter(i);
            }
            elementsSprites[i].setTexture(data.getCurrentElementTexture(i));
            elementsSprites[i].setPosition(data.getElementPos(i));
        }
    }

    void Overworld::printElements(sf::RenderTarget &frame) const {
        //"i" is the element's id
        for(const std::pair<std::string, sf::Sprite> &spr : elementsSprites) {
            frame.draw(spr.second);
        }
    }

    void Overworld::tp(std::string toTp, sf::Vector2i pos, Side tpDir) {
        this->toTp = toTp;
        tpPos = pos;
        this->tpDir = tpDir;
        fadeCountdown++;
        fadeDir = true;
        character.getPositionMapRef().lockMove();
    }

    void Overworld::tpNoAnim(std::string toTp, sf::Vector2i pos, Side tpDir) {
        data.setCurrentMap(toTp);
        current = data.getCurrentMap();
        character.setPosition(pos.x, pos.y);
        resetCamera();
        setMusic(current->getBg());

        // Recreates the layers
        layer1 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer1(),
            data.getTileset(current->getTileset()));
        layer2 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer2(),
            data.getTileset(current->getTileset()));
        layer3 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer3(),
            data.getTileset(current->getTileset()));

        if(tpDir != Side::NO_MOVE) {
            character.getPositionMapRef().setDir(tpDir);
        }
    }

    void Overworld::pause() { data.getGameDataPtr()->getJukebox().pause(); }

    void Overworld::play() {
        data.getGameDataPtr()->getJukebox().play(current->getBg());
    }

    Overworld::Overworld(const std::string &mapId, OverworldData &data)
        : data(data), character(data.getPlayerEvent()) {
        current = data.getMap(mapId);
        data.setCurrentMap(mapId);
        character.setPosition(2, 4);
        camera.setSize(sf::Vector2f(30 SQUARES, 16.875 SQUARES));
        resetCamera();

        setMusic(current->getBg());
        layer1 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer1(),
            data.getTileset(current->getTileset()));
        layer2 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer2(),
            data.getTileset(current->getTileset()));
        layer3 = std::make_unique<Ui::MapLayer>(
            current->getDimensions(), current->getLayer3(),
            data.getTileset(current->getTileset()));

        data.getGameDataPtr()->getJukebox().play(current->getBg());

        Utils::I18n::Translator::getInstance().setLang(
            Utils::I18n::Translator::getInstance().getLang());

        screenCache =
            sf::RectangleShape(data.getGameDataPtr()->getWindowSize<float>());
        screenCache.setSfmlColor(sf::Color(0, 0, 0, 0));
        screenCache.setOrigin(0, 0);
    }

    void Overworld::draw(sf::RenderTarget &frame,
                         sf::RenderStates states) const {
        bool is_in_dialog = this->dialog && !this->dialog->isDialogOver();
        frame.setView(camera);
        frame.clear(sf::Color::Black);

        // Drawing the two first layers
        if((debugMode ? printlayer[0] : true)) {
            frame.draw(*layer1);
        }
        if((debugMode ? printlayer[1] : true)) {
            frame.draw(*layer2);
        }
        // Drawing events under the player
        for(const Elements::AbstractEvent *event : current->getEvents()) {
            const sf::Sprite *sprite = event->getSprite();
            if(sprite->getPosition().y <= character.getPosition().y) {
                frame.draw(*sprite);
            }
        }

        frame.draw(*character.getSprite());

        // Drawing the events above the player
        for(const Elements::AbstractEvent *event : current->getEvents()) {
            const sf::Sprite *sprite = event->getSprite();
            if(sprite->getPosition().y > character.getPosition().y) {
                frame.draw(*sprite);
            }
        }

        if(debugMode && printCollisions) {
            printCollisionLayer(frame);
        }

        // Drawing the third layer
        if((debugMode ? printlayer[2] : true)) {
            frame.draw(*layer3);
        }

        printElements(frame);

        /***** draw GUI *****/
        frame.setView(frame.getDefaultView());

        if(is_in_dialog) {
            frame.draw(*this->dialog);
        }

        frame.draw(screenCache);

        if(debugMode) {
            frame.draw(debugText);
            frame.draw(fpsPrint);
            frame.draw(coordPrint);
        }
    }

    GameStatus Overworld::update() {
        bool is_in_dialog = this->dialog && !this->dialog->isDialogOver();

        if(initPlayerAnimation) {
            startPlayerAnimationTime = Utils::Time::getElapsedMilliseconds();
            initPlayerAnimation = false;
        }

        fpsCounter++;
        if(Utils::Time::getElapsedMilliseconds() - oldTicksFps >= 1000) {
            fpsPrint.setString(std::to_string(fpsCounter));
            fpsCounter = 0;
            oldTicksFps = Utils::Time::getElapsedMilliseconds();
        }

        if(debugMode) {
            std::cout << "Elapsed Time: " << Utils::Time::getElapsedSeconds()
                      << "s" << std::endl;
            std::cout << "Loop : " << (is_in_dialog ? "Dialog" : "Normal")
                      << std::endl;
            std::cout << "PlayerPosition: "
                      << character.getPositionMap().getPosition().x << " - "
                      << character.getPositionMap().getPosition().y
                      << std::endl;
            std::cout << "PlayerPositionPx: " << character.getPosition().x
                      << " - " << character.getPosition().y << std::endl;
            std::cout << "Moving: "
                      << (character.getPositionMap().isMoving() ? "true" :
                                                                  "false")
                      << std::endl;
            std::cout << "Anim: "
                      << (character.getPositionMap().isAnim() ? "true" :
                                                                "false")
                      << std::endl;
            std::cout << "PlayerDirection: "
                      << (int)character.getPositionMap().getDir() << std::endl;
            std::cout << "Start player Animation Time: "
                      << (double)startPlayerAnimationTime / 1000 << std::endl;

            debugText.setString("Debug mode");
            debugText.setPosition(0, 0);
            debugText.setFont(data.getGameDataPtr()->getFont());
            debugText.setSfmlColor(sf::Color(127, 127, 127));
            debugText.setCharacterSize(40);
            fpsPrint.setPosition(0, 50);
            fpsPrint.setFont(data.getGameDataPtr()->getFont());
            fpsPrint.setCharacterSize(48);
            std::ostringstream oss;
            oss << "Position : " << character.getPositionMap().getPosition().x
                << " - " << character.getPositionMap().getPosition().y
                << std::endl
                << "PxPosition : " << character.getPosition().x << " - "
                << character.getPosition().y << std::endl;
            coordPrint.setString(oss.str());
            coordPrint.setFont(data.getGameDataPtr()->getFont());
            coordPrint.setPosition(0, 100);
            coordPrint.setSfmlColor(sf::Color(127, 127, 127));
            coordPrint.setCharacterSize(30);
        }

        if(is_in_dialog) {
            this->dialog->updateTextAnimation();
        }

        updateCamera();

        // Updates events under the player
        for(Elements::AbstractEvent *event : current->getEvents()) {
            event->updateFrame();
        }

        character.update(*this);
        character.updateFrame();

        // Updates the events above the player
        for(Elements::AbstractEvent *event : current->getEvents()) {
            event->updateFrame();
        }

        updateElements();

        if(fadeCountdown != 0) { // If the fading animation is occuring
            if(fadeCountdown == fadeFrames &&
               fadeDir) { // End of the first phase: teleports the player
                fadeDir = false;
                tpNoAnim(toTp, tpPos, tpDir);
            } else { // Else just continues the fading
                if(fadeDir)
                    fadeCountdown++;
                else
                    fadeCountdown--;
            }
        }
        // Updates the color of the cache
        screenCache.setSfmlColor(
            sf::Color(0, 0, 0, (fadeCountdown / fadeFrames) * 255.0));
        // Updates coordinates
        screenCache.setPosition(
            data.getGameDataPtr()->mapPixelToCoords(sf::Vector2i(0, 0)));

        if(fadeCountdown == 0 && !fadeDir) {
            character.getPositionMapRef().unlockMove();
            fadeDir = true;
        }

        return GameStatus::CONTINUE;
    }

    void Overworld::printCollisionLayer(sf::RenderTarget &frame) const {
        sf::Vector2i pos;
        sf::RectangleShape tile({32, 32});
        std::map<int, sf::Color> collision2Color {
            {1, sf::Color(255, 0, 0, 128)},
            {2, sf::Color(0, 0, 255, 128)},
            {3, sf::Color(255, 255, 0, 128)},
            {4, sf::Color(255, 0, 255, 128)},
            {5, sf::Color(255, 255, 255, 128)},
            {6, sf::Color(255, 50, 0, 128)},
            {7, sf::Color(255, 50, 0, 128)},
            {8, sf::Color(255, 50, 0, 128)}};

        for(pos.x = 0; pos.x < current->getW(); ++pos.x) {
            for(pos.y = 0; pos.y < current->getH(); ++pos.y) {
                int collision = current->getCollision(pos);
                if(collision != 0) {
                    tile.setFillColor(collision2Color[collision]);
                    tile.setPosition(pos.x SQUARES, pos.y SQUARES);
                    frame.draw(tile);
                }
            }
        }
    }

    /**
     * Events can call this method to start a new dialog with the player.
     */
    void Overworld::startDialog(sf::String const &dialog) {
        if(this->dialog) {
            if(!this->dialog->isDialogOver()) {
                Utils::Log::oplog("WARNING: We create a new dialog ... but the "
                                  "last one isn't finished yet!",
                                  true);
            }
            this->dialog = nullptr;
        }

        this->dialog =
            std::make_unique<Ui::Dialog>(dialog, data.getGameDataPtr());
    }

} // namespace OpMon
