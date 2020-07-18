/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2020  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "otpch.h"

#include "item.h"
#include "container.h"
#include "teleport.h"
#include "trashholder.h"
#include "mailbox.h"
#include "house.h"
#include "game.h"
#include "bed.h"

#include "actions.h"
#include "spells.h"

Items Item::items;

Item* Item::CreateItem(const uint16_t type, uint16_t count /*= 0*/)
{
	Item* newItem = nullptr;

	const ItemType& it = Item::items[type];
	if (it.group == ITEM_GROUP_DEPRECATED) {
		return nullptr;
	}

	if (it.stackable && count == 0) {
		count = 1;
	}

	if (it.id != 0) {
		if (it.isDepot()) {
			newItem = new DepotLocker(type);
		} else if (it.isContainer()) {
			newItem = new Container(type);
		} else if (it.isTeleport()) {
			newItem = new Teleport(type);
		} else if (it.isMagicField()) {
			newItem = new MagicField(type);
		} else if (it.isDoor()) {
			newItem = new Door(type);
		} else if (it.isTrashHolder()) {
			newItem = new TrashHolder(type);
		} else if (it.isMailbox()) {
			newItem = new Mailbox(type);
		} else if (it.isBed()) {
			newItem = new BedItem(type);
		} else if (it.id >= 2210 && it.id <= 2212) {
			newItem = new Item(type - 3, count);
		} else if (it.id == 2215 || it.id == 2216) {
			newItem = new Item(type - 2, count);
		} else if (it.id >= 2202 && it.id <= 2206) {
			newItem = new Item(type - 37, count);
		} else if (it.id == 2640) {
			newItem = new Item(6132, count);
		} else if (it.id == 6301) {
			newItem = new Item(6300, count);
		} else if (it.id == 18528) {
			newItem = new Item(18408, count);
		} else {
			newItem = new Item(type, count);
		}

		newItem->incrementReferenceCounter();
	}

	return newItem;
}

Container* Item::CreateItemAsContainer(const uint16_t type, uint16_t size)
{
	const ItemType& it = Item::items[type];
	if (it.id == 0 || it.group == ITEM_GROUP_DEPRECATED || it.stackable || it.useable || it.moveable || it.pickupable || it.isDepot() || it.isSplash() || it.isDoor()) {
		return nullptr;
	}

	Container* newItem = new Container(type, size);
	newItem->incrementReferenceCounter();
	return newItem;
}

Item* Item::CreateItem(PropStream& propStream)
{
	uint16_t id;
	if (!propStream.read<uint16_t>(id)) {
		return nullptr;
	}

	switch (id) {
		case ITEM_FIREFIELD_PVP_FULL: id = ITEM_FIREFIELD_PERSISTENT_FULL; break;
		case ITEM_FIREFIELD_PVP_MEDIUM: id = ITEM_FIREFIELD_PERSISTENT_MEDIUM; break;
		case ITEM_FIREFIELD_PVP_SMALL: id = ITEM_FIREFIELD_PERSISTENT_SMALL; break;
		case ITEM_ENERGYFIELD_PVP: id = ITEM_ENERGYFIELD_PERSISTENT; break;
		case ITEM_POISONFIELD_PVP: id = ITEM_POISONFIELD_PERSISTENT; break;
		case ITEM_MAGICWALL: id = ITEM_MAGICWALL_PERSISTENT; break;
		case ITEM_WILDGROWTH: id = ITEM_WILDGROWTH_PERSISTENT; break;
		default: break;
	}

	return Item::CreateItem(id, 0);
}

Item* Item::CreateItem_legacy(PropStream& propStream)
{
	uint16_t id;
	if (!propStream.read<uint16_t>(id)) {
		return nullptr;
	}

	switch (id) {
		case ITEM_FIREFIELD_PVP_FULL: id = ITEM_FIREFIELD_PERSISTENT_FULL; break;
		case ITEM_FIREFIELD_PVP_MEDIUM: id = ITEM_FIREFIELD_PERSISTENT_MEDIUM; break;
		case ITEM_FIREFIELD_PVP_SMALL: id = ITEM_FIREFIELD_PERSISTENT_SMALL; break;
		case ITEM_ENERGYFIELD_PVP: id = ITEM_ENERGYFIELD_PERSISTENT; break;
		case ITEM_POISONFIELD_PVP: id = ITEM_POISONFIELD_PERSISTENT; break;
		case ITEM_MAGICWALL: id = ITEM_MAGICWALL_PERSISTENT; break;
		case ITEM_WILDGROWTH: id = ITEM_WILDGROWTH_PERSISTENT; break;
		default: break;
	}

	const ItemType& iType = Item::items[id];
	uint8_t count = 0;
	if (iType.stackable || iType.isSplash() || iType.isFluidContainer()) {
		if (!propStream.read<uint8_t>(count)) {
			return nullptr;
		}
	}
	return Item::CreateItem(id, count);
}

Item::Item(const uint16_t type, uint16_t count /*= 0*/) :
	id(type)
{
  if (items.size() > 0) {
    const ItemType& it = items[id];

    if (it.isFluidContainer() || it.isSplash()) {
      setFluidType(count);
    } else if (it.stackable) {
      if (count != 0) {
        setItemCount(count);
      } else if (it.charges != 0) {
        setItemCount(it.charges);
      }
    } else if (it.charges != 0) {
      if (count != 0) {
        setCharges(count);
      } else {
        setCharges(it.charges);
      }
    }

    setDefaultDuration();
  }
}

Item::Item(const Item& i) :
	Thing(), id(i.id), count(i.count), loadedFromMap(i.loadedFromMap)
{
	if (i.attributes) {
		attributes.reset(new ItemAttributes(*i.attributes));
	}
}

Item* Item::clone() const
{
	Item* item = Item::CreateItem(id, count);
	if (attributes) {
		item->attributes.reset(new ItemAttributes(*attributes));
	}
	return item;
}

bool Item::equals(const Item* otherItem) const
{
	if (!otherItem || id != otherItem->id) {
		return false;
	}

	if (!attributes || attributes->attributeBits == 0) {
		return (!otherItem->attributes || otherItem->attributes->attributeBits == 0);
	}

	const auto& otherAttributes = otherItem->attributes;
	if (!otherAttributes || attributes->attributeBits != otherAttributes->attributeBits) {
		return false;
	}

	const auto& attributeList = attributes->attributes;
	const auto& otherAttributeList = otherAttributes->attributes;
	for (const auto& attribute : attributeList) {
		if (ItemAttributes::isStrAttrType(attribute.type)) {
			for (const auto& otherAttribute : otherAttributeList) {
				if (attribute.type == otherAttribute.type && *attribute.value.string != *otherAttribute.value.string) {
					return false;
				}
			}
		} else {
			for (const auto& otherAttribute : otherAttributeList) {
				if (attribute.type == otherAttribute.type && attribute.value.integer != otherAttribute.value.integer) {
					return false;
				}
			}
		}
	}
	return true;
}

void Item::setDefaultSubtype()
{
	const ItemType& it = items[id];

	setItemCount(1);

	if (it.charges != 0) {
		if (it.stackable) {
			setItemCount(it.charges);
		} else {
			setCharges(it.charges);
		}
	}
}

void Item::onRemoved()
{
	ScriptEnvironment::removeTempItem(this);

	if (hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
		g_game().removeUniqueItem(getUniqueId());
	}
}

void Item::setID(uint16_t newid)
{
	const ItemType& prevIt = Item::items[id];
	id = newid;

	const ItemType& it = Item::items[newid];
	uint32_t newDuration = it.decayTime * 1000;

	if (newDuration == 0 && !it.stopTime && it.decayTo < 0) {
		//We'll get called startDecay anyway so let's schedule it - actually not in all casses
		if (hasAttribute(ITEM_ATTRIBUTE_DECAYSTATE)) {
			setDecaying(DECAYING_STOPPING);
		}
		removeAttribute(ITEM_ATTRIBUTE_DURATION);
	}

	removeAttribute(ITEM_ATTRIBUTE_CORPSEOWNER);

	if (newDuration > 0 && (!prevIt.stopTime || !hasAttribute(ITEM_ATTRIBUTE_DURATION))) {
		setDecaying(DECAYING_PENDING);
		setDuration(newDuration);
	}
}

