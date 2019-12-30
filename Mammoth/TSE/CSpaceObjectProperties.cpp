//	CSpaceObjectProperties.cpp
//
//	CSpaceObject class
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define CATEGORY_BEAM							CONSTLIT("beam")
#define CATEGORY_EFFECT							CONSTLIT("effect")
#define CATEGORY_MARKER							CONSTLIT("marker")
#define CATEGORY_MISSILE						CONSTLIT("missile")
#define CATEGORY_MISSION						CONSTLIT("mission")
#define CATEGORY_SHIP							CONSTLIT("ship")
#define CATEGORY_STATION						CONSTLIT("station")

#define FIELD_ARMOR_INTEGRITY					CONSTLIT("armorIntegrity")
#define FIELD_HULL_INTEGRITY					CONSTLIT("hullIntegrity")
#define FIELD_OBJ_ID							CONSTLIT("objID")
#define FIELD_POS								CONSTLIT("pos")
#define FIELD_SHIELD_LEVEL						CONSTLIT("shieldLevel")
#define FIELD_STATUS							CONSTLIT("status")

#define PROPERTY_ASCENDED						CONSTLIT("ascended")
#define PROPERTY_CAN_ATTACK						CONSTLIT("canAttack")
#define PROPERTY_CAN_BE_ATTACKED				CONSTLIT("canBeAttacked")
#define PROPERTY_CATEGORY						CONSTLIT("category")
#define PROPERTY_COMMS_KEY						CONSTLIT("commsKey")
#define PROPERTY_CURRENCY						CONSTLIT("currency")
#define PROPERTY_CURRENCY_NAME					CONSTLIT("currencyName")
#define PROPERTY_CYBER_DEFENSE_LEVEL			CONSTLIT("cyberDefenseLevel")
#define PROPERTY_DAMAGE_DESC					CONSTLIT("damageDesc")
#define PROPERTY_DESTINY						CONSTLIT("destiny")
#define PROPERTY_DOCKING_PORTS					CONSTLIT("dockingPorts")
#define PROPERTY_EVENT_SUBSCRIBERS				CONSTLIT("eventSubscribers")
#define PROPERTY_HAS_DOCKING_PORTS				CONSTLIT("hasDockingPorts")
#define PROPERTY_ID								CONSTLIT("id")
#define PROPERTY_IDENTIFIED						CONSTLIT("identified")
#define PROPERTY_INSTALL_ARMOR_MAX_LEVEL		CONSTLIT("installArmorMaxLevel")
#define PROPERTY_INSTALL_DEVICE_MAX_LEVEL		CONSTLIT("installDeviceMaxLevel")
#define PROPERTY_INSTALL_DEVICE_UPGRADE_ONLY	CONSTLIT("installDeviceUpgradeOnly")
#define PROPERTY_KNOWN							CONSTLIT("known")
#define PROPERTY_LEVEL							CONSTLIT("level")
#define PROPERTY_MASS							CONSTLIT("mass")
#define PROPERTY_NAME_PATTERN					CONSTLIT("namePattern")
#define PROPERTY_PAINT_LAYER					CONSTLIT("paintLayer")
#define PROPERTY_PLAYER_MISSIONS_GIVEN			CONSTLIT("playerMissionsGiven")
#define PROPERTY_RADIOACTIVE					CONSTLIT("radioactive")
#define PROPERTY_REFUEL_MAX_LEVEL				CONSTLIT("refuelMaxLevel")
#define PROPERTY_REMOVE_DEVICE_MAX_LEVEL		CONSTLIT("removeDeviceMaxLevel")
#define PROPERTY_REPAIR_ARMOR_MAX_LEVEL			CONSTLIT("repairArmorMaxLevel")
#define PROPERTY_SCALE							CONSTLIT("scale")
#define PROPERTY_SHOW_AS_DESTINATION			CONSTLIT("showAsDestination")
#define PROPERTY_SIZE_PIXELS					CONSTLIT("sizePixels")
#define PROPERTY_SOVEREIGN						CONSTLIT("sovereign")
#define PROPERTY_STEALTH						CONSTLIT("stealth")
#define PROPERTY_SUSPENDED						CONSTLIT("suspended")
#define PROPERTY_TYPE							CONSTLIT("type")
#define PROPERTY_UNDER_ATTACK					CONSTLIT("underAttack")

