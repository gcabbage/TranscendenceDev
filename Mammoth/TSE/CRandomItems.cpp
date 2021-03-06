//	CRandomItems.cpp
//
//	CRandomItems class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "ItemGeneratorImpl.h"

#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define CATEGORIES_ATTRIB						CONSTLIT("categories")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define DAMAGED_ATTRIB							CONSTLIT("damaged")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB						CONSTLIT("levelCurve")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define MODIFIERS_ATTRIB						CONSTLIT("modifiers")

//	CRandomItems ---------------------------------------------------------------

CRandomItems::~CRandomItems (void)

//	CRandomItems destructor

	{
	if (m_Table)
		delete [] m_Table;
	}

void CRandomItems::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	//	If this is a dynamic table, then we need to generate it now

	if (m_bDynamicLevelFrequency && m_iDynamicLevel != Ctx.iLevel)
		{
		SDesignLoadCtx LoadCtx(Ctx.GetUniverse());
		InitTable(LoadCtx, GenerateLevelFrequency(m_sLevelFrequency, Ctx.iLevel));
		m_iDynamicLevel = Ctx.iLevel;
		}

	//	Roll

	int iRoll = mathRandom(1, 1000);
	bool bAllAtOnce = (m_iDamaged == 0 && m_Enhanced.GetChance() == 0);

	for (int i = 0; i < m_iCount; i++)
		{
		iRoll -= m_Table[i].iProbability;

		if (iRoll <= 0)
			{
			CItemType *pType = m_Table[i].pType;
			int iCount = pType->GetNumberAppearing().Roll();

			//	If we don't have a chance of enhancement or damage, just optimize the
			//	result by adding a group of items.

			if (bAllAtOnce)
				Ctx.ItemList.AddItem(CItem(m_Table[i].pType, iCount));

			//	If this is armor, then treat them as a block

			else if (pType->GetCategory() == itemcatArmor)
				{
				CItem Item(m_Table[i].pType, iCount);

				if (mathRandom(1, 100) <= m_iDamaged)
					Item.SetDamaged();
				else
					m_Enhanced.EnhanceItem(Item);

				Ctx.ItemList.AddItem(Item);
				}

			//	Otherwise, enhance/damage each item individually

			else
				{
				for (int j = 0; j < iCount; j++)
					{
					CItem Item(m_Table[i].pType, 1);

					if (mathRandom(1, 100) <= m_iDamaged)
						Item.SetDamaged();
					else
						m_Enhanced.EnhanceItem(Item);

					Ctx.ItemList.AddItem(Item);
					}
				}
			break;
			}
		}
	}

void CRandomItems::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	//	Since random items tables don't explicitly refer to a type
	//	(they do it by criteria) we don't need to add them.
	}

ALERROR CRandomItems::Create (CUniverse &Universe,
							  const CItemCriteria &Crit, 
							  const CString &sLevelFrequency,
							  IItemGenerator **retpGenerator)

//	Create
//
//	Creates the table from criteria and level frequency

	{
	CRandomItems *pGenerator = new CRandomItems;
	pGenerator->m_Criteria = Crit;
	pGenerator->m_sLevelFrequency = sLevelFrequency;
	pGenerator->m_bDynamicLevelFrequency = (strFind(sLevelFrequency, CONSTLIT(":")) != -1);
	pGenerator->m_iDynamicLevel = 0;
	pGenerator->m_iDamaged = 0;
	pGenerator->m_iLevel = 0;
	pGenerator->m_iLevelCurve = 0;

	pGenerator->m_iCount = 0;
	pGenerator->m_Table = NULL;

	if (!pGenerator->m_bDynamicLevelFrequency)
		{
		SDesignLoadCtx LoadCtx(Universe);
		pGenerator->InitTable(LoadCtx, sLevelFrequency);
		if (pGenerator->m_iCount == 0)
			return ERR_FAIL;
		}

	//	Done

	*retpGenerator = pGenerator;

	return NOERROR;
	}

struct ItemEntryStruct
	{
	CItemType *pType;
	int iChance;
	int iRemainder;
	};