Cylinder* Item::getTopParent()
{
	Cylinder* aux = getParent();
	Cylinder* prevaux = dynamic_cast<Cylinder*>(this);
	if (!aux) {
		return prevaux;
	}

	while (aux->getParent() != nullptr) {
		prevaux = aux;
		aux = aux->getParent();
	}

	if (prevaux) {
		return prevaux;
	}
	return aux;
}

const Cylinder* Item::getTopParent() const
{
	const Cylinder* aux = getParent();
	const Cylinder* prevaux = dynamic_cast<const Cylinder*>(this);
	if (!aux) {
		return prevaux;
	}

	while (aux->getParent() != nullptr) {
		prevaux = aux;
		aux = aux->getParent();
	}

	if (prevaux) {
		return prevaux;
	}
	return aux;
}

Tile* Item::getTile()
{
	Cylinder* cylinder = getTopParent();
	//get root cylinder
	if (cylinder && cylinder->getParent()) {
		cylinder = cylinder->getParent();
	}
	return dynamic_cast<Tile*>(cylinder);
}

const Tile* Item::getTile() const
{
	const Cylinder* cylinder = getTopParent();
	//get root cylinder
	if (cylinder && cylinder->getParent()) {
		cylinder = cylinder->getParent();
	}
	return dynamic_cast<const Tile*>(cylinder);
}

uint16_t Item::getSubType() const
{
	const ItemType& it = items[id];
	if (it.isFluidContainer() || it.isSplash()) {
		return getFluidType();
	} else if (it.stackable) {
		return count;
	} else if (it.charges != 0) {
		return getCharges();
	}
	return count;
}

Player* Item::getHoldingPlayer() const
{
	Cylinder* p = getParent();
	while (p) {
		if (p->getCreature()) {
			return p->getCreature()->getPlayer();
		}

		p = p->getParent();
	}
	return nullptr;
}

void Item::setSubType(uint16_t n)
{
	const ItemType& it = items[id];
	if (it.isFluidContainer() || it.isSplash()) {
		setFluidType(n);
	} else if (it.stackable) {
		setItemCount(n);
	} else if (it.charges != 0) {
		setCharges(n);
	} else {
		setItemCount(n);
	}
}

Attr_ReadValue Item::readAttr(AttrTypes_t attr, PropStream& propStream)
{
	switch (attr) {
		case ATTR_COUNT:
		case ATTR_RUNE_CHARGES: {
			uint8_t count;
			if (!propStream.read<uint8_t>(count)) {
				return ATTR_READ_ERROR;
			}

			setSubType(count);
			break;
		}

		case ATTR_ACTION_ID: {
			uint16_t actionId;
			if (!propStream.read<uint16_t>(actionId)) {
				return ATTR_READ_ERROR;
			}

			setActionId(actionId);
			break;
		}

		case ATTR_UNIQUE_ID: {
			uint16_t uniqueId;
			if (!propStream.read<uint16_t>(uniqueId)) {
				return ATTR_READ_ERROR;
			}

			setUniqueId(uniqueId);
			break;
		}

		case ATTR_TEXT: {
			std::string text;
			if (!propStream.readString(text)) {
				return ATTR_READ_ERROR;
			}

			setText(text);
			break;
		}

		case ATTR_WRITTENDATE: {
			uint32_t writtenDate;
			if (!propStream.read<uint32_t>(writtenDate)) {
				return ATTR_READ_ERROR;
			}

			setDate(writtenDate);
			break;
		}

		case ATTR_WRITTENBY: {
			std::string writer;
			if (!propStream.readString(writer)) {
				return ATTR_READ_ERROR;
			}

			setWriter(writer);
			break;
		}

		case ATTR_DESC: {
			std::string text;
			if (!propStream.readString(text)) {
				return ATTR_READ_ERROR;
			}

			setSpecialDescription(text);
			break;
		}

		case ATTR_CHARGES: {
			uint16_t charges;
			if (!propStream.read<uint16_t>(charges)) {
				return ATTR_READ_ERROR;
			}

			setSubType(charges);
			break;
		}

		case ATTR_DURATION: {
			int32_t duration;
			if (!propStream.read<int32_t>(duration)) {
				return ATTR_READ_ERROR;
			}

			setDuration(duration);
			break;
		}

		case ATTR_DECAYING_STATE: {
			uint8_t state;
			if (!propStream.read<uint8_t>(state)) {
				return ATTR_READ_ERROR;
			}

			if (state != DECAYING_FALSE) {
				setDecaying(DECAYING_PENDING);
			}
			break;
		}

		case ATTR_NAME: {
			std::string name;
			if (!propStream.readString(name)) {
				return ATTR_READ_ERROR;
			}

			setStrAttr(ITEM_ATTRIBUTE_NAME, name);
			break;
		}

		case ATTR_ARTICLE: {
			std::string article;
			if (!propStream.readString(article)) {
				return ATTR_READ_ERROR;
			}

			setStrAttr(ITEM_ATTRIBUTE_ARTICLE, article);
			break;
		}

		case ATTR_PLURALNAME: {
			std::string pluralName;
			if (!propStream.readString(pluralName)) {
				return ATTR_READ_ERROR;
			}

			setStrAttr(ITEM_ATTRIBUTE_PLURALNAME, pluralName);
			break;
		}

		case ATTR_WEIGHT: {
			uint32_t weight;
			if (!propStream.read<uint32_t>(weight)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_WEIGHT, weight);
			break;
		}

		case ATTR_ATTACK: {
			int32_t attack;
			if (!propStream.read<int32_t>(attack)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_ATTACK, attack);
			break;
		}

		case ATTR_DEFENSE: {
			int32_t defense;
			if (!propStream.read<int32_t>(defense)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_DEFENSE, defense);
			break;
		}

		case ATTR_EXTRADEFENSE: {
			int32_t extraDefense;
			if (!propStream.read<int32_t>(extraDefense)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_EXTRADEFENSE, extraDefense);
			break;
		}

		case ATTR_ARMOR: {
			int32_t armor;
			if (!propStream.read<int32_t>(armor)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_ARMOR, armor);
			break;
		}

		case ATTR_HITCHANCE: {
			int8_t hitChance;
			if (!propStream.read<int8_t>(hitChance)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_HITCHANCE, hitChance);
			break;
		}

		case ATTR_SHOOTRANGE: {
			uint8_t shootRange;
			if (!propStream.read<uint8_t>(shootRange)) {
				return ATTR_READ_ERROR;
			}

			setIntAttr(ITEM_ATTRIBUTE_SHOOTRANGE, shootRange);
			break;
		}

		//these should be handled through derived classes
		//If these are called then something has changed in the items.xml since the map was saved
		//just read the values

		//Depot class
		case ATTR_DEPOT_ID: {
			if (!propStream.skip(2)) {
				return ATTR_READ_ERROR;
			}
			break;
		}

		//Door class
		case ATTR_HOUSEDOORID: {
			if (!propStream.skip(1)) {
				return ATTR_READ_ERROR;
			}
			break;
		}

		//Bed class
		case ATTR_SLEEPERGUID: {
			if (!propStream.skip(4)) {
				return ATTR_READ_ERROR;
			}
			break;
		}

		case ATTR_SLEEPSTART: {
			if (!propStream.skip(4)) {
				return ATTR_READ_ERROR;
			}
			break;
		}

		//Teleport class
		case ATTR_TELE_DEST: {
			if (!propStream.skip(5)) {
				return ATTR_READ_ERROR;
			}
			break;
		}

		//Container class
		case ATTR_CONTAINER_ITEMS: {
			return ATTR_READ_ERROR;
		}

		case ATTR_CUSTOM_ATTRIBUTES: {
			uint64_t size;
			if (!propStream.read<uint64_t>(size)) {
				return ATTR_READ_ERROR;
			}

			for (uint64_t i = 0; i < size; i++) {
				// Unserialize key type and value
				std::string key;
				if (!propStream.readString(key)) {
					return ATTR_READ_ERROR;
				};

				// Unserialize value type and value
				ItemAttributes::CustomAttribute val;
				if (!val.unserialize(propStream)) {
					return ATTR_READ_ERROR;
				}

				setCustomAttribute(key, val);
			}
			break;
		}

		default:
			return ATTR_READ_ERROR;
	}

	return ATTR_READ_CONTINUE;
}

