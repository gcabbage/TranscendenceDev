//	SpaceObjectItemProperties.cpp
//
//	CSpaceObject class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_DESC_ID							CONSTLIT("descID")
#define FIELD_CAN_INSTALL						CONSTLIT("canInstall")
#define FIELD_CAN_REMOVE						CONSTLIT("canRemove")
#define FIELD_PRICE								CONSTLIT("price")
#define FIELD_UPGRADE_INSTALL_ONLY				CONSTLIT("upgradeInstallOnly")

#define PROPERTY_DAMAGED						CONSTLIT("damaged")
#define PROPERTY_ENABLED						CONSTLIT("enabled")
#define PROPERTY_HP								CONSTLIT("hp")
#define PROPERTY_INSTALL_DEVICE_PRICE			CONSTLIT("installDevicePrice")
#define PROPERTY_INSTALL_DEVICE_STATUS			CONSTLIT("installDeviceStatus")
#define PROPERTY_INSTALL_ITEM_STATUS			CONSTLIT("installItemStatus")
#define PROPERTY_REMOVE_DEVICE_PRICE			CONSTLIT("removeDevicePrice")
#define PROPERTY_REMOVE_ITEM_STATUS				CONSTLIT("removeItemStatus")

ICCItem *CSpaceObject::GetItemProperty (CCodeChainCtx &CCCtx, const CItem &Item, const CString &sName) const

