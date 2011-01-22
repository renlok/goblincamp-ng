/* Copyright 2011 Ilkka Halila
This file is part of Goblin Camp.

Goblin Camp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Goblin Camp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with Goblin Camp. If not, see <http://www.gnu.org/licenses/>.*/
#include "stdafx.hpp"

#include "tileRenderer/TileSet.hpp"
#include <boost/numeric/conversion/cast.hpp> 

#include "FarmPlot.hpp"
#include "Stockpile.hpp"
#include "Door.hpp"
#include "SpawningPool.hpp"

TileSet::TileSet(std::string tileSetName, int tileW, int tileH) :
	name(tileSetName),
	tileWidth(tileW),
	tileHeight(tileH),
	author(""),
	description(""),
	waterTiles(),
	minorFilth(),
	majorFilth(),
	nonTerritoryOverlay(),
	territoryOverlay(),
	markedOverlay(),
	marker(),
	blood(),
	defaultUnderConstructionSprite(),
	defaultNPCSpriteSet(),
	npcSpriteSets(),
	npcSpriteLookup(),
	defaultNatureObjectSpriteSet(),
	natureObjectSpriteSets(),
	natureObjectSpriteLookup(),
	itemSpriteSets(),
	itemSpriteLookup(),
	defaultItemSpriteSet(),
	constructionSpriteSets(),
	constructionSpriteLookup(),
	defaultConstructionSpriteSet(){
		for (int i = 0; i < terrainTiles.size(); ++i) {
			terrainTiles[i] = Sprite();
		}
		for (int i = 0; i < placeableCursors.size(); ++i) {
			placeableCursors[i] = Sprite();
			nonplaceableCursors[i] = Sprite();
		}
		for (int i = 0; i < defaultStatusEffects.size(); ++i) {
			defaultStatusEffects[i] = Sprite();
		}
}

TileSet::~TileSet() {}

int TileSet::TileWidth() const {
	return tileWidth;
}

int TileSet::TileHeight() const {
	return tileHeight;
}

std::string TileSet::GetName() const {
	return name;
}

std::string TileSet::GetAuthor() const {
	return author;
}

std::string TileSet::GetDescription() const {
	return description;
}

void TileSet::DrawMarkedOverlay(SDL_Surface *dst, SDL_Rect* dstRect) const {
	markedOverlay.Draw(dst, dstRect);
}

void TileSet::DrawMarker(SDL_Surface *dst, SDL_Rect* dstRect) const {
	marker.Draw(dst, dstRect);
}

void TileSet::DrawTerrain(TileType type, SDL_Surface *dst, SDL_Rect* dstRect) const {
	terrainTiles.at(type).Draw(dst, dstRect);
}

void TileSet::DrawBlood(SDL_Surface *dst, SDL_Rect* dstRect) const {
	blood.Draw(dst, dstRect);
}

void TileSet::DrawWater(int index, SDL_Surface *dst, SDL_Rect* dstRect) const {
	int i = std::min(index, boost::numeric_cast<int>(waterTiles.size()) - 1);
	if (i > -1) {
		waterTiles.at(i).Draw(dst, dstRect);
	}
}

void TileSet::DrawFilthMinor(SDL_Surface *dst, SDL_Rect * dstRect) const {
	minorFilth.Draw(dst, dstRect);
}

void TileSet::DrawFilthMajor(SDL_Surface *dst, SDL_Rect * dstRect) const {
	majorFilth.Draw(dst, dstRect);
}

void TileSet::DrawTerritoryOverlay(bool owned, SDL_Surface *dst, SDL_Rect * dstRect) const {
	if (owned) {
		territoryOverlay.Draw(dst, dstRect);
	} else {
		nonTerritoryOverlay.Draw(dst, dstRect);
	}
}