bool Item::unserializeAttr(PropStream& propStream)
{
	uint8_t attr_type;
	while (propStream.read<uint8_t>(attr_type) && attr_type != 0) {
		Attr_ReadValue ret = readAttr(static_cast<AttrTypes_t>(attr_type), propStream);
		if (ret == ATTR_READ_ERROR) {
			return false;
		} else if (ret == ATTR_READ_END) {
			return true;
		}
	}
	return true;
}

bool Item::unserializeItemNode(OTB::Loader&, const OTB::Node&, PropStream& propStream, bool)
{
	return unserializeAttr(propStream);
}

void Item::serializeAttr(PropWriteStream& propWriteStream) const
{
	const ItemType& it = items[id];
	if (it.stackable || it.isFluidContainer() || it.isSplash()) {
		propWriteStream.write<uint8_t>(ATTR_COUNT);
		propWriteStream.write<uint8_t>(getSubType());
	}

	uint16_t charges = getCharges();
	if (charges != 0) {
		propWriteStream.write<uint8_t>(ATTR_CHARGES);
		propWriteStream.write<uint16_t>(charges);
	}

	if (it.moveable) {
		uint16_t actionId = getActionId();
		if (actionId != 0) {
			propWriteStream.write<uint8_t>(ATTR_ACTION_ID);
			propWriteStream.write<uint16_t>(actionId);
		}
	}

	const std::string& text = getText();
	if (!text.empty()) {
		propWriteStream.write<uint8_t>(ATTR_TEXT);
		propWriteStream.writeString(text);
	}

	const time_t writtenDate = getDate();
	if (writtenDate != 0) {
		propWriteStream.write<uint8_t>(ATTR_WRITTENDATE);
		propWriteStream.write<uint32_t>(writtenDate);
	}

	const std::string& writer = getWriter();
	if (!writer.empty()) {
		propWriteStream.write<uint8_t>(ATTR_WRITTENBY);
		propWriteStream.writeString(writer);
	}

	const std::string& specialDesc = getSpecialDescription();
	if (!specialDesc.empty()) {
		propWriteStream.write<uint8_t>(ATTR_DESC);
		propWriteStream.writeString(specialDesc);
	}

	if (hasAttribute(ITEM_ATTRIBUTE_DURATION)) {
		propWriteStream.write<uint8_t>(ATTR_DURATION);
		propWriteStream.write<int32_t>(getDuration());
	}

	ItemDecayState_t decayState = getDecaying();
	if (decayState == DECAYING_TRUE || decayState == DECAYING_PENDING) {
		propWriteStream.write<uint8_t>(ATTR_DECAYING_STATE);
		propWriteStream.write<uint8_t>(decayState);
	}

	if (hasAttribute(ITEM_ATTRIBUTE_NAME)) {
		propWriteStream.write<uint8_t>(ATTR_NAME);
		propWriteStream.writeString(getStrAttr(ITEM_ATTRIBUTE_NAME));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_ARTICLE)) {
		propWriteStream.write<uint8_t>(ATTR_ARTICLE);
		propWriteStream.writeString(getStrAttr(ITEM_ATTRIBUTE_ARTICLE));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_PLURALNAME)) {
		propWriteStream.write<uint8_t>(ATTR_PLURALNAME);
		propWriteStream.writeString(getStrAttr(ITEM_ATTRIBUTE_PLURALNAME));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_WEIGHT)) {
		propWriteStream.write<uint8_t>(ATTR_WEIGHT);
		propWriteStream.write<uint32_t>(getIntAttr(ITEM_ATTRIBUTE_WEIGHT));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_ATTACK)) {
		propWriteStream.write<uint8_t>(ATTR_ATTACK);
		propWriteStream.write<int32_t>(getIntAttr(ITEM_ATTRIBUTE_ATTACK));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_DEFENSE)) {
		propWriteStream.write<uint8_t>(ATTR_DEFENSE);
		propWriteStream.write<int32_t>(getIntAttr(ITEM_ATTRIBUTE_DEFENSE));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_EXTRADEFENSE)) {
		propWriteStream.write<uint8_t>(ATTR_EXTRADEFENSE);
		propWriteStream.write<int32_t>(getIntAttr(ITEM_ATTRIBUTE_EXTRADEFENSE));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_ARMOR)) {
		propWriteStream.write<uint8_t>(ATTR_ARMOR);
		propWriteStream.write<int32_t>(getIntAttr(ITEM_ATTRIBUTE_ARMOR));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_HITCHANCE)) {
		propWriteStream.write<uint8_t>(ATTR_HITCHANCE);
		propWriteStream.write<int8_t>(getIntAttr(ITEM_ATTRIBUTE_HITCHANCE));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_SHOOTRANGE)) {
		propWriteStream.write<uint8_t>(ATTR_SHOOTRANGE);
		propWriteStream.write<uint8_t>(getIntAttr(ITEM_ATTRIBUTE_SHOOTRANGE));
	}

	if (hasAttribute(ITEM_ATTRIBUTE_CUSTOM)) {
		const ItemAttributes::CustomAttributeMap* customAttrMap = attributes->getCustomAttributeMap();
		propWriteStream.write<uint8_t>(ATTR_CUSTOM_ATTRIBUTES);
		propWriteStream.write<uint64_t>(static_cast<uint64_t>(customAttrMap->size()));
		for (const auto &entry : *customAttrMap) {
			// Serializing key type and value
			propWriteStream.writeString(entry.first);

			// Serializing value type and value
			entry.second.serialize(propWriteStream);
		}
	}
}

bool Item::hasProperty(ITEMPROPERTY prop) const
{
	const ItemType& it = items[id];
	switch (prop) {
		case CONST_PROP_BLOCKSOLID: return it.blockSolid;
		case CONST_PROP_MOVEABLE: return it.moveable && !hasAttribute(ITEM_ATTRIBUTE_UNIQUEID);
		case CONST_PROP_HASHEIGHT: return it.hasHeight;
		case CONST_PROP_BLOCKPROJECTILE: return it.blockProjectile;
		case CONST_PROP_BLOCKPATH: return it.blockPathFind;
		case CONST_PROP_ISVERTICAL: return it.isVertical;
		case CONST_PROP_ISHORIZONTAL: return it.isHorizontal;
		case CONST_PROP_IMMOVABLEBLOCKSOLID: return it.blockSolid && (!it.moveable || hasAttribute(ITEM_ATTRIBUTE_UNIQUEID));
		case CONST_PROP_IMMOVABLEBLOCKPATH: return it.blockPathFind && (!it.moveable || hasAttribute(ITEM_ATTRIBUTE_UNIQUEID));
		case CONST_PROP_IMMOVABLENOFIELDBLOCKPATH: return !it.isMagicField() && it.blockPathFind && (!it.moveable || hasAttribute(ITEM_ATTRIBUTE_UNIQUEID));
		case CONST_PROP_NOFIELDBLOCKPATH: return !it.isMagicField() && it.blockPathFind;
		case CONST_PROP_SUPPORTHANGABLE: return it.isHorizontal || it.isVertical;
		default: return false;
	}
}

