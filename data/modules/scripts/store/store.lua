GameStore = {}
dofile('data/modules/scripts/store/cfg.lua')

GameStore.getCategoryByName = function(name)
	for i = 1, #GameStore.Categories do
		local category = GameStore.Categories[i]
		if(category.name:lower() == name) then
			return category
		end
	end
	return nil
end

GameStore.getOffersByName = function(name)
	local offers = {}
	for i = 1, #GameStore.Categories do
		local category = GameStore.Categories[i]
		if(category.offers) then
			for j = 1, #category.offers do
				local offer = category.offers[j]
				if(offer.name:lower():find(name)) then
					if(#offers >= GameStore.MaxSearchResults) then
						return offers, true
					end

					offers[#offers + 1] = offer
				end
			end
		end
	end
	return offers, false
end

GameStore.getCategoryById = function(id)
	for i = 1, #GameStore.Categories do
		local category = GameStore.Categories[i]
		if(category.offers) then
			for j = 1, #category.offers do
				local offer = category.offers[j]
				local idType = type(offer.id)
				if(idType == "number") then
					if(offer.id == id) then
						return category
					end
				elseif(idType == "table") then
					for _, offerId in pairs(offer.id) do
						if(offerId == id) then
							return category
						end
					end
				end
			end
		end
	end
	return nil
end

GameStore.getOfferById = function(id)
	for i = 1, #GameStore.Categories do
		local category = GameStore.Categories[i]
		if(category.offers) then
			for j = 1, #category.offers do
				local offer = category.offers[j]
				local idType = type(offer.id)
				if(idType == "number") then
					if(offer.id == id) then
						return offer
					end
				elseif(idType == "table") then
					for _, offerId in pairs(offer.id) do
						if(offerId == id) then
							return offer
						end
					end
				end
			end
		end
	end
	return nil
end

GameStore.insertHistory = function(accountId, mode, description, amount, coinType)
	return db.asyncQuery(string.format("INSERT INTO `store_history`(`account_id`, `mode`, `description`, `coin_amount`, `time`, `coin_type`) VALUES (%u, %u, %s, %d, %u, %u)", accountId, mode, db.escapeString(description), amount, os.time(), coinType))
end

GameStore.sendStoreError = function(player, errorType, message)
	local omsg = NetworkMessage()
	omsg:addByte(0xE0)
	omsg:addByte(errorType)
	omsg:addString(message)
	omsg:sendToPlayer(player)
	omsg:delete()
end

function addOffer(player, offer, omsg)
	local name = ""
	if(offer.type == GameStore.OfferTypes.OFFER_TYPE_ITEM and offer.count) then
		name = offer.count .. "x "
	end

	if(offer.type == GameStore.OfferTypes.OFFER_TYPE_STACKABLE and offer.count) then
		name = (offer.number and offer.number or 1) .. "x "
	end

	name = name .. (offer.name or "Something Special")

	local newPrice = nil
	if(offer.state == GameStore.States.STATE_SALE) then
		local daySub = offer.validUntil - os.date("*t").day
		if(daySub < 0) then
			newPrice = offer.basePrice
		end
	end

	local offerPrice = newPrice or offer.price or 0xFFFF
	local disabled, disabledReason = false, ""
	if(offer.disabled or offer.type == GameStore.OfferTypes.OFFER_TYPE_NONE) then
		disabled = true
	end

	if(offer.type ~= GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_EXPBOOST and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_PREYSLOT and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_PREYBONUS and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_TEMPLE and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE and
	offer.type ~= GameStore.OfferTypes.OFFER_TYPE_POUCH and
	not offer.id) then
		disabled = true
	end

	if(disabled and offer.disabledReason) then
		-- dynamic disable
		disabledReason = offer.disabledReason
	end

	if(not disabled) then
		if(offer.type == GameStore.OfferTypes.OFFER_TYPE_POUCH) then
			local pouch = player:getItemById(23721, true)
			if(pouch) then
				disabled = true
				disabledReason = "You already have Gold Pouch."
			end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_BLESSINGS) then
			disabled = true
			disabledReason = "This category ain't done yet."
			--if(player:hasBlessing(offer.id) and offer.id < 9) then
			--	disabled = true
			--	disabledReason = "You already have this Bless."
			--end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_ALLBLESSINGS) then
			disabled = true
			disabledReason = "This category ain't done yet."
			--if(player:hasBlessing(1) and player:hasBlessing(2) and player:hasBlessing(3) and player:hasBlessing(4) and player:hasBlessing(5) and player:hasBlessing(6) and player:hasBlessing(7) and player:hasBlessing(8)) then
			--	disabled = true
			--	disabledReason = "You already have all Blessings."
			--end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON) then
			local outfitLookType
			if(player:getSex() == PLAYERSEX_MALE) then
				outfitLookType = offer.sexId.male
			else
				outfitLookType = offer.sexId.female
			end

			if(outfitLookType) then
				if(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT and player:hasOutfit(outfitLookType)) then
					disabled = true
					disabledReason = "You already have this outfit."
				elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON) then
					if(player:hasOutfit(outfitLookType)) then
						if(player:hasOutfit(outfitLookType, offer.addon)) then
							disabled = true
							disabledReason = "You already have this addon."
						end
					else
						disabled = true
						disabledReason = "You don't have the outfit, you can't buy the addon."
					end
				end
			else
				disabled = true
				disabledReason = "The offer is fake."
			end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_MOUNT) then
			local hasMount = player:hasMount(offer.id)
			if(hasMount) then
				disabled = true
				disabledReason = "You already have this mount."
			end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PROMOTION) then
			disabled = true
			disabledReason = "This category ain't done yet."
			--if(not GameStore.canAddPromotionToPlayer(playerId, offer.id).ability) then
				--disabled = true
				--disabledReason = "You can't get this promotion"
			--end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PREYSLOT or offer.type == GameStore.OfferTypes.OFFER_TYPE_PREYBONUS) then
			disabled = true
			disabledReason = "This category ain't done yet."
			--local unlockedColumns = player:getStorageValue(STORE_SLOT_STORAGE)
			--if(unlockedColumns == 1) then
				--disabled = true
				--disabledReason = "You already have 3 slots released."
			--end
		elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_EXPBOOST or offer.type == GameStore.OfferTypes.OFFER_TYPE_HOUSE) then
			disabled = true
			disabledReason = "This category ain't done yet."
			--if(player:getStorageValue(51052) == 6 and (os.time() - player:getStorageValue(51053)) < 86400) then
				--disabled = true
				--disabledReason = "You can't buy XP Boost for today."
			--end
			--if(player:getExpBoostStamina() > 0) then
				--disabled = true
				--disabledReason = "You can't buy XP Boost while you still have one active."
			--end
		end
	end

	if(CLIENT_VERSION >= 1180) then
		omsg:addString(name)
		omsg:addByte(1)
		omsg:addU32(offer.id and offer.id or 0xFFFF)
		omsg:addU16(1)
		omsg:addU32(offerPrice)
		omsg:addByte(offer.coin and offer.coin or GameStore.CoinTypes.TIBIA_COIN)

		omsg:addByte(disabled and 0x01 or 0x00)
		if(disabled) then
			omsg:addByte(0x01)
			omsg:addString(disabledReason)
		end

		if(offer.state) then
			if(offer.state == GameStore.States.STATE_SALE) then
				local daySub = offer.validUntil - os.date("*t").day
				if(daySub >= 0) then
					omsg:addByte(offer.state)
					omsg:addU32(os.time() + daySub * 86400)
					omsg:addU32(offer.basePrice)
					--haveSaleOffer = 1
				else
					omsg:addByte(GameStore.States.STATE_NONE)
				end
			else
				omsg:addByte(offer.state)
			end
		else
			omsg:addByte(GameStore.States.STATE_NONE)
		end

		if(offer.mount) then
			omsg:addByte(GameStore.ShowTypes.SHOW_MOUNT)
			omsg:addU16(offer.mount)
		elseif(offer.item) then
			omsg:addByte(GameStore.ShowTypes.SHOW_ITEM)
			omsg:addItemId(offer.item)
		elseif(offer.sexId) then
			local outfit = player:getOutfit()
			omsg:addByte(GameStore.ShowTypes.SHOW_OUTFITMF)
			omsg:addByte((player:getSex() == PLAYERSEX_FEMALE and 2 or 1))
			omsg:addU16(offer.sexId.male)
			omsg:addU16(offer.sexId.female)
			omsg:addByte(outfit.lookHead)
			omsg:addByte(outfit.lookBody)
			omsg:addByte(outfit.lookLegs)
			omsg:addByte(outfit.lookFeet)
		elseif(offer.outfit) then
			local outfit = player:getOutfit()
			omsg:addByte(GameStore.ShowTypes.SHOW_OUTFIT)
			omsg:addU16(offer.outfit)
			omsg:addByte(outfit.lookHead)
			omsg:addByte(outfit.lookBody)
			omsg:addByte(outfit.lookLegs)
			omsg:addByte(outfit.lookFeet)
		else
			omsg:addByte(GameStore.ShowTypes.SHOW_ICON)
			omsg:addString(offer.icons[1])
		end

		if(CLIENT_VERSION >= 1212) then
			omsg:addByte(0)
		end

		omsg:addString(offer.filter)
		omsg:addU16(0x01)
		omsg:addU32(1488653090)
		omsg:addByte(0)
		if(offer.package) then
			omsg:addU16(#offer.package)
			for j = 1, #offer.package do
				local package = offer.package[j]
				omsg:addString(package.name)
				if(package.mount) then
					omsg:addByte(GameStore.ShowTypes.SHOW_MOUNT)
					omsg:addU16(package.mount)
				elseif(package.item) then
					omsg:addByte(GameStore.ShowTypes.SHOW_ITEM)
					omsg:addItemId(package.item)
				elseif(package.sexId) then
					local outfit = player:getOutfit()
					omsg:addByte(GameStore.ShowTypes.SHOW_OUTFITMF)
					omsg:addByte((player:getSex() == PLAYERSEX_FEMALE and 2 or 1))
					omsg:addU16(package.sexId.male)
					omsg:addU16(package.sexId.female)
					omsg:addByte(outfit.lookHead)
					omsg:addByte(outfit.lookBody)
					omsg:addByte(outfit.lookLegs)
					omsg:addByte(outfit.lookFeet)
				elseif(package.outfit) then
					local outfit = player:getOutfit()
					omsg:addByte(GameStore.ShowTypes.SHOW_OUTFIT)
					omsg:addU16(package.outfit)
					omsg:addByte(outfit.lookHead)
					omsg:addByte(outfit.lookBody)
					omsg:addByte(outfit.lookLegs)
					omsg:addByte(outfit.lookFeet)
				else
					omsg:addByte(GameStore.ShowTypes.SHOW_ICON)
					omsg:addString(package.icons[1])
				end
			end
		else
			omsg:addU16(0)
		end
	else
		omsg:addU32(offer.id and offer.id or 0xFFFF)
		omsg:addString(name)
		omsg:addString(offer.description)
		omsg:addU32(offerPrice)
		if(offer.state) then
			if(offer.state == GameStore.States.STATE_SALE and CLIENT_VERSION >= 1097) then
				local daySub = offer.validUntil - os.date("*t").day
				if(daySub >= 0) then
					omsg:addByte(offer.state)
					omsg:addU32(os.time() + daySub * 86400)
					omsg:addU32(offer.basePrice)
					--haveSaleOffer = 1
				else
					omsg:addByte(GameStore.States.STATE_NONE)
				end
			else
				omsg:addByte(offer.state)
			end
		else
			omsg:addByte(GameStore.States.STATE_NONE)
		end

		if(player:isUsingOtClient()) then
			if(disabled) then
				omsg:addByte(GameStore.OfferTypes.OFFER_TYPE_NONE)
			else
				omsg:addByte(offer.type)
			end
		else
			omsg:addByte(disabled and 0x01 or 0x00)
		end

		if(disabled and CLIENT_VERSION >= 1093) then
			omsg:addString(disabledReason)
		end

		omsg:addByte(#offer.icons)
		for j = 1, #offer.icons do
			omsg:addString(offer.icons[j])
		end

		omsg:addU16(0)
	end
end

function sendHomeStore(player)
	local omsg = NetworkMessage()
	omsg:addByte(0xFC)
	omsg:addString("Home")
	omsg:addU32(0)
	omsg:addByte(0)
	omsg:addByte(0)
	if(CLIENT_VERSION >= 1185) then
		omsg:addString("")
	end

	if(GameStore.Home.newOffers) then
		omsg:addU16(#GameStore.Home.newOffers)
		for i = 1, #GameStore.Home.newOffers do
			addOffer(player, GameStore.Home.newOffers[i], omsg)
		end
	else
		omsg:addU16(0x00)
	end

	if(GameStore.Home.features) then
		omsg:addByte(#GameStore.Home.features)
		for i = 1, #GameStore.Home.features do
			local feature = GameStore.Home.features[i]
			omsg:addString(feature.icon)
			if(feature.categoryName) then
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_CATEGORY_NAME)
				omsg:addString(feature.categoryName)
				omsg:addString(feature.categoryFilter)
			elseif(feature.categoryType) then
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_CATEGORY_TYPE)
				omsg:addByte(feature.categoryType)
			elseif(feature.offerName) then
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_OFFER_NAME)
				omsg:addString(feature.offerName)
			elseif(feature.offerType) then
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_OFFER_TYPE)
				omsg:addByte(feature.offerType)
			elseif(feature.offerId) then
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_OFFER_ID)
				omsg:addU32(feature.offerId)
			else
				omsg:addByte(GameStore.FeatureTypes.FEATURE_TYPE_NONE)
			end

			omsg:addByte(0)
			omsg:addByte(0)
		end
	else
		omsg:addByte(0)
	end

	omsg:addByte(GameStore.Home.featureScrollingTime)
	omsg:sendToPlayer(player)
	omsg:delete()