CurrencyValue CRandomItems::GetAverageValue (SItemAddCtx &Ctx, int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	//	If this is a dynamic table we need to compute all levels

	if (m_bDynamicLevelFrequency)
		{
		Metric rTotal = 0.0;
		SDesignLoadCtx LoadCtx(Ctx.GetUniverse());
		InitTable(LoadCtx, GenerateLevelFrequency(m_sLevelFrequency, iLevel));
		m_iDynamicLevel = iLevel;

		for (int i = 0; i < m_iCount; i++)
			{
			CItemType *pType = m_Table[i].pType;
			CurrencyValue ItemValue = CSingleItem::CalcItemValue(pType);
			rTotal += (pType->GetNumberAppearing().GetAveValueFloat() * (Metric)ItemValue * (Metric)m_Table[i].iProbability / 1000.0);
			}

		return (CurrencyValue)(rTotal + 0.5);
		}

	//	Otherwise the table is already initialized.

	else
		{
		//	Average value is proportional to chances.

		Metric rTotal = 0.0;
		for (int i = 0; i < m_iCount; i++)
			{
			CItemType *pType = m_Table[i].pType;
			CurrencyValue ItemValue = CSingleItem::CalcItemValue(pType);
			rTotal += (pType->GetNumberAppearing().GetAveValueFloat() * (Metric)ItemValue * (Metric)m_Table[i].iProbability / 1000.0);
			}

		return (CurrencyValue)(rTotal + 0.5);
		}
	}

CItemTypeProbabilityTable CRandomItems::GetProbabilityTable (SItemAddCtx &Ctx) const

//	GetProbabilityTable
//
//	Returns a table with the probability of each item type appearing.

	{
	//	If this is a dynamic table, then we need to generate it now

	if (m_bDynamicLevelFrequency && m_iDynamicLevel != Ctx.iLevel)
		{
		SDesignLoadCtx LoadCtx(Ctx.GetUniverse());
		InitTable(LoadCtx, GenerateLevelFrequency(m_sLevelFrequency, Ctx.iLevel));
		m_iDynamicLevel = Ctx.iLevel;
		}

	//	Short-circuit

	if (m_iCount == 0)
		return CItemTypeProbabilityTable();

	//	Scale chances

	CItemTypeProbabilityTable Result;
	for (int i = 0; i < m_iCount; i++)
		Result.Add(m_Table[i].pType, (Metric)m_Table[i].iProbability / 1000.0);

	return Result;
	}

bool CRandomItems::HasItemAttribute (const CString &sAttrib) const

//	HasItemAttribute
//
//	Returns TRUE if any item has the given attribute

	{
	for (int i = 0; i < m_iCount; i++)
		if (m_Table[i].pType->HasAttribute(sAttrib))
			return true;

	return false;
	}

void CRandomItems::InitTable (SDesignLoadCtx &Ctx, const CString &sLevelFrequency) const