uint32_t Item::getWeight() const
{
	uint32_t weight = getBaseWeight();
	if (isStackable()) {
		return weight * std::max<uint32_t>(1, getItemCount());
	}
	return weight;
}

std::vector<std::pair<std::string, std::string>> Item::getDescriptions(const ItemType& it, const Item* item /*= nullptr*/)
{
	char buffer[24]; // Enough to contain uint64_t
	std::string str;
	str.reserve(32);

	std::vector<std::pair<std::string, std::string>> descriptions;
	descriptions.reserve(30);
	if (item) {
		const std::string& specialDescription = item->getSpecialDescription();
		if (!specialDescription.empty()) {
			descriptions.emplace_back("Description", specialDescription);
		} else if (!it.description.empty()) {
			descriptions.emplace_back("Description", it.description);
		}

		if (it.showCharges) {
			int32_t charges = item->getCharges();
			if (charges != 0) {
				descriptions.emplace_back("Charges", std::to_string(charges));
			}
		}

		int32_t attack = item->getAttack();
		if (attack != 0) {
			if (it.abilities && it.abilities->elementType != COMBAT_NONE && it.abilities->elementDamage != 0) {
				str.clear();
				str.append(std::to_string(attack)).append(" physical +").append(std::to_string(static_cast<uint32_t>(it.abilities->elementDamage))).append(1, ' ').append(getCombatName(it.abilities->elementType));
				descriptions.emplace_back("Attack", str);
			} else {
				descriptions.emplace_back("Attack", std::to_string(attack));
			}
		}

		int32_t hitChance = item->getHitChance();
		if (hitChance != 0) {
			descriptions.emplace_back("HitChance", std::to_string(hitChance));
		}

		int32_t defense = item->getDefense(), extraDefense = item->getExtraDefense();
		if (defense != 0 || extraDefense != 0) {
			if (extraDefense != 0) {
				int32_t len = sprintf(buffer, " %+d", extraDefense);
				str.clear();
				str.append(std::to_string(defense)).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Defense", str);
			} else {
				descriptions.emplace_back("Defense", std::to_string(defense));
			}
		}

		int32_t armor = item->getArmor();
		if (armor != 0) {
			descriptions.emplace_back("Armor", std::to_string(armor));
		}

		if (it.abilities) {
			for (uint8_t i = SKILL_FIRST; i <= SKILL_LAST; i++) {
				if (!it.abilities->skills[i]) {
					continue;
				}

				int32_t len = sprintf(buffer, "%+d", it.abilities->skills[i]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back(getSkillName(i), str);
			}

			for (uint8_t i = SPECIALSKILL_FIRST; i <= SPECIALSKILL_LAST; i++) {
				if (!it.abilities->specialSkills[i]) {
					continue;
				}

				int32_t len = sprintf(buffer, "%+d%%", it.abilities->specialSkills[i]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back(getSpecialSkillName(i), str);
			}

			if (it.abilities->stats[STAT_MAGICPOINTS]) {
				int32_t len = sprintf(buffer, "%+d", it.abilities->stats[STAT_MAGICPOINTS]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Magic Level", str);
			}

			if (it.abilities->speed) {
				int32_t len = sprintf(buffer, "%+d", (it.abilities->speed >> 1));
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Speed", str);
			}

			if (hasBitSet(CONDITION_DRUNK, it.abilities->conditionSuppressions)) {
				descriptions.emplace_back("Effect", "Hard Drinking");
			}

			if (it.abilities->invisible) {
				descriptions.emplace_back("Effect", "Invisibility");
			}

			if (it.abilities->regeneration) {
				descriptions.emplace_back("Effect", "Faster Regeneration");
			}

			if (it.abilities->manaShield) {
				descriptions.emplace_back("Effect", "Mana Shield");
			}

			for (size_t i = 0; i < COMBAT_COUNT; ++i) {
				if (it.abilities->absorbPercent[i] == 0) {
					continue;
				}

				int32_t len = sprintf(buffer, " %+d%%", it.abilities->absorbPercent[i]);
				str.clear();
				str.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Protection", str);
			}

			for (size_t i = 0; i < COMBAT_COUNT; ++i) {
				if (it.abilities->fieldAbsorbPercent[i] == 0) {
					continue;
				}

				int32_t len = sprintf(buffer, " %+d%%", it.abilities->fieldAbsorbPercent[i]);
				str.clear();
				str.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Field Protection", str);
			}
		}

		if (it.isKey()) {
			int32_t len = sprintf(buffer, "%04d", static_cast<int32_t>(item->getActionId()));
			str.clear();
			str.append(buffer, static_cast<size_t>(len));
			descriptions.emplace_back("Key", str);
		}

		if (it.isFluidContainer()) {
			uint16_t subType = item->getSubType();
			if (subType > 0) {
				const std::string& itemName = items[subType].name;
				descriptions.emplace_back("Contain", (!itemName.empty() ? itemName : "Nothing"));
			} else {
				descriptions.emplace_back("Contain", "Nothing");
			}
		}

		if (item->getContainer()) {
			descriptions.emplace_back("Capacity", std::to_string(item->getContainer()->capacity()));
		}

		if (it.isRune()) {
			descriptions.emplace_back("Rune Spell Name", it.runeSpellName);
		}

		uint32_t weight = item->getWeight();
		if (weight != 0) {
			str.clear();
			if (weight < 10) {
				str.append("0.0").append(std::to_string(weight));
			} else if (weight < 100) {
				str.append("0.").append(std::to_string(weight));
			} else {
				std::string weightString = std::to_string(weight);
				weightString.insert(weightString.end() - 2, '.');
				str.append(weightString);
			}
			str.append(" oz");
			descriptions.emplace_back("Weight", str);
		}

		if (it.showDuration) {
			str.clear();
			if (item->hasAttribute(ITEM_ATTRIBUTE_DURATION)) {
				uint32_t duration = item->getDuration() / 1000;
				str.append("Will expire in ");
				if (duration >= 86400) {
					uint32_t days = duration / 86400;
					uint32_t hours = (duration % 86400) / 3600;
					str.append(std::to_string(days)).append(" day").append(days != 1 ? "s" : "");

					if (hours > 0) {
						str.append(" and ").append(std::to_string(hours)).append(" hour").append(hours != 1 ? "s" : "");
					}
				} else if (duration >= 3600) {
					uint32_t hours = duration / 3600;
					uint32_t minutes = (duration % 3600) / 60;
					str.append(std::to_string(hours)).append(" hour").append(hours != 1 ? "s" : "");

					if (minutes > 0) {
						str.append(" and ").append(std::to_string(minutes)).append(" minute").append(minutes != 1 ? "s" : "");
					}
				} else if (duration >= 60) {
					uint32_t minutes = duration / 60;
					uint32_t seconds = duration % 60;
					str.append(std::to_string(minutes)).append(" minute").append(minutes != 1 ? "s" : "");

					if (seconds > 0) {
						str.append(" and ").append(std::to_string(seconds)).append(" second").append(seconds != 1 ? "s" : "");
					}
				} else {
					str.append(std::to_string(duration)).append(" second").append(duration != 1 ? "s" : "");
				}
			} else {
				str.append("Is brand-new");
			}
			descriptions.emplace_back("Expiration", str);
		}

		if (it.wieldInfo & WIELDINFO_PREMIUM) {
			descriptions.emplace_back("Required", "Premium");
		}

		if (it.minReqLevel != 0) {
			descriptions.emplace_back("Required Level", std::to_string(it.minReqLevel));
		}

		if (it.minReqMagicLevel != 0) {
			descriptions.emplace_back("Required Magic Level", std::to_string(it.minReqMagicLevel));
		}

		if (!it.vocationString.empty()) {
			descriptions.emplace_back("Professions", it.vocationString);
		}

		std::string weaponName = getWeaponName(it.weaponType);
		if (it.slotPosition & SLOTP_TWO_HAND) {
			if (!weaponName.empty()) {
				weaponName += ", two-handed";
			} else {
				weaponName = "two-handed";
			}
		}
		if (!weaponName.empty()) {
			descriptions.emplace_back("Weapon Type", weaponName);
		}

		if (it.slotPosition & SLOTP_BACKPACK) {
			descriptions.emplace_back("Body Position", "Container");
		} else if (it.slotPosition & SLOTP_HEAD) {
			descriptions.emplace_back("Body Position", "Head");
		} else if (it.slotPosition & SLOTP_ARMOR) {
			descriptions.emplace_back("Body Position", "Body");
		} else if (it.slotPosition & SLOTP_LEGS) {
			descriptions.emplace_back("Body Position", "Legs");
		} else if (it.slotPosition & SLOTP_FEET) {
			descriptions.emplace_back("Body Position", "Feet");
		} else if (it.slotPosition & SLOTP_NECKLACE) {
			descriptions.emplace_back("Body Position", "Neck");
		} else if (it.slotPosition & SLOTP_RING) {
			descriptions.emplace_back("Body Position", "Finger");
		} else if (it.slotPosition & SLOTP_AMMO) {
			descriptions.emplace_back("Body Position", "Extra Slot");
		} else if (it.slotPosition & SLOTP_TWO_HAND || it.slotPosition & SLOTP_LEFT || it.slotPosition & SLOTP_RIGHT) {
			descriptions.emplace_back("Body Position", "Hand");
		}
	} else {
		if (!it.description.empty()) {
			descriptions.emplace_back("Description", it.description);
		}

		if (it.showCharges) {
			int32_t charges = it.charges;
			if (charges != 0) {
				descriptions.emplace_back("Charges", std::to_string(charges));
			}
		}

		int32_t attack = it.attack;
		if (attack != 0) {
			if (it.abilities && it.abilities->elementType != COMBAT_NONE && it.abilities->elementDamage != 0) {
				str.clear();
				str.append(std::to_string(attack)).append(" physical +").append(std::to_string(static_cast<uint32_t>(it.abilities->elementDamage))).append(1, ' ').append(getCombatName(it.abilities->elementType));
				descriptions.emplace_back("Attack", str);
			} else {
				descriptions.emplace_back("Attack", std::to_string(attack));
			}
		}

		int32_t hitChance = it.hitChance;
		if (hitChance != 0) {
			descriptions.emplace_back("HitChance", std::to_string(hitChance));
		}

		int32_t defense = it.defense, extraDefense = it.extraDefense;
		if (defense != 0 || extraDefense != 0) {
			if (extraDefense != 0) {
				int32_t len = sprintf(buffer, " %+d", extraDefense);
				str.clear();
				str.append(std::to_string(defense)).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Defense", str);
			} else {
				descriptions.emplace_back("Defense", std::to_string(defense));
			}
		}

		int32_t armor = it.armor;
		if (armor != 0) {
			descriptions.emplace_back("Armor", std::to_string(armor));
		}

		if (it.abilities) {
			for (uint8_t i = SKILL_FIRST; i <= SKILL_LAST; i++) {
				if (!it.abilities->skills[i]) {
					continue;
				}

				int32_t len = sprintf(buffer, "%+d", it.abilities->skills[i]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back(getSkillName(i), str);
			}

			for (uint8_t i = SPECIALSKILL_FIRST; i <= SPECIALSKILL_LAST; i++) {
				if (!it.abilities->specialSkills[i]) {
					continue;
				}

				int32_t len = sprintf(buffer, "%+d%%", it.abilities->specialSkills[i]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back(getSpecialSkillName(i), str);
			}

			if (it.abilities->stats[STAT_MAGICPOINTS]) {
				int32_t len = sprintf(buffer, "%+d", it.abilities->stats[STAT_MAGICPOINTS]);
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Magic Level", str);
			}

			if (it.abilities->speed) {
				int32_t len = sprintf(buffer, "%+d", (it.abilities->speed >> 1));
				str.clear();
				str.append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Speed", str);
			}

			if (hasBitSet(CONDITION_DRUNK, it.abilities->conditionSuppressions)) {
				descriptions.emplace_back("Effect", "Hard Drinking");
			}

			if (it.abilities->invisible) {
				descriptions.emplace_back("Effect", "Invisibility");
			}

			if (it.abilities->regeneration) {
				descriptions.emplace_back("Effect", "Faster Regeneration");
			}

			if (it.abilities->manaShield) {
				descriptions.emplace_back("Effect", "Mana Shield");
			}

			for (size_t i = 0; i < COMBAT_COUNT; ++i) {
				if (it.abilities->absorbPercent[i] == 0) {
					continue;
				}

				int32_t len = sprintf(buffer, " %+d%%", static_cast<int32_t>(it.abilities->absorbPercent[i]));
				str.clear();
				str.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Protection", str);
			}

			for (size_t i = 0; i < COMBAT_COUNT; ++i) {
				if (it.abilities->fieldAbsorbPercent[i] == 0) {
					continue;
				}

				int32_t len = sprintf(buffer, " %+d%%", static_cast<int32_t>(it.abilities->fieldAbsorbPercent[i]));
				str.clear();
				str.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				descriptions.emplace_back("Field Protection", str);
			}
		}

		if (it.isKey()) {
			descriptions.emplace_back("Key", "0000");
		}

		if (it.isFluidContainer()) {
			descriptions.emplace_back("Contain", "Nothing");
		}

		if (it.isContainer()) {
			descriptions.emplace_back("Capacity", std::to_string(it.maxItems));
		}

		if (it.isRune()) {
			descriptions.emplace_back("Rune Spell Name", it.runeSpellName);
		}

		uint32_t weight = it.weight;
		if (weight != 0) {
			str.clear();
			if (weight < 10) {
				str.append("0.0").append(std::to_string(weight));
			} else if (weight < 100) {
				str.append("0.").append(std::to_string(weight));
			} else {
				std::string weightString = std::to_string(weight);
				weightString.insert(weightString.end() - 2, '.');
				str.append(weightString);
			}
			str.append(" oz");
			descriptions.emplace_back("Weight", str);
		}

		if (it.showDuration) {
			descriptions.emplace_back("Expiration", "Is brand-new");
		}

		if (it.wieldInfo & WIELDINFO_PREMIUM) {
			descriptions.emplace_back("Required", "Premium");
		}

		if (it.minReqLevel != 0) {
			descriptions.emplace_back("Required Level", std::to_string(it.minReqLevel));
		}

		if (it.minReqMagicLevel != 0) {
			descriptions.emplace_back("Required Magic Level", std::to_string(it.minReqMagicLevel));
		}

		if (!it.vocationString.empty()) {
			descriptions.emplace_back("Professions", it.vocationString);
		}

		std::string weaponName = getWeaponName(it.weaponType);
		if (it.slotPosition & SLOTP_TWO_HAND) {
			if (!weaponName.empty()) {
				weaponName += ", two-handed";
			} else {
				weaponName = "two-handed";
			}
		}
		if (!weaponName.empty()) {
			descriptions.emplace_back("Weapon Type", weaponName);
		}

		if (it.slotPosition & SLOTP_BACKPACK) {
			descriptions.emplace_back("Body Position", "Container");
		} else if (it.slotPosition & SLOTP_HEAD) {
			descriptions.emplace_back("Body Position", "Head");
		} else if (it.slotPosition & SLOTP_ARMOR) {
			descriptions.emplace_back("Body Position", "Body");
		} else if (it.slotPosition & SLOTP_LEGS) {
			descriptions.emplace_back("Body Position", "Legs");
		} else if (it.slotPosition & SLOTP_FEET) {
			descriptions.emplace_back("Body Position", "Feet");
		} else if (it.slotPosition & SLOTP_NECKLACE) {
			descriptions.emplace_back("Body Position", "Neck");
		} else if (it.slotPosition & SLOTP_RING) {
			descriptions.emplace_back("Body Position", "Finger");
		} else if (it.slotPosition & SLOTP_AMMO) {
			descriptions.emplace_back("Body Position", "Extra Slot");
		} else if (it.slotPosition & SLOTP_TWO_HAND || it.slotPosition & SLOTP_LEFT || it.slotPosition & SLOTP_RIGHT) {
			descriptions.emplace_back("Body Position", "Hand");
		}
	}
	return descriptions;
}

std::string Item::getDescription(const ItemType& it, int32_t lookDistance,
                                 const Item* item /*= nullptr*/, int32_t subType /*= -1*/, bool addArticle /*= true*/)
{
	const std::string* text = nullptr;
	char buffer[24]; // Enough to contain uint64_t

	std::string sink;
	sink.reserve(1024);
	sink.append(getNameDescription(it, item, subType, addArticle));

	if (item) {
		subType = item->getSubType();
	}
	if (it.isRune()) {
		if (it.runeLevel > 0 || it.runeMagLevel > 0) {
			if (RuneSpell* rune = g_spells().getRuneSpell(it.id)) {
				int32_t tmpSubType = subType;
				sink.append(". ").append(it.stackable && tmpSubType > 1 ? "They" : "It").append(" can only be used by ");

				const VocSpellMap& vocMap = rune->getVocMap();
				std::vector<Vocation*> showVocMap;

				// vocations are usually listed with the unpromoted and promoted version, the latter being
				// hidden from description, so `total / 2` is most likely the amount of vocations to be shown.
				showVocMap.reserve(vocMap.size() / 2);
				for (const auto& voc : vocMap) {
					if (voc.second) {
						showVocMap.push_back(g_vocations().getVocation(voc.first));
					}
				}

				if (!showVocMap.empty()) {
					auto vocIt = showVocMap.begin(), vocLast = (showVocMap.end() - 1);
					while (vocIt != vocLast) {
						sink.append(asLowerCaseString((*vocIt)->getVocName())).append("s");
						if (++vocIt == vocLast) {
							sink.append(" and ");
						} else {
							sink.append(", ");
						}
					}
					sink.append(asLowerCaseString((*vocLast)->getVocName())).append("s");
				} else {
					sink.append("players");
				}

				sink.append(" with");
				if (it.runeLevel > 0) {
					sink.append(" level ").append(std::to_string(it.runeLevel));
				}

				if (it.runeMagLevel > 0) {
					if (it.runeLevel > 0) {
						sink.append(" and");
						sink.append(" magic level ").append(std::to_string(it.runeMagLevel));
					}
				}

				sink.append(" or higher");
			}
		}
	} else if (it.weaponType != WEAPON_NONE) {
		bool begin = true;
		if (it.weaponType == WEAPON_DISTANCE && it.ammoType != AMMO_NONE) {
			sink.append(" (Range:").append(std::to_string(static_cast<uint32_t>(item ? item->getShootRange() : it.shootRange)));

			int32_t attack;
			int8_t hitChance;
			if (item) {
				attack = item->getAttack();
				hitChance = item->getHitChance();
			} else {
				attack = it.attack;
				hitChance = it.hitChance;
			}

			if (attack != 0) {
				int32_t len = sprintf(buffer, "%+d", attack);
				sink.append(", Atk").append(buffer, static_cast<size_t>(len));
			}

			if (hitChance != 0) {
				int32_t len = sprintf(buffer, "%+d", static_cast<int32_t>(hitChance));
				sink.append(", Hit%").append(buffer, static_cast<size_t>(len));
			}

			begin = false;
		} else if (it.weaponType != WEAPON_AMMO) {
			int32_t attack, defense, extraDefense;
			if (item) {
				attack = item->getAttack();
				defense = item->getDefense();
				extraDefense = item->getExtraDefense();
			} else {
				attack = it.attack;
				defense = it.defense;
				extraDefense = it.extraDefense;
			}

			if (attack != 0) {
				begin = false;
				sink.append(" (Atk:").append(std::to_string(attack));

				if (it.abilities && it.abilities->elementType != COMBAT_NONE && it.abilities->elementDamage != 0) {
					sink.append(" physical + ").append(std::to_string(static_cast<uint32_t>(it.abilities->elementDamage))).append(1, ' ').append(getCombatName(it.abilities->elementType));
				}
			}

			if (defense != 0 || extraDefense != 0) {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				sink.append("Def:").append(std::to_string(defense));
				if (extraDefense != 0) {
					int32_t len = sprintf(buffer, " %+d", extraDefense);
					sink.append(buffer, static_cast<size_t>(len));
				}
			}
		}

		if (it.abilities) {
			for (uint8_t i = SKILL_FIRST; i <= SKILL_LAST; i++) {
				if (!it.abilities->skills[i]) {
					continue;
				}

				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, " %+d", it.abilities->skills[i]);
				sink.append(getSkillName(i)).append(buffer, static_cast<size_t>(len));
			}

			for (uint8_t i = SPECIALSKILL_FIRST; i <= SPECIALSKILL_LAST; i++) {
				if (!it.abilities->specialSkills[i]) {
					continue;
				}

				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, " %+d%%", it.abilities->specialSkills[i]);
				sink.append(getSpecialSkillName(i)).append(buffer, static_cast<size_t>(len));
			}

			if (it.abilities->stats[STAT_MAGICPOINTS]) {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d", it.abilities->stats[STAT_MAGICPOINTS]);
				sink.append("magic level ").append(buffer, static_cast<size_t>(len));
			}

			int16_t show = it.abilities->absorbPercent[0];
			if (show != 0) {
				for (size_t i = 1; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] != show) {
						show = 0;
						break;
					}
				}
			}

			if (show == 0) {
				bool tmp = true;

				for (size_t i = 0; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] == 0) {
						continue;
					}

					if (tmp) {
						tmp = false;

						if (begin) {
							begin = false;
							sink.append(" (");
						} else {
							sink.append(", ");
						}

						sink.append("protection ");
					} else {
						sink.append(", ");
					}

					int32_t len = sprintf(buffer, " %+d%%", static_cast<int32_t>(it.abilities->absorbPercent[i]));
					sink.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				}
			} else {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(show));
				sink.append("protection all ").append(buffer, static_cast<size_t>(len));
			}

			show = it.abilities->fieldAbsorbPercent[0];
			if (show != 0) {
				for (size_t i = 1; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] != show) {
						show = 0;
						break;
					}
				}
			}

			if (show == 0) {
				bool tmp = true;

				for (size_t i = 0; i < COMBAT_COUNT; ++i) {
					if (it.abilities->fieldAbsorbPercent[i] == 0) {
						continue;
					}

					if (tmp) {
						tmp = false;

						if (begin) {
							begin = false;
							sink.append(" (");
						} else {
							sink.append(", ");
						}

						sink.append("protection ");
					} else {
						sink.append(", ");
					}

					int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(it.abilities->fieldAbsorbPercent[i]));
					sink.append(getCombatName(indexToCombatType(i))).append(" field ").append(buffer, static_cast<size_t>(len));
				}
			} else {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(show));
				sink.append("protection all fields ").append(buffer, static_cast<size_t>(len));
			}

			if (it.abilities->speed) {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d", (it.abilities->speed >> 1));
				sink.append("speed ").append(buffer, static_cast<size_t>(len));
			}
		}

		if (!begin) {
			sink.append(1, ')');
		}
	} else if (it.armor != 0 || (item && item->getArmor() != 0) || it.showAttributes) {
		bool begin = true;

		int32_t armor = (item ? item->getArmor() : it.armor);
		if (armor != 0) {
			sink.append(" (Arm:").append(std::to_string(armor));
			begin = false;
		}

		if (it.abilities) {
			for (uint8_t i = SKILL_FIRST; i <= SKILL_LAST; i++) {
				if (!it.abilities->skills[i]) {
					continue;
				}

				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, " %+d", it.abilities->skills[i]);
				sink.append(getSkillName(i)).append(buffer, static_cast<size_t>(len));
			}

			if (it.abilities->stats[STAT_MAGICPOINTS]) {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d", it.abilities->stats[STAT_MAGICPOINTS]);
				sink.append("magic level ").append(buffer, static_cast<size_t>(len));
			}

			int16_t show = it.abilities->absorbPercent[0];
			if (show != 0) {
				for (size_t i = 1; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] != show) {
						show = 0;
						break;
					}
				}
			}

			if (!show) {
				bool tmp = true;
				for (size_t i = 0; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] == 0) {
						continue;
					}

					if (tmp) {
						tmp = false;

						if (begin) {
							begin = false;
							sink.append(" (");
						} else {
							sink.append(", ");
						}

						sink.append("protection ");
					} else {
						sink.append(", ");
					}

					int32_t len = sprintf(buffer, " %+d%%", static_cast<int32_t>(it.abilities->absorbPercent[i]));
					sink.append(getCombatName(indexToCombatType(i))).append(buffer, static_cast<size_t>(len));
				}
			} else {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(show));
				sink.append("protection all ").append(buffer, static_cast<size_t>(len));
			}

			show = it.abilities->fieldAbsorbPercent[0];
			if (show != 0) {
				for (size_t i = 1; i < COMBAT_COUNT; ++i) {
					if (it.abilities->absorbPercent[i] != show) {
						show = 0;
						break;
					}
				}
			}

			if (!show) {
				bool tmp = true;

				for (size_t i = 0; i < COMBAT_COUNT; ++i) {
					if (it.abilities->fieldAbsorbPercent[i] == 0) {
						continue;
					}

					if (tmp) {
						tmp = false;

						if (begin) {
							begin = false;
							sink.append(" (");
						} else {
							sink.append(", ");
						}

						sink.append("protection ");
					} else {
						sink.append(", ");
					}

					int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(it.abilities->fieldAbsorbPercent[i]));
					sink.append(getCombatName(indexToCombatType(i))).append(" field ").append(buffer, static_cast<size_t>(len));
				}
			} else {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d%%", static_cast<int32_t>(show));
				sink.append("protection all fields ").append(buffer, static_cast<size_t>(len));
			}

			if (it.abilities->speed) {
				if (begin) {
					begin = false;
					sink.append(" (");
				} else {
					sink.append(", ");
				}

				int32_t len = sprintf(buffer, "%+d", (it.abilities->speed >> 1));
				sink.append("speed ").append(buffer, static_cast<size_t>(len));
			}
		}

		if (!begin) {
			sink.append(1, ')');
		}
	} else if (it.isContainer() || (item && item->getContainer())) {
		uint32_t volume = 0;
		if (!item || !item->hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
			if (it.isContainer()) {
				volume = it.maxItems;
			} else {
				volume = item->getContainer()->capacity();
			}
		}

		if (volume != 0) {
			sink.append(" (Vol:").append(std::to_string(volume)).append(1, ')');
		}
	} else {
		bool found = true;

		if (it.abilities) {
			if (it.abilities->speed > 0) {
				int32_t len = sprintf(buffer, "%+d", (it.abilities->speed >> 1));
				sink.append(" (speed ").append(buffer, static_cast<size_t>(len)).append(1, ')');
			} else if (hasBitSet(CONDITION_DRUNK, it.abilities->conditionSuppressions)) {
				sink.append(" (hard drinking)");
			} else if (it.abilities->invisible) {
				sink.append(" (invisibility)");
			} else if (it.abilities->regeneration) {
				sink.append(" (faster regeneration)");
			} else if (it.abilities->manaShield) {
				sink.append(" (mana shield)");
			} else {
				found = false;
			}
		} else {
			found = false;
		}

		if (!found) {
			if (it.isKey()) {
				int32_t len = sprintf(buffer, "%04d", (item ? static_cast<int32_t>(item->getActionId()) : 0));
				sink.append(" (Key:").append(buffer, static_cast<size_t>(len)).append(1, ')');
			} else if (it.isFluidContainer()) {
				if (subType > 0) {
					const std::string& itemName = items[subType].name;
					sink.append(" of ").append(!itemName.empty() ? itemName : "unknown");
				} else {
					sink.append(". It is empty");
				}
			} else if (it.isSplash()) {
				sink.append(" of ");

				if (subType > 0 && !items[subType].name.empty()) {
					sink.append(items[subType].name);
				} else {
					sink.append("unknown");
				}
			} else if (it.allowDistRead && (it.id < 7369 || it.id > 7371)) {
				sink.append(".\n");

				if (lookDistance <= 4) {
					if (item) {
						text = &item->getText();
						if (!text->empty()) {

							const std::string& writer = item->getWriter();
							if (!writer.empty()) {
								sink.append(writer).append(" wrote");
								time_t date = item->getDate();
								if (date != 0) {
									sink.append(" on ").append(formatDateShort(date));
								}
								sink.append(": ");
							} else {
								sink.append("You read: ");
							}

							sink.append(*text);
						} else {
							sink.append("Nothing is written on it");
						}
					} else {
						sink.append("Nothing is written on it");
					}
				} else {
					sink.append("You are too far away to read it");
				}
			} else if (it.levelDoor != 0 && item) {
				uint32_t actionId = static_cast<uint32_t>(item->getActionId());
				if (actionId >= it.levelDoor) {
					sink.append(" for level ").append(std::to_string(actionId - it.levelDoor));
				}
			}
		}
	}

	if (it.showCharges) {
		sink.append(" that has ").append(std::to_string(subType)).append(" charge").append(subType != 1 ? "s" : "").append(" left");
	}

	if (it.showDuration) {
		if (item && item->hasAttribute(ITEM_ATTRIBUTE_DURATION)) {
			uint32_t duration = item->getDuration() / 1000;
			sink.append(" that will expire in ");

			if (duration >= 86400) {
				uint32_t days = duration / 86400;
				uint32_t hours = (duration % 86400) / 3600;
				sink.append(std::to_string(days)).append(" day").append(days != 1 ? "s" : "");

				if (hours > 0) {
					sink.append(" and ").append(std::to_string(hours)).append(" hour").append(hours != 1 ? "s" : "");
				}
			} else if (duration >= 3600) {
				uint32_t hours = duration / 3600;
				uint32_t minutes = (duration % 3600) / 60;
				sink.append(std::to_string(hours)).append(" hour").append(hours != 1 ? "s" : "");

				if (minutes > 0) {
					sink.append(" and ").append(std::to_string(minutes)).append(" minute").append(minutes != 1 ? "s" : "");
				}
			} else if (duration >= 60) {
				uint32_t minutes = duration / 60;
				uint32_t seconds = duration % 60;
				sink.append(std::to_string(minutes)).append(" minute").append(minutes != 1 ? "s" : "");

				if (seconds > 0) {
					sink.append(" and ").append(std::to_string(seconds)).append(" second").append(seconds != 1 ? "s" : "");
				}
			} else {
				sink.append(std::to_string(duration)).append(" second").append(duration != 1 ? "s" : "");
			}
		} else {
			sink.append(" that is brand-new");
		}
	}

	if (!it.allowDistRead || (it.id >= 7369 && it.id <= 7371)) {
		sink.append(1, '.');
	} else {
		if (!text && item) {
			text = &item->getText();
		}

		if (!text || text->empty()) {
			sink.append(1, '.');
		}
	}

	if (it.wieldInfo != 0) {
		sink.append("\nIt can only be wielded properly by ");

		if (it.wieldInfo & WIELDINFO_PREMIUM) {
			sink.append("premium ");
		}

		if (!it.vocationString.empty()) {
			sink.append(it.vocationString);
		} else {
			sink.append("players");
		}

		if (it.wieldInfo & WIELDINFO_LEVEL) {
			sink.append(" of level ").append(std::to_string(it.minReqLevel)).append(" or higher");
		}

		if (it.wieldInfo & WIELDINFO_MAGLV) {
			if (it.wieldInfo & WIELDINFO_LEVEL) {
				sink.append(" and");
			} else {
				sink.append(" of");
			}

			sink.append(" magic level ").append(std::to_string(it.minReqMagicLevel)).append(" or higher");
		}

		sink.append(1, '.');
	}

	if (lookDistance <= 1) {
		if (item) {
			const uint32_t weight = item->getWeight();
			if (weight != 0 && it.pickupable) {
				sink.append(1, '\n').append(getWeightDescription(it, weight, item->getItemCount()));
			}
		} else if (it.weight != 0 && it.pickupable) {
			sink.append(1, '\n').append(getWeightDescription(it, it.weight));
		}
	}

	if (item) {
		const std::string& specialDescription = item->getSpecialDescription();
		if (!specialDescription.empty()) {
			sink.append(1, '\n').append(specialDescription);
		} else if (lookDistance <= 1 && !it.description.empty()) {
			sink.append(1, '\n').append(it.description);
		}
	} else if (lookDistance <= 1 && !it.description.empty()) {
		sink.append(1, '\n').append(it.description);
	}

	if (it.allowDistRead && it.id >= 7369 && it.id <= 7371) {
		if (!text && item) {
			text = &item->getText();
		}

		if (text && !text->empty()) {
			sink.append(1, '\n').append(*text);
		}
	}
	return sink;
}

