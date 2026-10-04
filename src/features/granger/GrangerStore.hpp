#pragma once

#include "GrangerModels.hpp"
#include <string>
#include <vector>

namespace granger {

class GrangerStore {
public:
    explicit GrangerStore(const std::string& filepath);

    void load();
    void save() const;

    const std::vector<Player>& getPlayers() const;
    const std::vector<Animal>& getAnimals() const;

    const Player* getPlayer(int id) const;
    const Animal* getAnimal(int id) const;

    void addPlayer(const Player& player);
    bool updatePlayer(const Player& player);
    bool deletePlayer(int id);

    void addAnimal(const Animal& animal);
    bool updateAnimal(const Animal& animal);
    bool deleteAnimal(int id);

    int getCaredAnimalCount(int playerId) const;
    int getRemainingSlots(int playerId) const;

    int getNextPlayerId() const;
    int getNextAnimalId() const;

private:
    std::string m_filepath;
    std::vector<Player> m_players;
    std::vector<Animal> m_animals;
};

} // namespace granger