end

function sendSearchStore(player, offers, overflow)
	local omsg = NetworkMessage()
	omsg:addByte(0xFC)
	omsg:addString("Search")
	omsg:addU32(0)
	omsg:addByte(0)
	omsg:addByte(0)
	if(CLIENT_VERSION >= 1185) then
		omsg:addString("")
	end

	omsg:addU16(#offers)
	for i = 1, #offers do
		addOffer(player, offers[i], omsg)
	end

	omsg:addByte(overflow and 0x01 or 0x00)
	omsg:sendToPlayer(player)
	omsg:delete()
end

function sendStoreOffers(player, category, selection, sortType)
	local omsg = NetworkMessage()
	omsg:addByte(0xFC)
	omsg:addString(category.name)
	if(CLIENT_VERSION >= 1180) then
		omsg:addU32(selection and selection or 0)
		omsg:addByte(sortType and sortType or SORT_MOSTPOPULAR)
		if(category.filters) then
			omsg:addByte(#category.filters)
			for i = 1, #category.offers do
				omsg:addString(category.offers[i])
			end
			if(CLIENT_VERSION >= 1185) then
				omsg:addString(category.filter)
			end
		else
			omsg:addByte(0)
			if(CLIENT_VERSION >= 1185) then
				omsg:addString("")
			end
		end
	end

	omsg:addU16(category.offers and #category.offers or 0x00)
	if(category.offers) then
		for i = 1, #category.offers do
			addOffer(player, category.offers[i], omsg)
		end
	end

	omsg:sendToPlayer(player)
	omsg:delete()
end

function parseGetDescription(player, msg)
	local offerId = msg:getU32()
	local offer = GameStore.getOfferById(offerId)
	if(offer) then
		local omsg = NetworkMessage()
		omsg:addByte(0xEA)
		omsg:addU32(offerId)
		omsg:addString(offer.description or "No description to be displayed")
		omsg:sendToPlayer(player)
		omsg:delete()
	end
end

function playerTransferCoins(resultId, playerId, amount)
	local player = Player(playerId)
	if(not player) then
		return
	end

	if(resultId == false) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "That player does not exist.")
		return
	end

	local balance = result.getDataInt(resultId, "coins")
	if(balance < amount) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You don't have this amount of coins.")
		return
	end

	local accountId = result.getDataInt(resultId, "account_id")
	if(accountId == player:getAccountId()) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You can't transfer coins to a character in the same account.")
		return
	end

	local receiver = result.getDataString(resultId, "name")
	balance = balance - amount
	db.asyncQuery("UPDATE `accounts` SET `coins` = `coins` + " .. amount .. " WHERE `id` = " .. accountId)
	db.asyncQuery("UPDATE `accounts` SET `coins` = " .. balance .. " WHERE `id` = " .. player:getAccountId())
	GameStore.insertHistory(accountId, GameStore.HistoryTypes.HISTORY_TYPE_NONE, player:getName() .. " transfered you this amount.", amount, GameStore.CoinTypes.TRANSFERABLE_TIBIA_COIN)
	GameStore.insertHistory(player:getAccountId(), GameStore.HistoryTypes.HISTORY_TYPE_NONE, "You transfered this amount to " .. receiver, -1 * amount, GameStore.CoinTypes.TRANSFERABLE_TIBIA_COIN)

	local packages = configManager.getNumber(configKeys.STORE_COIN_PACKAGES)
	local omsg = NetworkMessage()
	omsg:addByte(0xFE)
	omsg:addByte(0x01)
	omsg:addString("You have transfered " .. amount .. " coins to " .. receiver .. " successfully")
	if(CLIENT_VERSION >= 1215) then
		omsg:addByte(0xDF)
		omsg:addByte(1)
		omsg:addU32(balance)
		omsg:addU32(math.floor(balance / packages) * packages)
		omsg:addU32(0) -- tournament coins
	else
		omsg:addU32(balance)
		omsg:addU32(math.floor(balance / packages) * packages)
	end
	omsg:sendToPlayer(player)
	omsg:delete()
end

function parseTransferCoins(player, msg)
	local packages = configManager.getNumber(configKeys.STORE_COIN_PACKAGES)
	local receiver = msg:getString():lower()
	local amount = math.floor(msg:getU32() / packages) * packages
	if(amount < packages) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You can't transfer this amount of coins.")
		return
	end

	if(receiver == player:getName():lower()) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You can't transfer coins to yourself.")
		return
	end

	db.asyncStoreQuery("SELECT `account_id`, `name`, (SELECT `coins` FROM `accounts` WHERE `id` = " .. player:getAccountId() .. ") as `coins` FROM `players` WHERE `name` = " .. db.escapeString(receiver), playerTransferCoins, player:getId(), amount)
end

function playerUpdateCoins(resultId, playerId)
	local player = Player(playerId)
	if(not player) then
		return
	end

	local balance = 0
	if(resultId ~= false) then
		balance = result.getDataInt(resultId, "coins")
	end

	local packages = configManager.getNumber(configKeys.STORE_COIN_PACKAGES)
	local omsg = NetworkMessage()
	omsg:addByte(0xDF)
	omsg:addByte(1)
	omsg:addU32(balance)
	omsg:addU32(math.floor(balance / packages) * packages)
	if(CLIENT_VERSION >= 1215) then
		omsg:addU32(0) -- tournament coins
	end
	omsg:sendToPlayer(player)
	omsg:delete()
end

function parseOpenStore(player, msg)
	local omsg = NetworkMessage()
	omsg:addByte(0xFB)
	if(CLIENT_VERSION < 1180) then
		omsg:addByte(0)
	end

	omsg:addU16(#GameStore.Categories)
	for i = 1, #GameStore.Categories do
		local category = GameStore.Categories[i]
		omsg:addString(category.name)
		if(CLIENT_VERSION < 1180) then
			omsg:addString(category.description)
		end

		if(CLIENT_VERSION >= 1093) then
			omsg:addByte(category.state or GameStore.States.STATE_NONE)
		end

		omsg:addByte(#category.icons)
		for j = 1, #category.icons do
			omsg:addString(category.icons[j])
		end

		omsg:addString(category.parentCategory)
	end

	omsg:sendToPlayer(player)
	omsg:delete()
	db.asyncStoreQuery("SELECT `coins` FROM `accounts` WHERE `id` = " .. player:getAccountId(), playerUpdateCoins, player:getId())
end

function parseRequestStore(player, msg)
	if(CLIENT_VERSION >= 1180) then
		local serviceType = msg:getByte()
		if(serviceType == GameStore.ServiceTypes.SERVICE_HOME) then
			sendHomeStore(player)
		elseif(serviceType == GameStore.ServiceTypes.SERVICE_CATEGORY_NAME) then
			local categoryName = msg:getString()
			--local categoryFilter = msg:getString()

			local category = GameStore.getCategoryByName(categoryName:lower())
			if(category) then
				sendStoreOffers(player, category)
			end
		elseif(serviceType == GameStore.ServiceTypes.SERVICE_OFFER_ID) then
			local offerId = msg:getU32()

			local category = GameStore.getCategoryById(offerId)
			if(category) then
				sendStoreOffers(player, category, offerId)
			end
		elseif(serviceType == GameStore.ServiceTypes.SERVICE_OFFER_NAME) then
			local offerName = msg:getString()
			if(offerName:len() < 3) then
				return
			end

			local offers, overflow = GameStore.getOffersByName(offerName:lower())
			sendSearchStore(player, offers, overflow)
		end
		return
	end

	local serviceType = GameStore.LegacyServiceTypes.SERVICE_STANDERD
	if(CLIENT_VERSION >= 1092) then
		serviceType = msg:getByte()
	end

	local categoryName = msg:getString()
	local category = GameStore.getCategoryByName(categoryName:lower())
	if(category) then
		sendStoreOffers(player, category)
	end
end

function playerBuyStore(resultId, playerId, offerId, productType, extraInfo)
	local player = Player(playerId)
	if(not player) then
		return
	end

	if(resultId == false) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "Failed to retrieve coins.")
		return
	end

	local offer = GameStore.getOfferById(offerId)
	if(offer.coin == GameStore.CoinTypes.TOURNAMENT_COIN) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "We don't support tournament coins yet.")
		return
	end

	local balance = result.getDataInt(resultId, "coins")
	if(balance < offer.price) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "You don't have enough coins. Your purchase has been cancelled.")
		return
	end

	if(offer.disabled or offer.type == GameStore.OfferTypes.OFFER_TYPE_NONE) then
		if(offer.disabledReason) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: " .. offer.disabledReason)
		else
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is unavailable.")
		end
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE) then
		if(productType ~= GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE) then
			local omsg = NetworkMessage()
			omsg:addByte(0xE1)
			omsg:addU32(offerId)
			omsg:addByte(GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE)
			omsg:sendToPlayer(player)
			omsg:delete()
			return
		end
	elseif(offer.type ~= GameStore.OfferTypes.OFFER_TYPE_EXPBOOST and
		offer.type ~= GameStore.OfferTypes.OFFER_TYPE_PREYSLOT and
		offer.type ~= GameStore.OfferTypes.OFFER_TYPE_PREYBONUS and
		offer.type ~= GameStore.OfferTypes.OFFER_TYPE_TEMPLE and
		offer.type ~= GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE and
		offer.type ~= GameStore.OfferTypes.OFFER_TYPE_POUCH and
		not offer.id) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is unavailable.")
		return
	end

	if(offer.type == GameStore.OfferTypes.OFFER_TYPE_ITEM) then
		local itemId = offer.id
		if(offer.item) then
			itemId = offer.item
		end

		if(player:getFreeCapacity() < ItemType(itemId):getWeight(offer.count)) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free capacity to hold this item.")
			return
		end

		local inbox = player:getSlotItem(11)
		if(inbox) then -- check if we have store inbox
			inbox:addItem(itemId, offer.count, INDEX_WHEREEVER, FLAG_NOLIMIT)
		else
			if(not player:addItem(itemId, offer.count, false)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free slots in your inventory.")
				return
			end
		end
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_STACKABLE) then
		local itemId = offer.id
		if(offer.item) then
			itemId = offer.item
		end

		local stackable = ItemType(itemId):isStackable()
		if(stackable) then
			if(player:getFreeCapacity() < ItemType(itemId):getWeight(offer.count)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free capacity to hold this item.")
				return
			end
		else
			if(player:getFreeCapacity() < ItemType(itemId):getWeight(1)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free capacity to hold this item.")
				return
			end
		end

		local inbox = player:getSlotItem(11)
		if(inbox) then -- check if we have store inbox
			if(stackable) then
				local pendingCount = offer.count
				while(pendingCount > 0) do
					local pack
					if(pendingCount > 100) then
						pack = 100
					else
						pack = pendingCount
					end
					inbox:addItem(itemId, pack, INDEX_WHEREEVER, FLAG_NOLIMIT)
					pendingCount = pendingCount - pack
				end
			else
				local item = inbox:addItem(itemId, 1, INDEX_WHEREEVER, FLAG_NOLIMIT)
				item:setAttribute(ITEM_ATTRIBUTE_CHARGES, offer.count)
			end
		else
			if(stackable) then
				if(not player:addItem(itemId, offer.count, false)) then
					GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free slots in your inventory.")
					return
				end
			else
				local item = player:addItem(itemId, 1, false)
				if(not item) then
					GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free slots in your inventory.")
					return
				end

				item:setAttribute(ITEM_ATTRIBUTE_CHARGES, offer.count)
			end
		end
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON) then
		local outfitLookType
		if(player:getSex() == PLAYERSEX_MALE) then
			outfitLookType = offer.sexId.male
		else
			outfitLookType = offer.sexId.female
		end

		if(outfitLookType) then
			if(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT and player:hasOutfit(outfitLookType)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You already have this outfit.")
				return
			elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON) then
				if(player:hasOutfit(outfitLookType)) then
					if(player:hasOutfit(outfitLookType, offer.addon)) then
						GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You already have this addon.")
						return
					end
				else
					GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You don't have the outfit, you can't buy the addon.")
					return
				end
			end

			if(not player:addOutfitAddon(outfitLookType, offer.addon) or not player:hasOutfit(outfitLookType, offer.addon)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "There has been an issue with your outfit purchase. Your purchase has been cancelled.")
				return
			end
		else
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: The offer is fake.")
			return
		end
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_MOUNT) then
		local hasMount = player:hasMount(offer.id)
		if(hasMount) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You already have this mount.")
			return
		end

		player:addMount(offer.id)
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE) then
		local tile = Tile(player:getPosition())
		if(not tile or not tile:hasFlag(TILESTATE_PROTECTIONZONE)) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "You can change name only in Protection Zone.")
			return
		end

		local newName = extraInfo:lower():gsub("(%l)(%w*)", function(a, b) return string.upper(a) .. b end)
		local resultId = db.storeQuery("SELECT `name` FROM `players` WHERE `name` = " .. db.escapeString(newName))
		if(resultId ~= false) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This name is already used, please try again!")
			return
		end

		if(newName:len() < 3 or newName:len() > NETWORKMESSAGE_PLAYERNAME_MAXLENGTH) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your character name must be between 3 - " .. NETWORKMESSAGE_PLAYERNAME_MAXLENGTH .. " characters long.")
			return
		end

		local matchAZ = newName:match("[^a-zA-Z ]")
		if(matchAZ) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your name may only contain a-z, A-Z and spaces.")
			return
		end

		local count = 0
		for m in newName:gmatch("%s+") do
			count = count + 1
		end

		if(count > 1) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your name can not contain more than 2 words.")
			return
		end

		if(MonsterType(newName)) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your username contains a restricted word.")
			return
		elseif(Npc(newName)) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your username contains a restricted word.")
			return
		else
			-- just copied from znote aac.
			local words = {"owner", "gamemaster", "hoster", "admin", "staff", "tibia", "account", "god", "anal", "ass", "fuck", "sex", "hitler", "pussy", "dick", "rape", "adm", "cm", "gm", "tutor", "counsellor"}
			local splits = newName:split(" ")
			for i = 1, #splits do
				local split = splits[i]
				if(isInArray(words, split:lower())) then
					GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Your username contains a restricted word.")
					return
				elseif(split:len() == 1) then
					GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Too short words in your name.")
					return
				end
			end
		end

		db.query("UPDATE `players` SET `name` = " .. db.escapeString(newName) .. " WHERE `id` = " .. player:getGuid())
		player:setName(newName)
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE) then
		local outfit = player:getOutfit()
		if(player:getSex() == PLAYERSEX_MALE) then
			player:setSex(PLAYERSEX_FEMALE)
			outfit.lookType = 136
		else
			player:setSex(PLAYERSEX_MALE)
			outfit.lookType = 128
		end
		player:setOutfit(outfit)
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PROMOTION) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_HOUSE) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_EXPBOOST) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PREYSLOT) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PREYBONUS) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_TEMPLE) then
		if(player:hasCondition(CONDITION_INFIGHT) or player:isPzLocked()) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You can't use temple teleport in fight!")
			return
		end

		player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
		player:teleportTo(player:getTown():getTemplePosition())
		player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You have been teleported to your hometown.")
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_BLESSINGS) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_PREMIUM) then
		player:addPremiumDays(offer.id - 3000)
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_POUCH) then
		local pouch = player:getItemById(26377, true)
		if(pouch) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: You already have Gold Pouch.")
			return
		end

		if(player:getFreeCapacity() < ItemType(26377):getWeight(1)) then
			GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free capacity to hold this item.")
			return
		end

		local inbox = player:getSlotItem(11)
		if(inbox) then -- check if we have store inbox
			inbox:addItem(26377, 1, INDEX_WHEREEVER, FLAG_NOLIMIT)
		else
			if(not player:addItem(26377, 1, false)) then
				GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "Please make sure you have free slot in your inventory.")
				return
			end
		end
	elseif(offer.type == GameStore.OfferTypes.OFFER_TYPE_ALLBLESSINGS) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is disabled: This category ain't done yet.")
		return
	else
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_PURCHASE, "This offer is unavailable.")
		return
	end

	balance = balance - offer.price
	db.asyncQuery("UPDATE `accounts` SET `coins` = " .. balance .. " WHERE `id` = " .. player:getAccountId())
	GameStore.insertHistory(player:getAccountId(), GameStore.HistoryTypes.HISTORY_TYPE_NONE, offer.name, -1 * offer.price, GameStore.CoinTypes.TIBIA_COIN)

	local packages = configManager.getNumber(configKeys.STORE_COIN_PACKAGES)
	local omsg = NetworkMessage()
	omsg:addByte(0xFE)
	omsg:addByte(0x00)
	omsg:addString("You have purchased " .. offer.name .. " for " .. offer.price .. " tibia coins.")
	if(CLIENT_VERSION >= 1215) then
		omsg:addByte(0xDF)
		omsg:addByte(1)
		omsg:addU32(balance)
		omsg:addU32(math.floor(balance / packages) * packages)
		omsg:addU32(0) -- tournament coins
	else
		omsg:addU32(balance)
		omsg:addU32(math.floor(balance / packages) * packages)
	end
	omsg:sendToPlayer(player)
	omsg:delete()