std::string Item::getDescription(int32_t lookDistance) const
{
	const ItemType& it = items[id];
	return getDescription(it, lookDistance, this);
}

std::string Item::getNameDescription(const ItemType& it, const Item* item /*= nullptr*/, int32_t subType /*= -1*/, bool addArticle /*= true*/)
{
	if (item) {
		subType = item->getSubType();
	}

	std::string str;
	str.reserve(32);

	const std::string& name = (item ? item->getName() : it.name);
	if (!name.empty()) {
		if (it.stackable && subType > 1) {
			if (it.showCount) {
				str.append(std::to_string(subType)).append(1, ' ');
			}

			str.append(item ? item->getPluralName() : it.getPluralName());
		} else {
			if (addArticle) {
				const std::string& article = (item ? item->getArticle() : it.article);
				if (!article.empty()) {
					str.append(article).append(1, ' ');
				}
			}

			str.append(name);
		}
	} else {
		str.append("an item of type ").append(std::to_string(it.id));
	}
	return str;
}

std::string Item::getNameDescription() const
{
	const ItemType& it = items[id];
	return getNameDescription(it, this);
}

std::string Item::getWeightDescription(const ItemType& it, uint32_t weight, uint32_t count /*= 1*/)
{
	std::string str;
	str.reserve(20);
	if (it.stackable && count > 1 && it.showCount != 0) {
		str.append("They weigh ");
	} else {
		str.append("It weighs ");
	}

	if (weight < 10) {
		str.append("0.0").append(std::to_string(weight));
	} else if (weight < 100) {
		str.append("0.").append(std::to_string(weight));
	} else {
		std::string weightString = std::to_string(weight);
		weightString.insert(weightString.end() - 2, '.');
		str.append(weightString);
	}

	return str.append(" oz.");
}

