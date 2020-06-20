local doorIds = {}

for index, value in ipairs(LevelDoorTable) do
	if not table.contains(doorIds, value.openDoor) then
		table.insert(doorIds, value.openDoor)
	end

	if not table.contains(doorIds, value.closedDoor) then
		table.insert(doorIds, value.closedDoor)
	end
end

local levelDoor = Action()

function levelDoor.onUse(creature, item, fromPosition, target, toPosition, isHotkey)
	for index, value in ipairs(LevelDoorTable) do
		 if value.closedDoor == item.itemid then
			if item.actionid > 0 and creature:getLevel() >= item.actionid - 1000 then
				item:transform(value.openDoor)
				creature:teleportTo(toPosition, true)
				return true
			else
				creature:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end
		end
	end

	if Creature.isInsideDoor(creature, toPosition) then
		return true
	end
	return true
end

for index, value in ipairs(doorIds) do
	levelDoor:id(value)
end

levelDoor:register()