void TileSet::DrawNPC(boost::shared_ptr<NPC> npc, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = npc->GetGraphicsHint();
	if (hint == -1 || hint >= npcSpriteSets.size()) {
		defaultNPCSpriteSet.tile.Draw(dst, dstRect);
	} else {
		npcSpriteSets[hint].tile.Draw(dst, dstRect);
	}

	if ((TCODSystem::getElapsedMilli() % 1000 < 700)) {
		if (npc->HasEffect(CARRYING)) {
			boost::shared_ptr<Item> carriedItem = npc->Carrying().lock();
			if (carriedItem.get() != 0) {
				DrawItem(carriedItem, dst, dstRect);
			}
		}
	}

	int numEffects = npc->StatusEffects()->size();
	if (npc->HasEffect(CARRYING)) numEffects--; // Already handled
	if (npc->HasEffect(WORKING)) numEffects--; // Don't draw
	if (npc->HasEffect(SWIM)) {
		numEffects--;
		defaultStatusEffects.at(SWIM).Draw(dst, dstRect);
	}
	if (npc->HasEffect(FLYING)) numEffects--; // Dont Draw
	if (numEffects > 0) {
		int effect = (TCODSystem::getElapsedMilli() % (250 * numEffects)) / 250;
		int atEffect = 0;
		for (std::list<StatusEffect>::iterator effecti(npc->StatusEffects()->begin()); effecti != npc->StatusEffects()->end(); ++effecti) {
			if (effect == atEffect++) {
				defaultStatusEffects.at(effecti->type).Draw(dst, dstRect);
				break;
			}
		}
	}
}

void TileSet::DrawNatureObject(boost::shared_ptr<NatureObject> plant, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = plant->GetGraphicsHint();
	if (hint == -1 || hint >= natureObjectSpriteSets.size()) {
		defaultNatureObjectSpriteSet.tile.Draw(dst, dstRect);
	} else {
		natureObjectSpriteSets[hint].tile.Draw(dst, dstRect);
	}
}

void TileSet::DrawItem(boost::shared_ptr<Item> item, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = item->GetGraphicsHint();
	if (hint == -1 || hint >= itemSpriteSets.size()) {
		defaultItemSpriteSet.tile.Draw(dst, dstRect);
	} else {
		itemSpriteSets[hint].tile.Draw(dst, dstRect);
	}
}


class TileSet::DrawConstructionVisitor : public ConstructionVisitor  {
private:
	TileSet * tileSet;
	SDL_Surface * dst;
	SDL_Rect * dstRect;
	const Coordinate& coordinate;
public:
	DrawConstructionVisitor(TileSet * t, SDL_Surface * destination, SDL_Rect *destinationRect, const Coordinate& pos)
		: tileSet(t),
		  dst(destination),
		  dstRect(destinationRect),
		  coordinate(pos)
	{}


	void Visit(FarmPlot * farmplot) { 
		tileSet->DrawBaseConstruction(farmplot, coordinate, dst, dstRect);
		tileSet->DrawStockpileContents(farmplot, coordinate, dst, dstRect);
	}

	void Visit(Stockpile * stockpile) {
		tileSet->DrawBaseConstruction(stockpile, coordinate, dst, dstRect);
		tileSet->DrawStockpileContents(stockpile, coordinate, dst, dstRect);
	}

	void Visit(Construction * construction) {
		Coordinate internal_pos = coordinate - construction->Position();
		int pos = internal_pos.X() + internal_pos.Y() * Construction::Blueprint(construction->Type()).X();
		int maxPos = Construction::Blueprint(construction->Type()).X() * Construction::Blueprint(construction->Type()).Y();
		if ((construction->GetMaxCondition() + construction->Condition()) * maxPos > (pos + 1) * construction->GetMaxCondition()) {
			tileSet->DrawBaseConstruction(construction, coordinate, dst, dstRect);
		} else {
			tileSet->DrawUnderConstruction(construction, coordinate, dst, dstRect);
		}
	}

	void Visit(SpawningPool * spawningPool) {
		if (spawningPool->Condition() < 0) {
			tileSet->DrawUnderConstruction(spawningPool, coordinate, dst, dstRect);
		} else { 
			tileSet->DrawBaseConstruction(spawningPool, coordinate, dst, dstRect);
		}
	}

	void Visit(Door * door) {
		if (door->Open()) {
			tileSet->DrawOpenDoor(door, coordinate, dst, dstRect);
		} else {
			tileSet->DrawBaseConstruction(door, coordinate, dst, dstRect);
		}
	}

};

void TileSet::DrawOpenDoor(Door * door, const Coordinate& worldPos, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = door->GetGraphicsHint();
	if (hint == -1 || hint >= constructionSpriteSets.size()) {
		defaultConstructionSpriteSet.DrawOpen(worldPos - door->Position(), dst, dstRect);
	} else {
		constructionSpriteSets[hint].DrawOpen(worldPos - door->Position(), dst, dstRect);
	}
}

void TileSet::DrawConstruction(boost::shared_ptr<Construction> construction, const Coordinate& worldPos, SDL_Surface *dst, SDL_Rect * dstRect) {
	DrawConstructionVisitor visitor(this, dst, dstRect, worldPos);
	construction->AcceptVisitor(visitor);
}