//	GetItemProperty
//
//	Returns an item property

	{
	CCodeChain &CC = GetUniverse().GetCC();

	//	Return the property

	if (strEquals(sName, PROPERTY_INSTALL_DEVICE_PRICE))
		{
		int iPrice;
		if (!GetDeviceInstallPrice(Item, 0, &iPrice))
			return CC.CreateNil();

		return CC.CreateInteger(iPrice);
		}
	else if (strEquals(sName, PROPERTY_INSTALL_DEVICE_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canInstall: True or Nil
		//	price: Install price
		//	descID: Message ID for description of install attempt
		//	upgradeInstallOnly: True if we only install on upgrade

		CString sMessageID;
		int iPrice;
		DWORD dwPriceFlags;
		bool bCanInstall = GetDeviceInstallPrice(Item, 0, &iPrice, &sMessageID, &dwPriceFlags);

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(FIELD_CAN_INSTALL, (bCanInstall ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(FIELD_PRICE, (bCanInstall ? iPrice : -1));
		if (bCanInstall && (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else if (strEquals(sName, PROPERTY_INSTALL_ITEM_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canInstall: True or Nil
		//	price: Install price
		//	descID: Message ID for description of install attempt
		//	upgradeInstallOnly: True if we only install on upgrade

		CString sMessageID;
		int iPrice;
		bool bCanInstall;
		DWORD dwPriceFlags = 0;
		if (Item.IsArmor())
			bCanInstall = GetArmorInstallPrice(Item, 0, &iPrice, &sMessageID);
		else if (Item.IsDevice())
			bCanInstall = GetDeviceInstallPrice(Item, 0, &iPrice, &sMessageID, &dwPriceFlags);
		else
			bCanInstall = false;

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(FIELD_CAN_INSTALL, (bCanInstall ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(FIELD_PRICE, (bCanInstall ? iPrice : -1));
		if (bCanInstall 
				&& (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		//	NOTE: Message is valid even if we cannot install

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else if (strEquals(sName, PROPERTY_REMOVE_DEVICE_PRICE))
		{
		int iPrice;
		if (!GetDeviceRemovePrice(Item, 0, &iPrice))
			return CC.CreateNil();

		return CC.CreateInteger(iPrice);
		}
	else if (strEquals(sName, PROPERTY_REMOVE_ITEM_STATUS))
		{
		//	We return a structure with the following fields:
		//
		//	canRemove: True or Nil
		//	price: Remove price
		//	descID: Message ID for description of remove attempt
		//	upgradeInstallOnly: True if we only install/remove on upgrade

		CString sMessageID;
		int iPrice;
		bool bCanRemove;
		DWORD dwPriceFlags = 0;
		if (Item.IsDevice())
			bCanRemove = GetDeviceRemovePrice(Item, 0, &iPrice, &dwPriceFlags);
		else
			bCanRemove = false;

		//	Create the structure

		ICCItem *pResult = CC.CreateSymbolTable();
		pResult->SetAt(FIELD_CAN_REMOVE, (bCanRemove ? CC.CreateTrue() : CC.CreateNil()));
		pResult->SetIntegerAt(FIELD_PRICE, (bCanRemove ? iPrice : -1));
		if (bCanRemove 
				&& (dwPriceFlags & CTradingDesc::PRICE_UPGRADE_INSTALL_ONLY))
			pResult->SetAt(FIELD_UPGRADE_INSTALL_ONLY, CC.CreateTrue());

		if (!sMessageID.IsBlank())
			pResult->SetStringAt(FIELD_DESC_ID, sMessageID);

		return pResult;
		}
	else
		{
		//	Select the item (to make sure that it is part of the object)
		//
		//	LATER: We need a const-version of CItemListManupulator

		CItemListManipulator ItemList(const_cast<CItemList &>(GetItemList()));
		if (!ItemList.SetCursorAtItem(Item))
			return CC.CreateError(CONSTLIT("Item not found on object."));

		//	LATER: Eventually we will get rid of CItemCtx.

		CItemCtx Ctx(&Item, const_cast<CSpaceObject *>(this));
		return Item.GetItemProperty(CCCtx, Ctx, sName, false);
		}
	}

bool CSpaceObject::SetItemProperty (const CItem &Item, const CString &sName, ICCItem *pValue, int iCount, CItem *retItem, CString *retsError)

//	SetItemProperty
//
//	Sets the item property.
//
//	NOTE: pValue may be NULL.

	{
	//	Select the item to make sure it exists on this object.

	CItemListManipulator ItemList(GetItemList());
	if (!ItemList.SetCursorAtItem(Item))
		{
		if (retsError) *retsError = CONSTLIT("Item not found on object.");
		return false;
		}

	//	Notify any dock screens that we might modify an item

	IDockScreenUI::SModifyItemCtx ModifyCtx;
	OnModifyItemBegin(ModifyCtx, Item);

	//	We handle damage differently because we may need to remove enhancements,
	//	etc.

	if (strEquals(sName, PROPERTY_DAMAGED))
		{
		if (pValue && pValue->IsNil())
			RepairItem(ItemList);
		else
			DamageItem(ItemList);
		}

	//	Enabling/disabling needs special handling

	else if (strEquals(sName, PROPERTY_ENABLED))
		{
		CShip *pShip = AsShip();
		if (pShip == NULL)
			{
			if (retsError) *retsError = CONSTLIT("Property not supported.");
			return false;
			}

		if (!ItemList.GetItemAtCursor().IsInstalled())
			{
			if (retsError) *retsError = CONSTLIT("Device is not installed on object.");
			return false;
			}

		//	Parse the parameter

		bool bEnabled;
		bool bSilent;
		if (pValue == NULL)
			{
			bEnabled = true;
			bSilent = false;
			}
		else if (pValue->IsIdentifier())
			{
			if (strEquals(pValue->GetStringValue(), CONSTLIT("silentDisabled")))
				{
				bEnabled = false;
				bSilent = true;
				}
			else if (strEquals(pValue->GetStringValue(), CONSTLIT("silentEnabled")))
				{
				bEnabled = true;
				bSilent = true;
				}
			else if (strEquals(pValue->GetStringValue(), CONSTLIT("disabled")))
				{
				bEnabled = false;
				bSilent = false;
				}
			else
				{
				bEnabled = true;
				bSilent = false;
				}
			}
		else
			{
			bEnabled = !pValue->IsNil();
			bSilent = false;
			}

		//	Do it.

		pShip->EnableDevice(ItemList.GetItemAtCursor().GetInstalled(), bEnabled, bSilent);
		}

	//	Otherwise, just set the property, but pass enough context (this object)
	//	so that it can find the appropriate device.

	else
		{
		//	Set the data

		if (!ItemList.SetPropertyAtCursor(this, sName, pValue, iCount, retsError))
			return false;

		//	Update the object

		ItemEnhancementModified(ItemList);
		}

	//	Return the newly changed item. We do this before the notification 
	//	because the notification might change the underlying item list (because
	//	it sorts).

	if (retItem)
		*retItem = ItemList.GetItemAtCursor();

	//	Update the object

	OnModifyItemComplete(ModifyCtx, ItemList.GetItemAtCursor());

	return true;
	}