std::string Item::getWeightDescription(uint32_t weight) const
{
	const ItemType& it = Item::items[id];
	return getWeightDescription(it, weight, getItemCount());
}

std::string Item::getWeightDescription() const
{
	uint32_t weight = getWeight();
	if (weight == 0) {
		return std::string();
	}
	return getWeightDescription(weight);
}

void Item::setUniqueId(uint16_t n)
{
	if (hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
		return;
	}

	if (g_game().addUniqueItem(n, this)) {
		getAttributes()->setUniqueId(n);
	}
}

bool Item::canDecay() const
{
	if (isRemoved()) {
		return false;
	}

	const ItemType& it = Item::items[id];
	if (it.decayTo < 0 || it.decayTime == 0) {
		return false;
	}

	if (hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
		return false;
	}

	return true;
}

uint32_t Item::getWorth() const
{
	switch (id) {
		case ITEM_GOLD_COIN:
			return count;

		case ITEM_PLATINUM_COIN:
			return count * 100;

		case ITEM_CRYSTAL_COIN:
			return count * 10000;

		default:
			return 0;
	}
}

LightInfo Item::getLightInfo() const
{
	const ItemType& it = items[id];
	return {it.lightLevel, it.lightColor};
}

std::string ItemAttributes::emptyString;
int64_t ItemAttributes::emptyInt;
double ItemAttributes::emptyDouble;
bool ItemAttributes::emptyBool;