void TileSet::DrawBaseConstruction(Construction * construction, const Coordinate& worldPos, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = construction->GetGraphicsHint();
	if (hint == -1 || hint >= constructionSpriteSets.size()) {
		defaultConstructionSpriteSet.Draw(worldPos - construction->Position(), dst, dstRect);
	} else {
		constructionSpriteSets[hint].Draw(worldPos - construction->Position(), dst, dstRect);
	}
}

void TileSet::DrawUnderConstruction(Construction * construction, const Coordinate& worldPos, SDL_Surface *dst, SDL_Rect * dstRect) const {
	int hint = construction->GetGraphicsHint();
	const ConstructionSpriteSet& set((hint == -1 || hint >= constructionSpriteSets.size()) ? defaultConstructionSpriteSet : constructionSpriteSets[hint]);
	if (set.HasUnderConstructionSprites()) {
		set.DrawUnderConstruction(worldPos - construction->Position(), dst, dstRect);
	} else {
		defaultUnderConstructionSprite.Draw(dst, dstRect);
	}
}

void TileSet::DrawStockpileContents(Stockpile * stockpile, const Coordinate& worldPos, SDL_Surface *dst, SDL_Rect * dstRect) const {
	boost::shared_ptr<Container> storage = stockpile->Storage(worldPos).lock();
	if (storage.get() != 0 && !storage->empty()) {
		boost::shared_ptr<Item> item = storage->GetFirstItem().lock();
		if (item.get() != 0) {
			DrawItem(item, dst, dstRect);
		}
	}
}

void TileSet::DrawCursor(CursorType type, int cursorHint, bool placeable, SDL_Surface *dst, SDL_Rect * dstRect) const {
	if (type == Cursor_Item_Mode)
	{
		if (cursorHint == -1 || cursorHint >= itemSpriteSets.size()) {
			defaultItemSpriteSet.tile.Draw(dst, dstRect);
		} else {
			itemSpriteSets[cursorHint].tile.Draw(dst, dstRect);
		}
	} else if (type == Cursor_NPC_Mode) {
		if (cursorHint == -1 || cursorHint >= npcSpriteSets.size()) {
			defaultNPCSpriteSet.tile.Draw(dst, dstRect);
		} else {
			npcSpriteSets[cursorHint].tile.Draw(dst, dstRect);
		}
	} else {
		if (placeable) {
			placeableCursors.at(type).Draw(dst, dstRect);
		} else {
			nonplaceableCursors.at(type).Draw(dst, dstRect);
		}
	}
}

int TileSet::GetGraphicsHintFor(const NPCPreset& npcPreset) const {
	LookupMap::const_iterator set;

	set = npcSpriteLookup.find(npcPreset.typeName);
	if (set != npcSpriteLookup.end()) {
		return set->second;
	}

	set = npcSpriteLookup.find(npcPreset.fallbackGraphicsSet);
	if (set != npcSpriteLookup.end()) {
		return set->second;
	}	

	return -1;
}

int TileSet::GetGraphicsHintFor(const NatureObjectPreset& natureObjectPreset) const {
	LookupMap::const_iterator set;
	
	set = natureObjectSpriteLookup.find(natureObjectPreset.name);
	if (set != natureObjectSpriteLookup.end()) {
		return set->second;
	}

	set = natureObjectSpriteLookup.find(natureObjectPreset.fallbackGraphicsSet);
	if (set != natureObjectSpriteLookup.end()) {
		return set->second;
	}

	return -1;
}

int TileSet::GetGraphicsHintFor(const ItemPreset& itemPreset) const {
	LookupMap::const_iterator set;
	
	set = itemSpriteLookup.find(itemPreset.name);
	if (set != itemSpriteLookup.end()) {
		return set->second;
	}

	set = itemSpriteLookup.find(itemPreset.fallbackGraphicsSet);
	if (set != itemSpriteLookup.end()) {
		return set->second;
	}

	for (std::set<ItemCategory>::const_iterator cati = itemPreset.specificCategories.begin(); cati != itemPreset.specificCategories.end(); ++cati) {
		ItemCategory catId = *cati;
		while (catId != -1) {
			ItemCat& cat = Item::Categories.at(catId);
			set = itemSpriteLookup.find(cat.GetName());
			if (set != itemSpriteLookup.end())
			{
				return set->second;
			}
			catId = cat.parent;
		}
	}

	return -1;
}

