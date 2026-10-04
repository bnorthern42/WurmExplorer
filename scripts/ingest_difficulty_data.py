#!/usr/bin/env python3
"""
Wurm Online Master Data Ingestion Script
Fetches official game data from the Master Wurm Data Google Sheet and
generates configs/difficulty_presets.json for the Mechanics Grinder.
"""

import urllib.request
import urllib.parse
import csv
import io
import json
import os
import re

SHEET_ID = "1XPPA0g187rQ-lFb8RJY0HKFHd84F7J3mVPL3Jq-72fg"
BASE_URL = f"https://docs.google.com/spreadsheets/d/{SHEET_ID}/gviz/tq?tqx=out:csv&sheet="

def fetch_csv(sheet_name: str):
    url = BASE_URL + urllib.parse.quote(sheet_name)
    req = urllib.request.Request(url, headers={"User-Agent": "WurmExplorer/1.0"})
    with urllib.request.urlopen(req, timeout=15) as resp:
        content = resp.read().decode("utf-8")
    return list(csv.reader(io.StringIO(content)))

def clean_creature_name(raw: str) -> str:
    name_map = {
        "BrownCow": "Brown Cow",
        "CatWild": "Wild Cat",
        "RatLarge": "Large Rat",
        "LionMountain": "Mountain Lion",
        "BlackWolf": "Black Wolf",
        "BearBlack": "Black Bear",
        "BearBrown": "Brown Bear",
        "CaveBug": "Cave Bug",
        "HellHorse": "Hell Horse",
        "HellHound": "Hell Hound",
        "HellScorpion": "Hell Scorpion",
        "DeathCrawlerMinion": "Death Crawler Minion",
        "GorillaMagranon": "Gorilla Magranon",
        "LavaSpider": "Lava Spider",
        "HyenaLabila": "Hyena Libila",
        "LavaCreature": "Lava Creature",
        "EasterBunny": "Easter Bunny",
        "BlueWhale": "Blue Whale",
        "EagleSpirit": "Eagle Spirit",
        "SharkHuge": "Huge Shark",
        "SpawnUttacha": "Spawn of Uttacha",
        "SonOfNogump": "Son of Nogump",
        "DrakeSpirit": "Drake Spirit",
        "DrakeWhite": "White Drake",
        "DrakeRed": "Red Drake",
        "DrakeGreen": "Green Drake",
        "DrakeBlack": "Black Drake",
        "DrakeBlue": "Blue Drake",
        "DragonBlue": "Blue Dragon",
        "DragonGreen": "Green Dragon",
        "DragonRed": "Red Dragon",
        "DragonBlack": "Black Dragon",
        "DragonWhite": "White Dragon",
        "AvengerOfLight": "Avenger of Light",
        "SeaSerpent": "Sea Serpent",
        "KingCobra": "King Cobra",
        "LadyLake": "Lady of the Lake",
        "EvilSanta": "Evil Santa",
        "SantaClaus": "Santa Claus",
        "IncarnationLibila": "Incarnation of Libila",
        "ManifestationFo": "Manifestation of Fo",
        "ForestGiant": "Forest Giant",
        "GoblinLeader": "Goblin Leader",
        "TrollKing": "Troll King",
        "EpiphanyVynora": "Epiphany of Vynora",
        "SealCub": "Seal Cub"
    }
    if raw in name_map:
        return name_map[raw]
    return re.sub(r'([a-z])([A-Z])', r'\1 \2', raw)