#define SCALE_STAR								CONSTLIT("star")
#define SCALE_WORLD								CONSTLIT("world")
#define SCALE_STATION							CONSTLIT("station")
#define SCALE_SHIP								CONSTLIT("ship")
#define SCALE_FLOTSAM							CONSTLIT("flotsam")

ICCItemPtr CSpaceObject::GetProperty (CCodeChainCtx &CCX, const CString &sProperty) const

//	GetProperty
//
//	Returns the property.

	{
	if (ICCItemPtr pValue = OnFindProperty(CCX, sProperty))
		return pValue;
	else
		{
		ICCItemPtr pResult(GetPropertyCompatible(CCX, sProperty));
		return pResult;
		}
	}

ICCItem *CSpaceObject::GetPropertyCompatible (CCodeChainCtx &Ctx, const CString &sName) const

//	GetProperty
//
//	Returns the property

	{
	int i;
	CCodeChain &CC = GetUniverse().GetCC();
	CDesignType *pType;

	if (strEquals(sName, PROPERTY_ASCENDED))
		return CC.CreateBool(IsAscended());

	else if (strEquals(sName, PROPERTY_CAN_ATTACK))
		return CC.CreateBool(CanAttack());

	else if (strEquals(sName, PROPERTY_CAN_BE_ATTACKED))
		return CC.CreateBool(CanBeAttacked());

	else if (strEquals(sName, PROPERTY_CATEGORY))
		{
		switch (GetCategory())
			{
			case catShip:
				return CC.CreateString(CATEGORY_SHIP);

			case catStation:
				return CC.CreateString(CATEGORY_STATION);

			case catBeam:
				return CC.CreateString(CATEGORY_BEAM);

			case catMissile:
				return CC.CreateString(CATEGORY_MISSILE);

			case catMission:
				return CC.CreateString(CATEGORY_MISSION);

			case catMarker:
				return CC.CreateString(CATEGORY_MARKER);

			default:
				return CC.CreateString(CATEGORY_EFFECT);
			}
		}
	else if (strEquals(sName, PROPERTY_COMMS_KEY))
		{
		if (m_iHighlightChar)
			{
			char chChar = m_iHighlightChar;
			return CC.CreateString(CString(&chChar, 1));
			}
		else if (m_iDesiredHighlightChar)
			{
			char chChar = m_iHighlightChar;
			return CC.CreateString(CString(&chChar, 1));
			}
		else
			return CC.CreateNil();
		}
	else if (strEquals(sName, PROPERTY_CURRENCY))
		return CC.CreateInteger(GetDefaultEconomy()->GetUNID());

	else if (strEquals(sName, PROPERTY_CURRENCY_NAME))
		return CC.CreateString(GetDefaultEconomy()->GetSID());

	else if (strEquals(sName, PROPERTY_CYBER_DEFENSE_LEVEL))
		return CC.CreateInteger(GetCyberDefenseLevel());

	else if (strEquals(sName, PROPERTY_DAMAGE_DESC))
		{
		ICCItem *pResult = CC.CreateSymbolTable();
		SVisibleDamage Damage;
		GetVisibleDamageDesc(Damage);

		if (Damage.iShieldLevel != -1)
			pResult->SetIntegerAt(FIELD_SHIELD_LEVEL, Damage.iShieldLevel);

		if (Damage.iArmorLevel != -1)
			pResult->SetIntegerAt(FIELD_ARMOR_INTEGRITY, Damage.iArmorLevel);

		if (Damage.iHullLevel != -1)
			pResult->SetIntegerAt(FIELD_HULL_INTEGRITY, Damage.iHullLevel);

		return pResult;
		}

	else if (strEquals(sName, PROPERTY_DESTINY))
		return CC.CreateInteger(GetDestiny());

	else if (strEquals(sName, PROPERTY_DOCKING_PORTS))
		{
		const CDockingPorts *pPorts = GetDockingPorts();
		if (pPorts == NULL || pPorts->GetPortCount(this) == 0)
			return CC.CreateNil();

		ICCItem *pList = CC.CreateLinkedList();
		for (i = 0; i < pPorts->GetPortCount(this); i++)
			{
			ICCItem *pPortDesc = CC.CreateSymbolTable();

			//	Status

			if (pPorts->IsPortEmpty(this, i))
				pPortDesc->SetStringAt(FIELD_STATUS, CONSTLIT("empty"));
			else
				pPortDesc->SetStringAt(FIELD_STATUS, CONSTLIT("inUse"));

			//	Position

			ICCItem *pValue = ::CreateListFromVector(pPorts->GetPortPos(this, i, NULL));
			pPortDesc->SetAt(FIELD_POS, pValue);
			pValue->Discard();

			//	ObjectID

			CSpaceObject *pObj = pPorts->GetPortObj(this, i);
			if (pObj)
				pPortDesc->SetIntegerAt(FIELD_OBJ_ID, pObj->GetID());

			//	Add to list

			pList->Append(pPortDesc);
			pPortDesc->Discard();
			}

		return pList;
		}

	else if (strEquals(sName, PROPERTY_EVENT_SUBSCRIBERS))
		{
		ICCItem *pResult = CC.CreateLinkedList();
		for (int i = 0; i < m_SubscribedObjs.GetCount(); i++)
			{
			CSpaceObject *pObj = m_SubscribedObjs.GetObj(i);
			if (!pObj->IsDestroyed())
				pResult->AppendInteger((int)pObj);
			}

		if (pResult->GetCount() == 0)
			{
			pResult->Discard();
			return CC.CreateNil();
			}
		else
			return pResult;
		}

	else if (strEquals(sName, PROPERTY_HAS_DOCKING_PORTS))
		return CC.CreateBool(GetDockingPortCount() > 0);

	else if (strEquals(sName, PROPERTY_ID))
		return CC.CreateInteger(GetID());

	else if (strEquals(sName, PROPERTY_IDENTIFIED))
		return CC.CreateBool(IsIdentified());

	else if (strEquals(sName, PROPERTY_INSTALL_ARMOR_MAX_LEVEL))
		{
		int iMaxLevel = GetTradeMaxLevel(serviceReplaceArmor);
		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_INSTALL_DEVICE_MAX_LEVEL))
		{
		int iMaxLevel = GetTradeMaxLevel(serviceInstallDevice);
		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_INSTALL_DEVICE_UPGRADE_ONLY))
		return CC.CreateBool(HasTradeUpgradeOnly(serviceInstallDevice));

	else if (strEquals(sName, PROPERTY_KNOWN))
		return CC.CreateBool(IsKnown());

	else if (strEquals(sName, PROPERTY_LEVEL))
		return CC.CreateInteger(GetLevel());

	else if (strEquals(sName, PROPERTY_MASS))
		return CC.CreateInteger((int)GetMass());

    else if (strEquals(sName, PROPERTY_NAME_PATTERN))
		{
		ICCItem *pResult = CC.CreateSymbolTable();
		DWORD dwFlags;
		pResult->SetStringAt(CONSTLIT("pattern"), GetNamePattern(0, &dwFlags));
		pResult->SetIntegerAt(CONSTLIT("flags"), dwFlags);
		return pResult;
		}

	else if (strEquals(sName, PROPERTY_PAINT_LAYER))
		return CC.CreateString(GetPaintLayerID(GetPaintLayer()));

	else if (strEquals(sName, PROPERTY_PLAYER_MISSIONS_GIVEN))
		{
		int iCount = GetUniverse().GetObjStats(GetID()).iPlayerMissionsGiven;
		if (iCount > 0)
			return CC.CreateInteger(iCount);
		else
			return CC.CreateNil();
		}

	else if (strEquals(sName, PROPERTY_RADIOACTIVE))
		return CC.CreateBool(IsRadioactive());

	else if (strEquals(sName, PROPERTY_REFUEL_MAX_LEVEL))
		{
		int iMaxLevel = GetTradeMaxLevel(serviceRefuel);
		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_REMOVE_DEVICE_MAX_LEVEL))
		{
		int iMaxLevel = GetTradeMaxLevel(serviceRemoveDevice);
		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_REPAIR_ARMOR_MAX_LEVEL))
		{
		int iMaxLevel = GetTradeMaxLevel(serviceRepairArmor);
		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_SCALE))
		{
		switch (GetScale())
			{
			case scaleStar:
				return CC.CreateString(SCALE_STAR);

			case scaleWorld:
				return CC.CreateString(SCALE_WORLD);

			case scaleStructure:
				return CC.CreateString(SCALE_STATION);

			case scaleShip:
				return CC.CreateString(SCALE_SHIP);

			case scaleFlotsam:
				return CC.CreateString(SCALE_FLOTSAM);

			default:
				return CC.CreateNil();
			}
		}

	else if (strEquals(sName, PROPERTY_SHOW_AS_DESTINATION))
		{
		if (!IsPlayerDestination())
			return CC.CreateNil();

		ICCItemPtr pResult(ICCItem::SymbolTable);
		pResult->SetBooleanAt(CONSTLIT("showDestination"), true);

		if (m_fAutoClearDestination)			pResult->SetBooleanAt(CONSTLIT("autoClear"), true);
		if (m_fAutoClearDestinationOnDestroy)	pResult->SetBooleanAt(CONSTLIT("autoClearOnDestroy"), true);
		if (m_fAutoClearDestinationOnDock)		pResult->SetBooleanAt(CONSTLIT("autoClearOnDock"), true);
		if (m_fAutoClearDestinationOnGate)		pResult->SetBooleanAt(CONSTLIT("autoClearOnGate"), true);
		if (m_fShowDistanceAndBearing)			pResult->SetBooleanAt(CONSTLIT("showDistance"), true);
		if (m_fShowHighlight)					pResult->SetBooleanAt(CONSTLIT("showHighlight"), true);

		return pResult->Reference();
		}

	else if (strEquals(sName, PROPERTY_SIZE_PIXELS))
		return CC.CreateInteger(GetHitSizePixels());

	else if (strEquals(sName, PROPERTY_SOVEREIGN))
		{
		CSovereign *pSovereign = GetSovereign();
		if (pSovereign)
			return CC.CreateInteger(pSovereign->GetUNID());
		else
			return CC.CreateNil();
		}

	else if (strEquals(sName, PROPERTY_STEALTH))
		return CC.CreateInteger(GetStealth());

	else if (strEquals(sName, PROPERTY_SUSPENDED))
		return CC.CreateBool(IsSuspended());

	else if (strEquals(sName, PROPERTY_TYPE))
		{
		if (CDesignType *pType = GetType())
			return CC.CreateInteger(pType->GetUNID());
		else
			return CC.CreateNil();
		}

	else if (strEquals(sName, PROPERTY_UNDER_ATTACK))
		return CC.CreateBool(IsUnderAttack());

	else if (pType = GetType())
		{
		EPropertyType iType;
		ICCItemPtr pResult = pType->GetProperty(Ctx, sName, &iType);

		//	If the property is an object property, then we need to look in 
		//	object data.

		if (iType == EPropertyType::propVariant || iType == EPropertyType::propData)
			return GetData(sName)->Reference();

		//	If this is a dynamic property, we need to evaluate.

		else if (iType == EPropertyType::propDynamicData)
			{
			CCodeChainCtx Ctx(GetUniverse());
			Ctx.SaveAndDefineSourceVar(this);

			ICCItemPtr pValue = Ctx.RunCode(pResult);
			return pValue->Reference();
			}

		//	Otherwise we have a valid property.

		else
			return pResult->Reference();
		}

	else
		return CC.CreateNil();
	}