end

function parseBuyStore(player, msg)
	local offerId = msg:getU32()
	local productType = msg:getByte()

	local extraInfo = nil
	if(productType == GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE) then
		extraInfo = msg:getString()
	end

	db.asyncStoreQuery("SELECT `coins` FROM `accounts` WHERE `id` = " .. player:getAccountId(), playerBuyStore, player:getId(), offerId, productType, extraInfo)
end

function playerTransactionResult(resultId, playerId, page, entires)
	local player = Player(playerId)
	if(not player) then
		return
	end

	if(resultId == false) then
		GameStore.sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_HISTORY, "You don't have any entries yet.")
		return
	end

	local totalEntries = result.getDataInt(resultId, "total")
	local totalPages = math.ceil(totalEntries / entires)
	local sendEntries = 0

	local omsg = NetworkMessage()
	omsg:addByte(0xFD)
	if(CLIENT_VERSION <= 1096) then
		omsg:addU16(page)
		omsg:addByte((totalPages > page and 0x01 or 0x00))
	else
		omsg:addU32(page)
		omsg:addU32(totalPages)
	end

	local msgPosition = omsg:getMsgPosition()
	omsg:skipBytes(1)
	if(CLIENT_VERSION >= 1208) then
		repeat
			omsg:addU32(result.getDataInt(resultId, "id"))
			omsg:addU32(result.getDataInt(resultId, "time"))
			omsg:addByte(result.getDataInt(resultId, "mode"))
			omsg:addU32(result.getDataInt(resultId, "coin_amount"))
			omsg:addByte(result.getDataInt(resultId, "coin_type"))
			omsg:addString(result.getDataString(resultId, "description"))
			omsg:addByte(0x00)
			sendEntries = sendEntries + 1
		until not result.next(resultId)
		result.free(resultId)
	else
		repeat
			omsg:addU32(result.getDataInt(resultId, "time"))
			omsg:addByte(result.getDataInt(resultId, "mode"))
			omsg:addU32(result.getDataInt(resultId, "coin_amount"))
			if(CLIENT_VERSION >= 1200) then
				omsg:addByte(result.getDataInt(resultId, "coin_type"))
			end
			omsg:addString(result.getDataString(resultId, "description"))
			sendEntries = sendEntries + 1
		until not result.next(resultId)
		result.free(resultId)
	end

	omsg:setMsgPosition(msgPosition)
	omsg:addByte(sendEntries)
	omsg:sendToPlayer(player)
	omsg:delete()
