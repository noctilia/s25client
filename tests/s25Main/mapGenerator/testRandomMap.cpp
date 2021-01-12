// Copyright (c) 2017 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "lua/GameDataLoader.h"
#include "mapGenerator/RandomMap.h"
#include "gameData/MaxPlayers.h"
#include "libsiedler2/ArchivItem_Map.h"
#include "libsiedler2/ArchivItem_Map_Header.h"
#include "libsiedler2/libsiedler2.h"
#include "rttr/test/TmpFolder.hpp"
#include "rttr/test/random.hpp"
#include <boost/test/unit_test.hpp>

using namespace rttr::mapGenerator;

BOOST_AUTO_TEST_SUITE(RandomMapTests)

static void ValidateMap(const Map& map, const MapExtent& size, unsigned players)
{
    BOOST_REQUIRE(map.players == players);
    BOOST_REQUIRE(map.size == size);

    for(unsigned index = 0; index < players; index++)
    {
        BOOST_REQUIRE(map.hqPositions[index].isValid());
    }
}

static MapExtent getRandomMapSize(const unsigned minSize = 32)
{
    auto size = rttr::test::randomPoint<MapExtent>(minSize, 80);
    // Need even size
    size.x &= ~1u;
    size.y &= ~1u;
    return size;
}

BOOST_AUTO_TEST_CASE(GenerateRandomMap_returns_valid_land_map)
{
    RandomUtility rnd(0);
    WorldDescription worldDesc;
    loadGameData(worldDesc);
    MapSettings settings;

    settings.size = getRandomMapSize();
    settings.numPlayers = rttr::test::randomValue(2u, MAX_PLAYERS);
    settings.style = MapStyle::Land;

    Map map = GenerateRandomMap(rnd, worldDesc, settings);
    ValidateMap(map, settings.size, settings.numPlayers);
}

BOOST_AUTO_TEST_CASE(GenerateRandomMap_returns_valid_water_map)
{
    RandomUtility rnd(0);
    WorldDescription worldDesc;
    loadGameData(worldDesc);
    MapSettings settings;

    settings.size = getRandomMapSize(50); // Need enough space for player islands
    settings.numPlayers = rttr::test::randomValue(2u, MAX_PLAYERS);
    settings.style = MapStyle::Water;

    Map map = GenerateRandomMap(rnd, worldDesc, settings);
    ValidateMap(map, settings.size, settings.numPlayers);
}

BOOST_AUTO_TEST_CASE(GenerateRandomMap_returns_valid_mixed_map)
{
    RandomUtility rnd(0);
    WorldDescription worldDesc;
    loadGameData(worldDesc);
    MapSettings settings;

    settings.size = getRandomMapSize();
    settings.numPlayers = rttr::test::randomValue(2u, MAX_PLAYERS);
    settings.style = MapStyle::Mixed;

    Map map = GenerateRandomMap(rnd, worldDesc, settings);
    ValidateMap(map, settings.size, settings.numPlayers);
}

BOOST_AUTO_TEST_CASE(GenerateRandomMap_returns_valid_map_with_max_players)
{
    RandomUtility rnd(0);
    WorldDescription worldDesc;
    loadGameData(worldDesc);
    MapSettings settings;

    settings.size = getRandomMapSize();
    settings.numPlayers = MAX_PLAYERS;
    settings.style = MapStyle::Land;

    Map map = GenerateRandomMap(rnd, worldDesc, settings);
    ValidateMap(map, settings.size, settings.numPlayers);
}

BOOST_AUTO_TEST_CASE(GenerateRandomMap_returns_valid_mapfile_with_max_players)
{
    rttr::test::TmpFolder tmpFolder;
    const auto mapPath = tmpFolder.get() / "map.wld";

    MapSettings settings;
    settings.size = MapExtent(64, 64);
    settings.numPlayers = MAX_PLAYERS;
    settings.style = MapStyle::Land;

    CreateRandomMap(mapPath, settings);
    libsiedler2::Archiv archive;
    BOOST_TEST_REQUIRE(libsiedler2::Load(mapPath, archive) == 0);
    const auto* map = dynamic_cast<const libsiedler2::ArchivItem_Map*>(archive[0]);
    BOOST_TEST_REQUIRE(map);
    const auto* mapHeader = dynamic_cast<const libsiedler2::ArchivItem_Map_Header*>(map->get(0));
    BOOST_TEST_REQUIRE(mapHeader);
    BOOST_TEST(mapHeader->getWidth() == settings.size.x);
    BOOST_TEST(mapHeader->getHeight() == settings.size.y);
    BOOST_TEST(mapHeader->getNumPlayers() == settings.numPlayers);
    // Must have unique HQ positions
    std::vector<MapPoint> hqs;
    for(unsigned i = 0; i < std::min(unsigned(mapHeader->maxPlayers), settings.numPlayers); i++)
    {
        Point<uint16_t> hqPos;
        mapHeader->getPlayerHQ(i, hqPos.x, hqPos.y);
        BOOST_TEST(!helpers::contains(hqs, MapPoint(hqPos)));
        hqs.emplace_back(hqPos);
    }
}

BOOST_AUTO_TEST_SUITE_END()