bool CSpaceObject::SetProperty (const CString &sName, ICCItem *pValue, CString *retsError)

//	SetProperty
//
//	Sets an object property

	{
	if (strEquals(sName, PROPERTY_IDENTIFIED))
		{
		SetIdentified(!pValue->IsNil());
		return true;
		}
	else if (strEquals(sName, PROPERTY_COMMS_KEY))
		{
		CString sKey = pValue->GetStringValue();
		m_iDesiredHighlightChar = *sKey.GetASCIIZPointer();
		return true;
		}
	else if (strEquals(sName, PROPERTY_KNOWN))
		{
		SetKnown(!pValue->IsNil());
		return true;
		}
	else if (strEquals(sName, PROPERTY_SOVEREIGN))
		{
		CSovereign *pSovereign = GetUniverse().FindSovereign(pValue->GetIntegerValue());
		if (pSovereign == NULL)
			{
			if (retsError) *retsError = strPatternSubst(CONSTLIT("Unknown sovereign: %s."), pValue->GetStringValue());
			return false;
			}

		SetSovereign(pSovereign);
		return true;
		}

	//	See if this is a custom property, we set data

	else if (CDesignType *pType = GetType())
		{
		ICCItemPtr pDummy;
		EPropertyType iType;
		if (!pType->FindCustomProperty(sName, pDummy, &iType))
			return false;

		switch (iType)
			{
			case EPropertyType::propGlobal:
				pType->SetGlobalData(sName, pValue);
				return true;

			case EPropertyType::propData:
				SetData(sName, pValue);
				return true;

			default:
				return false;
			}
		}

	else
		return false;
	}