end

function parseOpenTransactions(player, msg)
	local page = 0
	local entries = math.min(25, math.max(5, msg:getByte()))
	local offset = 0
	local accountId = player:getAccountId()
	local omsg = NetworkMessage()
	db.asyncStoreQuery("SELECT *, (select count(*) FROM `store_history` WHERE `account_id` = " .. accountId .. ") as `total` FROM `store_history` WHERE `account_id` = " .. accountId .. " ORDER BY `time` DESC LIMIT " .. offset .. ", " .. entries .. ";", playerTransactionResult, player:getId(), page, entries)
end

function parseRequestTransactions(player, msg)
	local page = nil
	if(CLIENT_VERSION <= 1096) then
		page = msg:getU16()
	else
		page = msg:getU32()
	end
	local entries = math.min(25, math.max(5, msg:getByte()))
	local offset = (page > 0 and page * entries or 0)
	local accountId = player:getAccountId()
	db.asyncStoreQuery("SELECT *, (select count(*) FROM `store_history` WHERE `account_id` = " .. accountId .. ") as `total` FROM `store_history` WHERE `account_id` = " .. accountId .. " ORDER BY `time` DESC LIMIT " .. offset .. ", " .. entries .. ";", playerTransactionResult, player:getId(), page, entries)
end

GameStore.switch = {
	[0xE8] = parseGetDescription,
	--[0xE9] = parseStoreEvent, -- We don't need this :)
	[0xEF] = parseTransferCoins,
	[0xFA] = parseOpenStore,
	[0xFB] = parseRequestStore,
	[0xFC] = parseBuyStore,
	[0xFD] = parseOpenTransactions,
	[0xFE] = parseRequestTransactions,
}

function onRecvbyte(player, msg, byte)
	local func = GameStore.switch[byte]
	if(func) then
		func(player, msg)
	end
end