//	InitTable
//
//	Initializes the m_Table array.
//
//	We assume that m_Criteria, m_iLevel, and m_iLevelCurve are properly initialized.

	{
	CUniverse &Universe = Ctx.GetUniverse();

	//	Free original, if necessary

	if (m_Table)
		delete [] m_Table;

	//	Start by allocating an array large enough to hold
	//	all item types in the universe

	ItemEntryStruct *pTable = new ItemEntryStruct [Universe.GetItemTypeCount()];
	int iTableSize = 0;

	//	Figure out if we should use level curves or level frequency

	bool bUseLevelFrequency = !sLevelFrequency.IsBlank();

	//	Iterate over every item type and add it to the table if
	//	it matches the given criteria

	for (int i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pType = Universe.GetItemType(i);
		CItem Item(pType, 1);

		//	If we're inside Bind, then make sure the item type is bound because
		//	we need to refer to it inside the criteria code.

		if (!Universe.GetDesignCollection().IsBindComplete()
				&& !pType->IsBound())
			{
			if (pType->BindDesign(Ctx) != NOERROR)
				{
				Universe.LogOutput(strPatternSubst(CONSTLIT("CRandomItems::InitTable Bind: %s"), Ctx.sError));
				continue;
				}
			}

		//	Skip if this item is not found randomly

		if (pType->GetFrequency() == ftNotRandom)
			continue;

		//	Skip if the item doesn't match the categories

		if (!Item.MatchesCriteria(m_Criteria))
			continue;

		//	Adjust score based on level, either the level curve
		//	or the level frequency string.

		int iScore;
		if (bUseLevelFrequency)
			{
			iScore = 1000 * GetFrequencyByLevel(sLevelFrequency, pType->GetLevel()) / ftCommon;
			}
		else
			{
			//	Skip if the item is not within the level curve

			if ((pType->GetLevel() < m_iLevel - m_iLevelCurve)
					|| (pType->GetLevel() > m_iLevel + m_iLevelCurve))
				continue;

			//	If we get this far then the item perfectly matches
			//	and we need to add it to our table. First, however, we need
			//	to calculate a score.
			//
			//	The score is number that represents how common the item
			//	is in the table. Later we normalize the score to be a probability

			int iLevelDelta = pType->GetLevel() - m_iLevel;
			switch (iLevelDelta)
				{
				case 0:
					iScore = 1000;
					break;

				case 1:
				case -1:
					iScore = 500;
					break;

				case 2:
				case -2:
					iScore = 200;
					break;

				default:
					iScore = 50;
				}
			}

		//	Adjust score based on item frequency

		iScore = iScore * pType->GetFrequency() * 10 / (ftCommon * 10);

		//	If we have a score of 0 then we skip this item

		if (iScore == 0)
			continue;

		//	Add the item to the table

		pTable[iTableSize].pType = pType;
		pTable[iTableSize].iChance = iScore;
		iTableSize++;
		}

	//	We must have items

	if (iTableSize == 0)
		{
		m_iCount = 0;
		m_Table = NULL;
		delete [] pTable;
		return;
		}

	//	Add up the total score of all items

	int iTotalScore = 0;
	for (int i = 0; i < iTableSize; i++)
		iTotalScore += pTable[i].iChance;

	//	Compute the chance

	int iTotalChance = 0;
	for (int i = 0; i < iTableSize; i++)
		{
		int iScore = pTable[i].iChance;
		pTable[i].iChance = (iScore * 1000) / iTotalScore;
		pTable[i].iRemainder = (iScore * 1000) % iTotalScore;

		iTotalChance += pTable[i].iChance;
		}

	//	Distribute the remaining chance points

	while (iTotalChance < 1000)
		{
		//	Look for the entry with the biggest remainder

		int iBestRemainder = 0;
		int iBestEntry = -1;

		for (int i = 0; i < iTableSize; i++)
			if (pTable[i].iRemainder > iBestRemainder)
				{
				iBestRemainder = pTable[i].iRemainder;
				iBestEntry = i;
				}

		pTable[iBestEntry].iChance++;
		pTable[iBestEntry].iRemainder = 0;
		iTotalChance++;
		}

	//	Count the number of entries that we've got

	m_iCount = 0;
	for (int i = 0; i < iTableSize; i++)
		if (pTable[i].iChance > 0)
			m_iCount++;

	//	Now loop over the entire table and add it to the 
	//	random entry generator

	m_Table = new SEntry [m_iCount];
	int j = 0;
	for (int i = 0; i < iTableSize; i++)
		if (pTable[i].iChance > 0)
			{
			m_Table[j].pType = pTable[i].pType;
			m_Table[j].iProbability = pTable[i].iChance;
			j++;
			}

	//	Done

	delete [] pTable;
	}

ALERROR CRandomItems::LoadFromXML (SDesignLoadCtx &Ctx, const CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	CString sCriteria = pDesc->GetAttribute(CRITERIA_ATTRIB);
	if (sCriteria.IsBlank())
		{
		CString sAttributes = pDesc->GetAttribute(ATTRIBUTES_ATTRIB);
		if (sAttributes.IsBlank())
			sAttributes = pDesc->GetAttribute(MODIFIERS_ATTRIB);

		sCriteria = strPatternSubst(CONSTLIT("%s %s"), pDesc->GetAttribute(CATEGORIES_ATTRIB), sAttributes);
		}

	m_Criteria.Init(sCriteria);
	m_sLevelFrequency = pDesc->GetAttribute(LEVEL_FREQUENCY_ATTRIB);
	m_bDynamicLevelFrequency = (strFind(m_sLevelFrequency, CONSTLIT(":")) != -1);
	m_iDynamicLevel = 0;

	m_iLevel = pDesc->GetAttributeInteger(LEVEL_ATTRIB);
	m_iLevelCurve = pDesc->GetAttributeInteger(LEVEL_CURVE_ATTRIB);
	m_iDamaged = pDesc->GetAttributeInteger(DAMAGED_ATTRIB);

	if (error = m_Enhanced.InitFromXML(Ctx, pDesc))
		return error;

	m_Table = NULL;
	m_iCount = 0;

	return NOERROR;
	}

ALERROR CRandomItems::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	if (!m_bDynamicLevelFrequency)
		InitTable(Ctx, m_sLevelFrequency);

	//	Reset

	m_iDynamicLevel = 0;

	return NOERROR;
	}