const std::string& ItemAttributes::getStrAttr(itemAttrTypes type) const
{
	if (!isStrAttrType(type)) {
		return emptyString;
	}

	const Attribute* attr = getExistingAttr(type);
	if (!attr) {
		return emptyString;
	}
	return *attr->value.string;
}

void ItemAttributes::setStrAttr(itemAttrTypes type, const std::string& value)
{
	if (!isStrAttrType(type)) {
		return;
	}

	if (value.empty()) {
		return;
	}

	Attribute& attr = getAttr(type);
	delete attr.value.string;
	attr.value.string = new std::string(value);
}

void ItemAttributes::removeAttribute(itemAttrTypes type)
{
	if (!hasAttribute(type)) {
		return;
	}

	for (auto it = attributes.begin(), end = attributes.end(); it != end; ++it) {
		if ((*it).type == type) {
			(*it) = std::move(attributes.back());
			attributes.pop_back();
			break;
		}
	}
	attributeBits &= ~type;
}

int64_t ItemAttributes::getIntAttr(itemAttrTypes type) const
{
	if (!isIntAttrType(type)) {
		return 0;
	}

	const Attribute* attr = getExistingAttr(type);
	if (!attr) {
		return 0;
	}
	return attr->value.integer;
}

void ItemAttributes::setIntAttr(itemAttrTypes type, int64_t value)
{
	if (!isIntAttrType(type)) {
		return;
	}

	getAttr(type).value.integer = value;
}

