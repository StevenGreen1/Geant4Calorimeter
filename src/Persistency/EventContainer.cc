/**
 *  @file   src/EventContainer.cc
 *
 *  @brief  Implementation of the EventContainer class.
 *
 *  $Log: $
 */

#include "Persistency/EventContainer.hh"
#include "Xml/tinyxml.hh"

EventContainer::EventContainer(const InputParameters *pInputParameters) :
    m_eventNumber(0),
    m_pInputParameters(pInputParameters)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------ 

EventContainer::~EventContainer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------ 

void EventContainer::BeginOfEventAction()
{
    m_mcParticles.push_back(MCParticleList());
    m_cells.push_back(CellList());
}

//------------------------------------------------------------------------------------------------------------------------------------------ 

void EventContainer::EndOfEventAction()
{
    m_eventNumber++;
}

//------------------------------------------------------------------------------------------------------------------------------------------ 

void EventContainer::SaveXml()
{
    TiXmlDocument tiXmlDocument;

    TiXmlElement *pRunTiXmlElement = new TiXmlElement("Run");
    tiXmlDocument.LinkEndChild(pRunTiXmlElement);

    for (int eventNumber = 0; eventNumber < m_eventNumber; eventNumber++)
    {
        TiXmlElement *pEventTiXmlElement = new TiXmlElement("Event");
        pRunTiXmlElement->LinkEndChild(pEventTiXmlElement);

        const CellList &cellList(m_cells.at(eventNumber));
        const MCParticleList &mcParticleList(m_mcParticles.at(eventNumber));

        // Cells
        for (const auto iter : cellList.m_idCellMap)
        {
            const Cell *pCell(iter.second);

            IntFloatVector trackIdToEnergy(cellList.m_mcComponents.at(pCell->GetIdx()));

            // ATTN : Doesn't account for track ID offset, but not using for now
            int mainMCTrackId(-1);
            float largestEnergyContribution(0.f);
            for (const auto contribution : trackIdToEnergy)
            {
                if (contribution.second > largestEnergyContribution)
                {
                    largestEnergyContribution = contribution.second;
                    mainMCTrackId = contribution.first;
                }
            }

            if (largestEnergyContribution < std::numeric_limits<float>::epsilon())
                continue;

            int mainVisibleMCTrackId(mainMCTrackId);
            while (!mcParticleList.KnownParticle(mainVisibleMCTrackId))
            {
                if (mcParticleList.m_trackIdParentMap.find(mainVisibleMCTrackId) != mcParticleList.m_trackIdParentMap.end())
                {
                    mainVisibleMCTrackId = mcParticleList.m_trackIdParentMap.at(mainVisibleMCTrackId);
                }
                else
                {
                    mainVisibleMCTrackId = 0;
                    break;
                }
            }

            TiXmlElement *pTiXmlElement = new TiXmlElement("Cell");
            pTiXmlElement->SetAttribute("Id", pCell->GetIdx());
            pTiXmlElement->SetAttribute("MCId", mainVisibleMCTrackId);
            pTiXmlElement->SetDoubleAttribute("X", pCell->GetX());
            pTiXmlElement->SetDoubleAttribute("Y", pCell->GetY());
            pTiXmlElement->SetDoubleAttribute("Z", pCell->GetZ());
            pTiXmlElement->SetDoubleAttribute("Energy", pCell->GetEnergy());
            pEventTiXmlElement->LinkEndChild(pTiXmlElement);
        }

        // MCParticles
        for (const auto iter : mcParticleList.m_mcParticles)
        {
            const MCParticle *pMCParticle(iter.second);

            TiXmlElement *pTiXmlElement = new TiXmlElement("MCParticle");
            pTiXmlElement->SetAttribute("Id", pMCParticle->GetTrackId());
            pTiXmlElement->SetAttribute("PDG", pMCParticle->GetPDGCode());
            pTiXmlElement->SetAttribute("ParentId", pMCParticle->GetParent());
            pTiXmlElement->SetDoubleAttribute("Mass", pMCParticle->GetMass());
            pTiXmlElement->SetDoubleAttribute("Energy", pMCParticle->GetEnergy());
            pTiXmlElement->SetDoubleAttribute("StartX", pMCParticle->GetPositionX());
            pTiXmlElement->SetDoubleAttribute("StartY", pMCParticle->GetPositionY());
            pTiXmlElement->SetDoubleAttribute("StartZ", pMCParticle->GetPositionZ());
            pTiXmlElement->SetDoubleAttribute("EndX", pMCParticle->GetEndPositionX());
            pTiXmlElement->SetDoubleAttribute("EndY", pMCParticle->GetEndPositionY());
            pTiXmlElement->SetDoubleAttribute("EndZ", pMCParticle->GetEndPositionZ());
            pTiXmlElement->SetDoubleAttribute("MomentumX", pMCParticle->GetMomentumX());
            pTiXmlElement->SetDoubleAttribute("MomentumY", pMCParticle->GetMomentumY());
            pTiXmlElement->SetDoubleAttribute("MomentumZ", pMCParticle->GetMomentumZ());
            pEventTiXmlElement->LinkEndChild(pTiXmlElement);
        }
    }

    tiXmlDocument.SaveFile(m_pInputParameters->GetOutputXmlFileName());
    tiXmlDocument.Clear();
}