int TileSet::GetGraphicsHintFor(const ConstructionPreset& constructPreset) const {
	LookupMap::const_iterator set;
	
	set = constructionSpriteLookup.find(constructPreset.name);
	if (set != itemSpriteLookup.end()) {
		return set->second;
	}

	set = itemSpriteLookup.find(constructPreset.fallbackGraphicsSet);
	if (set != itemSpriteLookup.end()) {
		return set->second;
	}

	return -1;
}

void TileSet::SetDescription(std::string desc) {
	description = desc;
}

void TileSet::SetAuthor(std::string auth) {
	author = auth;
}

void TileSet::SetTerrain(TileType type, const Sprite& sprite) {
	if (type < 0 || type >= TILE_TYPE_COUNT) 
		return;

	if (type == TILENONE) {
		for (int i = 0; i < terrainTiles.size(); ++i) {
			if (!terrainTiles[i].Exists()) terrainTiles[i] = sprite;
		}
	} else {
		terrainTiles[type] = sprite;
	}
}

void TileSet::AddWater(const Sprite& sprite) {
	waterTiles.push_back(sprite);
}

void TileSet::SetFilthMinor(const Sprite& sprite) {
	minorFilth = sprite;
}

void TileSet::SetFilthMajor(const Sprite& sprite) {
	majorFilth = sprite;
}

void TileSet::SetMarker(const Sprite& sprite) {
	marker = sprite;
}

void TileSet::SetBlood(const Sprite& sprite) {
	blood = sprite;
}

void TileSet::SetNonTerritoryOverlay(const Sprite& sprite) {
	nonTerritoryOverlay = sprite;
}

void TileSet::SetTerritoryOverlay(const Sprite& sprite) {
	territoryOverlay = sprite;
}

void TileSet::SetMarkedOverlay(const Sprite& sprite) {
	markedOverlay = sprite;
}

void TileSet::SetCursorSprites(CursorType type, const Sprite& sprite) {
	placeableCursors[type] = sprite;
	nonplaceableCursors[type] = sprite;
	if (type == Cursor_None) {
		for (CursorTypeSpriteArray::size_type i = 0; i < placeableCursors.size(); ++i) {
			if (!placeableCursors[i].Exists()) {
				placeableCursors[i] = sprite;
				nonplaceableCursors[i] = sprite;
			}
		}
	}
}

void TileSet::SetCursorSprites(CursorType type, const Sprite& placeableSprite, const Sprite& nonplaceableSprite) {
	placeableCursors[type] = placeableSprite;
	nonplaceableCursors[type] = nonplaceableSprite;
	if (type == Cursor_None) {
		for (CursorTypeSpriteArray::size_type i = 0; i < placeableCursors.size(); ++i) {
			if (!placeableCursors[i].Exists()) {
				placeableCursors[i] = placeableSprite;
				nonplaceableCursors[i] = nonplaceableSprite;
			}
		}
	}
}

void TileSet::SetStatusSprite(StatusEffectType statusEffect, const Sprite& sprite) {
	defaultStatusEffects[statusEffect] = sprite;
}

void TileSet::SetDefaultUnderConstructionSprite(const Sprite& sprite) {
	defaultUnderConstructionSprite = sprite;
}

void TileSet::AddNPCSpriteSet(std::string name, const NPCSpriteSet& set) {
	int index = npcSpriteSets.size();
	npcSpriteSets.push_back(set);
	npcSpriteLookup[name] = index;
}

void TileSet::SetDefaultNPCSpriteSet(const NPCSpriteSet& set) {
	defaultNPCSpriteSet = set;
}

void TileSet::AddNatureObjectSpriteSet(std::string name, const NatureObjectSpriteSet& set) {
	int index = natureObjectSpriteSets.size();
	natureObjectSpriteSets.push_back(set);
	natureObjectSpriteLookup[name] = index;
}

void TileSet::SetDefaultNatureObjectSpriteSet(const NatureObjectSpriteSet& set) {
	defaultNatureObjectSpriteSet = set;
}

void TileSet::AddItemSpriteSet(std::string name, const ItemSpriteSet& set) {
	int index = itemSpriteSets.size();
	itemSpriteSets.push_back(set);
	itemSpriteLookup[name] = index;
}

void TileSet::SetDefaultItemSpriteSet(const ItemSpriteSet& set) {
	defaultItemSpriteSet = set;
}

void TileSet::AddConstructionSpriteSet(std::string name, const ConstructionSpriteSet& set) {
	int index = constructionSpriteSets.size();
	constructionSpriteSets.push_back(set);
	constructionSpriteLookup[name] = index;
}

void TileSet::SetDefaultConstructionSpriteSet(const ConstructionSpriteSet& set) {
	defaultConstructionSpriteSet = set;
}