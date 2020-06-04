/*!
 * \file Overworld.hpp
 * \authors Cyrielle
 * \authors BAKFR
 * \copyright GNU GPL v3.0
 */
#ifndef OVERWORLD_HPP
#define OVERWORLD_HPP

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "OverworldData.hpp"
#include "src/opmon/core/Dialog.hpp"
#include "src/opmon/core/Elements.hpp"
#include "src/opmon/core/GameStatus.hpp"
#include "src/opmon/screens/overworld/events/BattleEvent.hpp"
#include "src/opmon/model/Enums.hpp"
#include "src/opmon/screens/overworld/Map.hpp"
#include "src/opmon/screens/overworld/events/PlayerEvent.hpp"
#include "src/utils/log.hpp"

namespace OpMon {
    /*!
     * \brief The overworld.
     * \details This class prints and manages the camera, the map, the events, the animated elements on the map, and the player's animation.
     * \todo Unify the player animation system and the npc animation system.
     */
    class Overworld : public sf::Drawable {
      public:
        Overworld(const std::string &mapId, OverworldData &data);

        PlayerEvent &getCharacter() { return character; }

        /*!
         * \brief Returns the map the player is in currently.
         * \todo Rename to getCurrentMap
         */
        Map *getCurrent() { return current; }

        /*!
         * \brief Updates the overworld.
         */
        GameStatus update();

        void draw(sf::RenderTarget &frame, sf::RenderStates states) const;

        /*!
         * \brief Teleports the player with a fading animation.    class OverworldData;

        class Map;
        namespace Events {
            class TrainerEvent;
        } // namespace Events
    } // namespace OpMon
         *
         * \param toTp The ID of the map in which teleport the player.
         * \param pos Where to teleport the player in the map.
         * \param tpDir The direction the player will face after the teleportation.
         * Side::NO_MOVE will keep the direction the player was facing before.
         */
        void tp(std::string toTp, sf::Vector2i pos, Side tpDir = Side::NO_MOVE);

        /*!
         * \brief Directly teleports the player without any animation.
         * \param toTp The ID of the map in which teleport the player.
         * \param pos Where to teleport the player in the map.
         * \param tpDir The direction the player will face after the teleportation.
         * Side::NO_MOVE will keep the direction the player was facing before.
         */
        void tpNoAnim(std::string toTp, sf::Vector2i pos,
                      Side tpDir = Side::NO_MOVE);

        /*!
         * \brief Returns a layer of the map the player is currently in.
         * \param number The number of the layer.
         * \returns The layer of the map. Returns nullptr if the number given is not 1, 2 or 3.
         */
        MapLayer *getMapLayer(int number) {
            switch(number) {
                case 1:
                    return layer1.get();
                case 2:
                    return layer2.get();
                case 3:
                    return layer3.get();
                default:
                    return nullptr;
            }
        }

        Dialog *getDialog() { return dialog.get(); }

        void moveCamera(Side dir);
        bool isCameraLocked();
        void setCameraLock(bool locked);

        /*!
         * \brief Decides which layer is printed.
         */
        bool printlayer[3] = {true, true, true};
        bool printCollisions = false;

        /*!
         * \brief Locks the player's movements for a short time after teleporting.
         */
        bool justTp = false;

        virtual ~Overworld() = default;

        virtual void play();
        virtual void pause();

        /*!
         * \brief Plays the music given in parameter.
         * \deprectated Directly use Jukebox::play.
         */
        void setMusic(std::string const &path);

        BattleEvent *getBattleDeclared() { return trainerToBattle; }

        /*!
         * \brief Makes the overworld start a Battle.
         * \details The controller will then start a Battle with the given trainer.
         * \param trainer The trainer to fight.
         */
        void declareBattle(BattleEvent *trainer) {
            if(trainerToBattle == nullptr) {
                trainerToBattle = trainer;
            } else {
                throw std::runtime_error(
                    "Trying to set up a battle, but an another one is already "
                    "set up. Recheck the code.");
            }
        }

        bool isInBattle() const { return !(trainerToBattle == nullptr); }

        bool isDialogOver() const {
            if(dialog == nullptr) {
                return true;
            } else {
                return dialog->isDialogOver();
            }
        }

        /*!
         * \brief Ends the current Battle.
         */
        void endBattle() {
            if(trainerToBattle == nullptr) {
                Utils::Log::warn("Ending a battle, but there is no battle.");
            }
            trainerToBattle = nullptr;
        }

        /*!
         * \brief Events can call this method to start a new dialog with the player.
         */
        void startDialog(sf::String const &dialog);

        /*!
         * \brief Animates the player.
         */
        void startPlayerAnimation() { initPlayerAnimation = true; }

        OverworldData &getData() { return data; }

        /*!
         * \brief If `true`, the game is in debug mode.
         */
        bool debugMode = false;

      private:
        /*!
         * \brief Prints the animated elements on the screen.
         */
        void printElements(sf::RenderTarget &frame) const;

        /*!
         * \brief Updates the animated elements.
         */
        void updateElements();

        /*!
         * \brief Shows the collision layer.
         * \brief The collision layer consists in semi-transparent squares on each tile. You can see it by pressing C while in debug mode ('=' key).
         */
        void printCollisionLayer(sf::RenderTarget &frame) const;

        /*!
         * \brief Moves the camera to follow the player's position.
         * \todo Change camera to make it always centered on the player, except on the edges of the map
         */
        void updateCamera();

        /*!
         * \brief Centers the camera on the player.
         */
        void resetCamera();

        BattleEvent *trainerToBattle = nullptr;

        sf::Text debugText;
        sf::View camera;
        PlayerEvent &character;
        /*!
         * \brief The map the player is currently in.
         */
        Map *current = nullptr;

        std::unique_ptr<MapLayer> layer1;
        std::unique_ptr<MapLayer> layer2;
        std::unique_ptr<MapLayer> layer3;
        std::unique_ptr<Dialog> dialog;
        /*!
         * \brief Indicates the frame of the walking animation that must be used.
         * \details Since the animation has two frames, this variable alternates between `true` and `false` every half of the animation.
         */
        bool anims = false;

        /*!
         * \brief A rectangle with the same size as the screen.
         *
         * Used for the fading animation.
         */
        sf::RectangleShape screenCache;

        /*!
         * \brief The number of frames a fading lasts
         *
         * This is the numer of frames it takes for the fading animation
         * to go from transparent to black or black to transparent.
         */
        const float fadeFrames = 15;

        /*!
         * \brief Counts the frame for the fading animation
         *
         * The counter increases when the animation starts,
         * then stops at \ref fadeFrames and decreases back to 0.
         */
        float fadeCountdown = 0;

        /*!
         * \brief Sets the direction of the fading animation.
         *
         * `true` for transparent to black, `false` for the opposite.
         */
        bool fadeDir = true;

        /*!
         * \brief The map where the player has to TP during the fade animation.
         */
        std::string toTp;

        /*!
         * \brief The position where the player has to TP during the fade animation.
         */
        sf::Vector2i tpPos;

        /*!
         * \brief The direction the player will face after the teleportation.
         */
        Side tpDir;

        int fpsCounter = 0;
        sf::Text fpsPrint;
        sf::Text coordPrint;
        int oldTicksFps = 0;

        int animsCounter = 0;
        bool initPlayerAnimation = false;
        int startPlayerAnimationTime = 0;

        bool cameraLock = false;

        std::map<std::string, sf::Sprite> elementsSprites;

        OverworldData &data;
    };

} // namespace OpMon

#endif // OVERWORLD_HPP