void ItemAttributes::increaseIntAttr(itemAttrTypes type, int64_t value)
{
	if (!isIntAttrType(type)) {
		return;
	}

	getAttr(type).value.integer += value;
}

const ItemAttributes::Attribute* ItemAttributes::getExistingAttr(itemAttrTypes type) const
{
	if (hasAttribute(type)) {
		for (const Attribute& attribute : attributes) {
			if (attribute.type == type) {
				return &attribute;
			}
		}
	}
	return nullptr;
}

ItemAttributes::Attribute& ItemAttributes::getAttr(itemAttrTypes type)
{
	if (hasAttribute(type)) {
		for (Attribute& attribute : attributes) {
			if (attribute.type == type) {
				return attribute;
			}
		}
	}

	attributeBits |= type;
	attributes.emplace_back(type);
	return attributes.back();
}

void Item::startDecaying()
{
	g_game().startDecay(this);
}

void Item::stopDecaying()
{
	g_game().stopDecay(this);
}

bool Item::hasMarketAttributes() const
{
	if (!attributes || attributes->attributeBits == 0) {
		return true;
	}

	for (const auto& attr : attributes->getList()) {
		if (attr.type == ITEM_ATTRIBUTE_CHARGES) {
			uint16_t charges = static_cast<uint16_t>(attr.value.integer);
			if (charges != items[id].charges) {
				return false;
			}
		} else if (attr.type == ITEM_ATTRIBUTE_DURATION) {
			uint32_t duration = static_cast<uint32_t>(attr.value.integer);
			if (duration != getDefaultDuration()) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}