def main():
    presets = {}

    # 1. Mining
    print("Ingesting Mining sheet...")
    mining_rows = fetch_csv("Mining")
    mining_presets = []
    for i, r in enumerate(mining_rows):
        if i == 0 or len(r) < 2:
            continue
        ore, diff_str = r[0].strip(), r[1].strip()
        if not ore or not diff_str:
            continue
        try:
            diff = float(diff_str)
            name = ore.capitalize()
            mining_presets.append({"name": f"{name} Vein", "difficulty": diff})
        except ValueError:
            pass

    presets["MiningPower"] = mining_presets
    presets["MiningQl"] = mining_presets

    # 2. Animal Taming
    print("Ingesting Animal Taming sheet...")
    taming_rows = fetch_csv("Animal Taming")
    taming_presets = []
    age_mods = [
        ("Young", 0.9),
        ("Adolescent", 1.0),
        ("Mature", 1.1),
        ("Aged", 1.2),
        ("Old", 1.3),
        ("Venerable", 1.4)
    ]

    for i, r in enumerate(taming_rows):
        if i == 0 or len(r) < 2:
            continue
        c_raw, diff_str = r[0].strip(), r[1].strip()
        if not c_raw or not diff_str:
            continue
        try:
            base_diff = float(diff_str)
            disp_name = clean_creature_name(c_raw)
            # Base preset
            taming_presets.append({"name": disp_name, "difficulty": base_diff})
            if c_raw == "BrownCow":
                taming_presets.append({"name": "Cow", "difficulty": base_diff})

            # Age-specific presets
            for age_name, mod in age_mods:
                taming_presets.append({
                    "name": f"{disp_name} ({age_name})",
                    "difficulty": round(base_diff * mod, 2)
                })
                if c_raw == "BrownCow":
                    taming_presets.append({
                        "name": f"Cow ({age_name})",
                        "difficulty": round(base_diff * mod, 2)
                    })
        except ValueError:
            pass

    presets["Taming"] = taming_presets

    # 3. Crops (Farming)
    try:
        print("Ingesting Crops sheet...")
        crop_rows = fetch_csv("Crops")
        farming_presets = []
        for i, r in enumerate(crop_rows):
            if i == 0 or len(r) < 2:
                continue
            crop, diff_str = r[0].strip(), r[1].strip()
            if not crop or not diff_str:
                continue
            try:
                diff = float(diff_str)
                farming_presets.append({"name": crop, "difficulty": diff})
            except ValueError:
                pass
        if farming_presets:
            presets["Farming"] = farming_presets
    except Exception as e:
        print("Warning: Crops ingestion failed:", e)

    # 4. Digging
    try:
        print("Ingesting Digging Diff. sheet...")
        dig_rows = fetch_csv("Digging Diff.")
        dig_presets = []
        for i, r in enumerate(dig_rows):
            if len(r) < 2:
                continue
            tile, diff_str = r[0].strip(), r[1].strip()
            if not tile or not diff_str:
                continue
            try:
                diff = float(diff_str)
                dig_presets.append({"name": tile, "difficulty": diff})
            except ValueError:
                pass
        if dig_presets:
            presets["Digging"] = dig_presets
    except Exception as e:
        print("Warning: Digging ingestion failed:", e)

    # 5. Wood
    try:
        print("Ingesting Wood sheet...")
        wood_rows = fetch_csv("Wood")
        wood_presets = []
        for i, r in enumerate(wood_rows):
            if i == 0 or len(r) < 2:
                continue
            tree, diff_str = r[0].strip(), r[1].strip()
            if not tree or not diff_str:
                continue
            try:
                diff = float(diff_str)
                wood_presets.append({"name": f"{tree} Tree", "difficulty": diff})
            except ValueError:
                pass
        if wood_presets:
            presets["WoodcuttingQl"] = wood_presets
    except Exception as e:
        print("Warning: Wood ingestion failed:", e)

    # 6. Complementary Action Modes for complete coverage
    presets["GenericCheck"] = [
        {"name": "Very Easy", "difficulty": 5.0},
        {"name": "Easy Task", "difficulty": 10.0},
        {"name": "Standard Task", "difficulty": 20.0},
        {"name": "Challenging Task", "difficulty": 40.0},
        {"name": "Hard Task", "difficulty": 60.0},
        {"name": "Extreme Task", "difficulty": 80.0}
    ]
    presets["Meditation"] = [
        {"name": "Level 1 Path", "difficulty": 10.0},
        {"name": "Level 3 Path", "difficulty": 20.0},
        {"name": "Level 5 Path", "difficulty": 30.0},
        {"name": "Level 7 Path", "difficulty": 45.0},
        {"name": "Level 9 Path", "difficulty": 60.0},
        {"name": "Level 11 Path", "difficulty": 75.0}
    ]
    presets["Creation"] = [
        {"name": "Shaft / Handle", "difficulty": 5.0},
        {"name": "Simple Tool", "difficulty": 10.0},
        {"name": "Carving Knife", "difficulty": 20.0},
        {"name": "Large Cart", "difficulty": 30.0},
        {"name": "Small Wooden Shacks", "difficulty": 40.0},
        {"name": "Rowing Boat", "difficulty": 50.0},
        {"name": "Corbita", "difficulty": 70.0}
    ]
    presets["Imping"] = [
        {"name": "Low QL < 30", "difficulty": 10.0},
        {"name": "Mid QL 30-60", "difficulty": 25.0},
        {"name": "High QL 60-80", "difficulty": 45.0},
        {"name": "Master QL 80-90", "difficulty": 65.0},
        {"name": "Supreme QL 90+", "difficulty": 85.0}
    ]
    presets["SmithingSteps"] = [
        {"name": "Pelt / Needle", "difficulty": 5.0},
        {"name": "Carving Knife Blade", "difficulty": 15.0},
        {"name": "Horseshoe", "difficulty": 20.0},
        {"name": "Large Anvil", "difficulty": 30.0},
        {"name": "Longsword", "difficulty": 40.0},
        {"name": "Plate Armor", "difficulty": 60.0}
    ]
    presets["Fileting"] = [
        {"name": "Roach / Perch", "difficulty": 10.0},
        {"name": "Trout / Bass", "difficulty": 20.0},
        {"name": "Pike / Catfish", "difficulty": 30.0},
        {"name": "Shark / Marlin", "difficulty": 50.0}
    ]
    presets["Forestry"] = [
        {"name": "Sprout Picking", "difficulty": 10.0},
        {"name": "Pruning Young Tree", "difficulty": 20.0},
        {"name": "Pruning Mature Tree", "difficulty": 30.0},
        {"name": "Pruning Old Tree", "difficulty": 45.0}
    ]
    presets["Shearing"] = [
        {"name": "Lamb (< 3 yrs)", "difficulty": 10.0},
        {"name": "Young Sheep (< 8 yrs)", "difficulty": 15.0},
        {"name": "Adolescent Sheep (< 12 yrs)", "difficulty": 20.0},
        {"name": "Adult Sheep (< 30 yrs)", "difficulty": 25.0},
        {"name": "Mature Sheep (< 40 yrs)", "difficulty": 30.0},
        {"name": "Old / Overgrown Fleece", "difficulty": 35.0}
    ]

    # Write configs/difficulty_presets.json
    out_dir = os.path.join(os.path.dirname(__file__), "..", "configs")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "difficulty_presets.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(presets, f, indent=2)

    print(f"Successfully generated {out_path} with {len(presets)} categories.")
    for k, v in presets.items():
        print(f"  - {k}: {len(v)} entries")

if __name__ == "__main__":
    main()
