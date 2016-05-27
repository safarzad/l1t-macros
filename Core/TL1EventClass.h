#ifndef TL1EVENTCLASS_H
#define TL1EVENTCLASS_H

#include <string>
#include <memory>

#include <TMath.h>
#include <TVector2.h>

#include "TL1PrimitiveEventClass.h"

//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisL1CaloTowerDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisRecoJetDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisRecoVertexDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetFilterDataFormat.h"
//#include "../160510_l1t-integration-v46p0/CMSSW_8_0_7/src/L1Trigger/L1TNtuples/interface/L1AnalysisRecoMuon2DataFormat.h"

class TL1EventClass
{
    public:
        TL1EventClass(std::string inDir);

        bool Next();
        void GetEntry(int i);
        void GetDerivatives();

        TL1PrimitiveEventClass const * GetPEvent() const;

        // Get L1
        double fL1Met, fL1Mht, fL1Ett, fL1Htt, fL1MetPhi, fL1MhtPhi;
        std::vector<double> fL1JetEt, fL1JetPhi, fL1JetEta;

        // Filter flags
        bool fMuonFilterPassFlag, fMetFilterPassFlag;
        vector<bool> fJetFilterPassFlags;

        // Recalc L1 Ht/Et Sums
        double fRecalcL1Mht, fRecalcL1MhtPhi;
        double fRecalcL1Ett;

        // Recalc Reco Ht/Et Sums
        double fRecalcRecoMht, fRecalcRecoMhtPhi, fRecalcRecoHtt;
        bool fMhtPassFlag;
        double fRecalcRecoEtt;

    private:
        std::shared_ptr<TL1PrimitiveEventClass> fPrimitiveEvent;

        // Get L1
        void GetL1Jets();
        void GetL1Sums();

        // Filters
        void MuonFilter();
        void JetFilter();
        void SumsFilter();

        // Recalc L1 Ht/Et Sums
        void GetRecalcL1Mht();
        void GetRecalcL1Ett();

        // Recalc Reco Ht/Et Sums
        void GetRecalcRecoHtSums();
        void GetRecalcRecoEtt();
};

TL1EventClass::TL1EventClass(std::string inDir) :
    fPrimitiveEvent(new TL1PrimitiveEventClass(inDir)),
    fMuonFilterPassFlag(true), fMetFilterPassFlag(true),
    fMhtPassFlag(true),
{
}

bool TL1EventClass::Next()
{
    bool isNext = fPrimitiveEvent->Next();
    this->GetDerivatives();
    return isNext;
}

void TL1EventClass::GetEntry(int i)
{
    fPrimitiveEvent->GetEntry(i);
    this->GetDerivatives();
}

void TL1EventClass::GetDerivatives()
{
    this->GetL1Jets();
    this->GetL1Sums();

    this->MuonFilter();
    this->JetFilter();
    this->SumsFilter();

    this->GetRecalcL1Mht();
    this->GetRecalcL1Ett();
    
    this->GetRecalcRecoHtSums();
    this->GetRecalcRecoEtt();
}

TL1PrimitiveEventClass const * TL1EventClass::GetPEvent() const
{
    return fPrimitiveEvent.get()
}

void TL1EventClass::GetL1Jets()
{
    fL1JetEt.clear();
    fL1JetEta.clear();
    fL1JetPhi.clear();
    for(unsigned iJet=0; iJet<fUpgrade->nJets; ++iJet)
    {
        if( fUpgrade->jetBx[iJet] == 0 )
        {
            fL1JetEt.push_back(fUpgrade->jetEt[iJet]);
            fL1JetEta.push_back(fUpgrade->jetEta[iJet]);
            fL1JetPhi.push_back(fUpgrade->jetPhi[iJet]);
        }
    }
}

void TL1EventClass::GetL1Sums()
{
    for(int iter=0; iter<fUpgrade->nSums; ++iter)
    {
        double et = fUpgrade->sumEt[iter];
        double phi = fUpgrade->sumPhi[iter];
        if( fUpgrade->sumBx[iter] == 0 )
        {
            if(fUpgrade->sumType[iter] == L1Analysis::kTotalEt)   fL1Ett = et;
            if(fUpgrade->sumType[iter] == L1Analysis::kTotalHt)   fL1Htt = et;
            if(fUpgrade->sumType[iter] == L1Analysis::kMissingEt)
            {
                fL1Met = et;
                fL1MetPhi = phi;
            }
            if(fUpgrade->sumType[iter] == L1Analysis::kMissingHt)
            {
                fL1Mht = et;
                fL1MhtPhi = phi;
            }
        }
    }
}

