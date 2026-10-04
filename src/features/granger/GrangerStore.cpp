#include "GrangerStore.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace granger {

GrangerStore::GrangerStore(const std::string& filepath)
    : m_filepath(filepath) {
    load();
}

void GrangerStore::load() {
    m_players.clear();
    m_animals.clear();

    std::ifstream file(m_filepath);
    if (!file.is_open()) return;

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("players") && j["players"].is_array()) {
            for (const auto& item : j["players"]) {
                m_players.push_back(item.get<Player>());
            }
        }

        if (j.contains("animals") && j["animals"].is_array()) {
            for (const auto& item : j["animals"]) {
                m_animals.push_back(item.get<Animal>());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse Granger JSON from " << m_filepath << ": " << e.what() << std::endl;
    }
}

void GrangerStore::save() const {
    nlohmann::json j;
    j["players"] = m_players;
    j["animals"] = m_animals;

    std::ofstream file(m_filepath);
    if (file.is_open()) {
        file << j.dump(2);
    }
}

const std::vector<Player>& GrangerStore::getPlayers() const {
    return m_players;
}

const std::vector<Animal>& GrangerStore::getAnimals() const {
    return m_animals;
}

const Player* GrangerStore::getPlayer(int id) const {
    for (const auto& p : m_players) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

const Animal* GrangerStore::getAnimal(int id) const {
    for (const auto& a : m_animals) {
        if (a.id == id) return &a;
    }
    return nullptr;
}

int GrangerStore::getNextPlayerId() const {
    int maxId = 0;
    for (const auto& p : m_players) {
        if (p.id > maxId) maxId = p.id;
    }
    return maxId + 1;
}

int GrangerStore::getNextAnimalId() const {
    int maxId = 0;
    for (const auto& a : m_animals) {
        if (a.id > maxId) maxId = a.id;
    }
    return maxId + 1;
}

void GrangerStore::addPlayer(const Player& player) {
    Player p = player;
    if (p.id <= 0) {
        p.id = getNextPlayerId();
    }
    m_players.push_back(p);
}

bool GrangerStore::updatePlayer(const Player& player) {
    for (auto& p : m_players) {
        if (p.id == player.id) {
            p = player;
            return true;
        }
    }
    return false;
}

bool GrangerStore::deletePlayer(int id) {
    auto it = std::remove_if(m_players.begin(), m_players.end(), [id](const Player& p) {
        return p.id == id;
    });
    if (it != m_players.end()) {
        m_players.erase(it, m_players.end());
        // Reset cared_by for animals cared for by this player
        for (auto& a : m_animals) {
            if (a.cared_by == id) {
                a.cared_by = 0;
            }
        }
        return true;
    }
    return false;
}

void GrangerStore::addAnimal(const Animal& animal) {
    Animal a = animal;
    if (a.id <= 0) {
        a.id = getNextAnimalId();
    }
    m_animals.push_back(a);
}

bool GrangerStore::updateAnimal(const Animal& animal) {
    for (auto& a : m_animals) {
        if (a.id == animal.id) {
            a = animal;
            return true;
        }
    }
    return false;
}

bool GrangerStore::deleteAnimal(int id) {
    auto it = std::remove_if(m_animals.begin(), m_animals.end(), [id](const Animal& a) {
        return a.id == id;
    });
    if (it != m_animals.end()) {
        m_animals.erase(it, m_animals.end());
        return true;
    }
    return false;
}

int GrangerStore::getCaredAnimalCount(int playerId) const {
    int count = 0;
    for (const auto& a : m_animals) {
        if (a.cared_by == playerId) {
            count++;
        }
    }
    return count;
}

int GrangerStore::getRemainingSlots(int playerId) const {
    const Player* p = getPlayer(playerId);
    if (!p) return 0;
    return p->maxSlots() - getCaredAnimalCount(playerId);
}

} // namespace granger
