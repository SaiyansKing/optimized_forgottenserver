local doorIds = {}

for index, value in ipairs(QuestDoorTable) do
	if not table.contains(doorIds, value.openDoor) then
		table.insert(doorIds, value.openDoor)
	end

	if not table.contains(doorIds, value.closedDoor) then
		table.insert(doorIds, value.closedDoor)
	end
end

local questDoor = Action()

function questDoor.onUse(creature, item, fromPosition, target, toPosition, isHotkey)
	for index, value in ipairs(QuestDoorTable) do
		if value.closedDoor == item.itemid then
			if item.actionid > 0 and creature:getStorageValue(item.actionid) ~= -1 then
				item:transform(value.openDoor)
				creature:teleportTo(toPosition, true)
				return true
			else
				creature:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The door seems to be sealed against unwanted intruders.")
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
	questDoor:id(value)
end

questDoor:register()