void TL1EventClass::MuonFilter()
{
    bool pass(false);
    for(unsigned jMuon=0; jMuon<fPrimitiveEvent->fMuons->nMuons; ++jMuon)
    {
        double pt = fPrimitiveEvent->fMuons->pt[jMuon];
        double iso = fPrimitiveEvent->fMuons->iso[jMuon];
        double isLoose = fPrimitiveEvent->fMuons->isLooseMuon[jMuon];
        if( pt>=20.0 && iso<=0.1 && isLoose==1.0 )
            pass = true;
    }
    fMuonFilterPassFlag = pass;
}

void TL1EventClass::JetFilter()
{
    for(unsigned iJet=0; iJet<fPrimitiveEvent->fJets->nJets; ++iJet)
    {
        double eta  = fPrimitiveEvent->fJets->eta[iJet];
        double nhef = fPrimitiveEvent->fJets->nhef[iJet];
        double pef  = fPrimitiveEvent->fJets->pef[iJet];
        double mef  = fPrimitiveEvent->fJets->mef[iJet];
        double chef = fPrimitiveEvent->fJets->chef[iJet];
        double eef  = fPrimitiveEvent->fJets->eef[iJet];
        int chMult  = fPrimitiveEvent->fJets->chMult[iJet];
        int nhMult  = fPrimitiveEvent->fJets->nhMult[iJet];
        int phMult  = fPrimitiveEvent->fJets->phMult[iJet];
        int elMult  = fPrimitiveEvent->fJets->elMult[iJet];
        int muMult  = fPrimitiveEvent->fJets->muMult[iJet];
        bool jetPass;
        if (abs(eta)<=3.00 && nhef<0.90 && pef<0.90 && (chMult+nhMult+phMult+elMult+muMult)>1 && mef<0.8 
                && (abs(eta)>2.4||(abs(eta)<=2.4 && chef>0 && (chMult+elMult+muMult)>0 && eef<0.90))){
            jetPass = true;}//this jet has passed
        else{jetPass = false;}//this jet has failed, or is outside the central zone           
        fJetFilterPassFlags.pushback(jetPass && muMult==0);
    }
}

void TL1EventClass::SumsFilter()
{
    fMetFilterPassFlag = fPrimitiveEvent->fMetFilters->hbheNoiseFilter;
}

void TL1EventClass::GetRecalcL1Mht()
{
    std::shared_ptr<TVector2> mht(new TVector2(0.0,0.0));
    for(unsigned iJet=0; iJet<fL1JetEt.size(); ++iJet)
    {
        if( abs(fL1JetEta[iJet]) >= 3.0 ) continue;
        if( fL1JetEt[iJet] < 30.0 ) continue;
        std::shared_ptr<TVector2> jet(new TVector2(0.0,0.0));
        jet->SetMagPhi(fL1JetEt[iJet], fL1JetPhi[iJet]);
        *(mht.get()) -= *(jet.get());
    }
    fRecalcL1Mht = mht->Mod();
    fRecalcL1MhtPhi = mht->Phi();
}

void TL1EventClass::GetRecalcL1Ett()
{
    int iEtaMax = 28;
    Double_t sumEt = 0.0;
    for(unsigned jTower=0; jTower<fPrimitiveEvent->fCaloTowers->nTower; ++jTower)
    {
        int ieta = fPrimitiveEvent->fCaloTowers->ieta[jTower];
        int iet  = fPrimitiveEvent->fCaloTowers->iet[jTower];
        if( abs(ieta) <= iEtaMax )
            sumEt += 0.5 * (double)iet;
    }
    fRecalcL1Ett = sumEt;
}

void TL1EventClass::GetRecalcRecoHtSums()
{
    std::shared_ptr<TVector2> mht(new TVector2(0.,0.));
    double jetEt(0.0);
    unsigned jetCount(0);
    for(int iJet=0; iJet<fJets->nJets; ++iJet)
    {
        bool passJetFilter = fJetFilterPassFlags[iJet];
        if( fPrimitiveEvent->fJets->etCorr[iJet] < 30.0 ) continue;
        if( !passJetFilter )
        {
            fMhtPassFlag = false;
            break;
        }
        std::shared_ptr<TVector2> jet(new TVector2(0.,0.));
        jet->SetMagPhi(fJets->etCorr[iJet], fJets->phi[iJet]);
        jetEt += fJets->etCorr[iJet];
        ++jetCount;

        *(mht.get()) -= *(jet.get());
    }
    fRecalcRecoMht = mht->Mod();
    fRecalcRecoMhtPhi = mht->Phi();
    fRecalcRecoHtt = jetEt;
}

void TL1EventClass::GetRecalcRecoEtt()
{
    double jetEt(0.0);
    for(int iJet=0; iJet<fJets->nJets; ++iJet)
        if( fJets->etCorr[iJet] >= 30.0 )
            jetEt += fJets->etCorr[iJet];
    fRecalcRecoEtt = jetEt;
}

#endif
