/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "bd.h"

/////////////////////////////////////////////////
// Blob-Pointer
ClassTVQ(TBlobPt, TBlobPtV, TBlobPtQ)//{
public:
  static const int MnBlobBfL;
  static const int Flags;
  uchar Seg;
  uint Addr;
  TB8Set FSet1, FSet2, FSet3;
public:
  TBlobPt():
    Seg(0), Addr(TUInt::Mx), FSet1(), FSet2(), FSet3(){}
  TBlobPt(const TBlobPt& Pt):
    Seg(Pt.Seg), Addr(Pt.Addr),
    FSet1(Pt.FSet1), FSet2(Pt.FSet2), FSet3(Pt.FSet3){}
  TBlobPt(const uchar& _Seg, const uint& _Addr,
   const TB8Set& _FSet1, const TB8Set& _FSet2, const TB8Set& _FSet3):
    Seg(_Seg), Addr(_Addr), FSet1(_FSet1), FSet2(_FSet2), FSet3(_FSet3){}
  TBlobPt(const uchar& _Seg, const uint& _Addr):
    Seg(_Seg), Addr(_Addr), FSet1(), FSet2(), FSet3(){}
  TBlobPt(const uint& _Addr):
    Seg(0), Addr(_Addr), FSet1(), FSet2(), FSet3(){}
  TBlobPt(const int& _Addr):
    Seg(0), Addr(uint(_Addr)), FSet1(), FSet2(), FSet3(){IAssert(_Addr>=0);}
  ~TBlobPt(){}
  TBlobPt(TSIn& SIn){
    SIn.Load(Seg); SIn.Load(Addr);
    FSet1=TB8Set(SIn); FSet2=TB8Set(SIn); FSet3=TB8Set(SIn);}
  void Save(TSOut& SOut) const {
    SOut.Save(Seg); SOut.Save(Addr);
    FSet1.Save(SOut); FSet2.Save(SOut); FSet3.Save(SOut);}

  TBlobPt& operator=(const TBlobPt& Pt){
    if (this!=&Pt){
      Seg=Pt.Seg; Addr=Pt.Addr;
      FSet1=Pt.FSet1; FSet2=Pt.FSet2; FSet3=Pt.FSet3;}
    return *this;}
  bool operator==(const TBlobPt& Pt) const {
    return (Seg==Pt.Seg)&&(Addr==Pt.Addr);}
  bool operator<(const TBlobPt& Pt) const {
    return (Seg<Pt.Seg)||((Seg==Pt.Seg)&&(Addr<Pt.Addr));}
  int GetMemUsed() const {return sizeof(TBlobPt);}

  int GetPrimHashCd() const {return abs(int(Addr));}
  int GetSecHashCd() const {return (abs(int(Addr))+int(Seg)*0x10);}

  bool Empty() const {return Addr==TUInt::Mx;}
  void Clr(){Seg=0; Addr=TUInt::Mx;}
  void PutSeg(const uchar& _Seg){Seg=_Seg;}
  uchar GetSeg() const {return Seg;}
  void PutAddr(const uint& _Addr){Addr=_Addr;}
  uint GetAddr() const {return Addr;}
  void PutFlag(const int& FlagN, const bool& Val);
  bool IsFlag(const int& FlagN) const;
  void MergeFlags(const TBlobPt& Pt){
    FSet1|=Pt.FSet1; FSet2|=Pt.FSet2; FSet3|=Pt.FSet3;}
  void PutFSet(const int& FSetN, const TB8Set& FSet);
  TB8Set GetFSet(const int& FSetN);

  static TBlobPt Load(const PFRnd& FRnd){
    uchar Seg=FRnd->GetUCh(); uint Addr=FRnd->GetUInt();
    TB8Set B8Set1(FRnd->GetUCh()); TB8Set B8Set2(FRnd->GetUCh());
    TB8Set B8Set3(FRnd->GetUCh());
    return TBlobPt(Seg, Addr, B8Set1, B8Set2, B8Set3);}
  void Save(const PFRnd& FRnd) const {
    FRnd->PutUCh(Seg); FRnd->PutUInt(Addr);
    FRnd->PutUCh(FSet1.GetUCh()); FRnd->PutUCh(FSet2.GetUCh());
    FRnd->PutUCh(FSet3.GetUCh());}
  static TBlobPt LoadAddr(const PFRnd& FRnd, const uchar& Seg=0){
    return TBlobPt(Seg, FRnd->GetUInt());}
  void SaveAddr(const PFRnd& FRnd) const {
    FRnd->PutUInt(Addr);}

  TStr GetAddrStr() const {
    TChA AddrChA; AddrChA+=TInt::GetStr(Seg); AddrChA+=':';
    AddrChA+=TUInt::GetStr(Addr); return AddrChA;}

  TStr GetStr() const;
};

/////////////////////////////////////////////////
// Statistics for TBlobBs
class TBlobBsStats {
public:
	uint64 Puts;
	uint64 PutsNew;
	uint64 Gets;
	uint64 Dels;
	uint64 SizeChngs;
	double AvgGetLen;
	double AvgPutLen;
	double AvgPutNewLen;
	TBlobBsStats() { Reset(); }
	void Reset() {
		AvgPutNewLen = AvgGetLen = AvgPutLen = 0;
		Dels = Puts = PutsNew = Gets = SizeChngs = 0;
	}
	TBlobBsStats Clone() const {
		TBlobBsStats res;
		res.AvgGetLen = this->AvgGetLen;
		res.AvgPutLen = this->AvgPutLen;
		res.AvgPutNewLen = this->AvgPutNewLen;
		res.Dels = this->Dels;
		res.Gets = this->Gets;
		res.Puts = this->Puts;
		res.PutsNew = this->PutsNew;
		res.SizeChngs = this->SizeChngs;
		return res;
	}
	void Add(const TBlobBsStats& Othr) {
		Puts += Othr.Puts;
		PutsNew += Othr.PutsNew;
		Gets += Othr.Gets;
		SizeChngs += Othr.SizeChngs;
		Dels += Othr.Dels;

		AvgPutNewLen = 0;
		AvgPutLen = 0;
		AvgGetLen = 0;

		if (PutsNew + Othr.PutsNew > 0) {
			AvgPutNewLen = (AvgPutNewLen*PutsNew + Othr.AvgPutNewLen*Othr.PutsNew) / (PutsNew + Othr.PutsNew);
		}
		if (Gets + Othr.Gets) {
			AvgGetLen = (AvgGetLen*Gets + Othr.AvgGetLen*Othr.Gets) / (Gets + Othr.Gets);
		}
		if (Puts + Othr.Puts > 0) {
			AvgPutLen = (AvgPutLen*Puts + Othr.AvgPutLen*Othr.Puts) / (Puts + Othr.Puts);
		}
	}
	static TBlobBsStats Add(const TBlobBsStats& Stat1, const TBlobBsStats& Stat2) {
		TBlobBsStats res = Stat1.Clone();
		res.Add(Stat2);
		return res;
	}
};

/////////////////////////////////////////////////
// Blob-Base
typedef enum {bbsUndef, bbsOpened, bbsClosed} TBlobBsState;
typedef enum {btUndef, btBegin, btEnd} TBlobTag;
typedef enum {bsUndef, bsActive, bsFree} TBlobState;

ClassTPV(TBlobBs, PBlobBs, TBlobBsV)//{
public:
  static const int MnBlobBfL;
  static const int MxBlobFLen;
  UndefCopyAssign(TBlobBs);  
public:
  TBlobBs(){}
  virtual ~TBlobBs(){}
  TBlobBs(TSIn&){Fail;}
  static PBlobBs Load(TSIn&){Fail; return NULL;}
  void Save(TSOut&) const {Fail;}

  virtual TStr GetVersionStr() const=0;
  void PutVersionStr(const PFRnd& FBlobBs);
  void AssertVersionStr(const PFRnd& FBlobBs);

  TStr GetBlobBsStateStr(const TBlobBsState& BlobBsState);
  int GetStateStrLen(){return 6;}
  void PutBlobBsStateStr(const PFRnd& FBlobBs, const TBlobBsState& State);
  void AssertBlobBsStateStr(const PFRnd& FBlobBs, const TBlobBsState& State);

  static const TStr MxSegLenVNm;
  void PutMxSegLen(const PFRnd& FBlobBs, const int& MxSegLen);
  int GetMxSegLen(const PFRnd& FBlobBs);

  static const TStr BlockLenVNm;
  void GenBlockLenV(TIntV& BlockLenV);
  void PutBlockLenV(const PFRnd& FBlobBs, const TIntV& BlockLenV);
  void GetBlockLenV(const PFRnd& FBlobBs, TIntV& BlockLenV);

  static const TStr FFreeBlobPtVNm;
  void GenFFreeBlobPtV(const TIntV& BlockLenV, TBlobPtV& FFreeBlobPtV);
  void PutFFreeBlobPtV(const PFRnd& FBlobBs, const TBlobPtV& FFreeBlobPtV);
  void GetFFreeBlobPtV(const PFRnd& FBlobBs, TBlobPtV& FFreeBlobPtV);

  void GetAllocInfo(
   const int& BfL, const TIntV& BlockLenV, int& MxBfL, int& FFreeBlobPtN);

  uint GetBeginBlobTag(){return 0xABCDEFFF;}
  uint GetEndBlobTag(){return 0xFFFEDCBA;}
  void PutBlobTag(const PFRnd& FBlobBs, const TBlobTag& BlobTag);
  void AssertBlobTag(const PFRnd& FBlobBs, const TBlobTag& BlobTag);

  void PutBlobState(const PFRnd& FBlobBs, const TBlobState& State);
  TBlobState GetBlobState(const PFRnd& FBlobBs);
  void AssertBlobState(const PFRnd& FBlobBs, const TBlobState& State);

  void AssertBfCsEqFlCs(const TCs& BfCs, const TCs& FCs);

  virtual TBlobPt PutBlob(const PSIn& SIn)=0;
  TBlobPt PutBlob(const TStr& Str){
    PSIn SIn=TStrIn::New(Str); return PutBlob(SIn);}
  virtual TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn)=0;
  virtual PSIn GetBlob(const TBlobPt& BlobPt)=0;
  virtual void DelBlob(const TBlobPt& BlobPt)=0;

  virtual TBlobPt GetFirstBlobPt()=0;
  virtual TBlobPt FFirstBlobPt()=0;
  virtual bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn)=0;
  bool FNextBlobPt(TBlobPt& TrvBlobPt, PSIn& BlobSIn){
    TBlobPt BlobPt; return FNextBlobPt(TrvBlobPt, BlobPt, BlobSIn);}

  virtual const TBlobBsStats& GetStats()=0;
  virtual void ResetStats() = 0;
};

/////////////////////////////////////////////////
// General-Blob-Base
class TGBlobBs: public TBlobBs{
private:
  /// Random-access file - BLOB storage
  PFRnd FBlobBs;
  /// access mode for file
  TFAccess Access;
  /// maximal length of segment - of BLOB file
  int MxSegLen;
  /// list of precomputed block lengths - each BLOB falls into one of them, 
  /// so that allocations happen in just several possible chuncks
  TIntV BlockLenV;
  /// list of free blob pointers (their content was deleted, so blobs are free)
  TBlobPtV FFreeBlobPtV;
  TBlobPt FirstBlobPt;
  static TStr GetNrBlobBsFNm(const TStr& BlobBsFNm);
  TBlobBsStats Stats;
public:
  TGBlobBs(const TStr& BlobBsFNm, const TFAccess& _Access=faRdOnly,
   const int& _MxSegLen=-1);
  static PBlobBs New(const TStr& BlobBsFNm, const TFAccess& Access=faRdOnly,
   const int& MxSegLen=-1){
    return PBlobBs(new TGBlobBs(BlobBsFNm, Access, MxSegLen));}
  ~TGBlobBs();

  TGBlobBs& operator=(const TGBlobBs&){Fail; return *this;}

  TStr GetVersionStr() const {return TStr("General Blob Base Format 1.0");}
  TBlobPt PutBlob(const PSIn& SIn);
  TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn);
  PSIn GetBlob(const TBlobPt& BlobPt);
  void DelBlob(const TBlobPt& BlobPt);

  TBlobPt GetFirstBlobPt(){return FirstBlobPt;}
  TBlobPt FFirstBlobPt();
  bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn);

  static bool Exists(const TStr& BlobBsFNm);

  const TBlobBsStats& GetStats() { return Stats; }
  void ResetStats() { Stats.Reset(); }
};

/////////////////////////////////////////////////
// Multiple-File-Blob-Base
class TMBlobBs: public TBlobBs{
private:
  TFAccess Access;
  int MxSegLen;
  TStr NrFPath, NrFMid;
  TBlobBsV SegV;
  int CurSegN;
  static void GetNrFPathFMid(const TStr& BlobBsFNm, TStr& NrFPath, TStr& NrFMid);
  static TStr GetMainFNm(const TStr& NrFPath, const TStr& NrFMid);
  static TStr GetSegFNm(const TStr& NrFPath, const TStr& NrFMid, const int& SegN);
  void LoadMain(int& Segs);
  void SaveMain() const;
  TBlobBsStats Stats;
public:
  TMBlobBs(const TStr& BlobBsFNm, const TFAccess& _Access=faRdOnly,
   const int& _MxSegLen=-1);
  static PBlobBs New(const TStr& BlobBsFNm, const TFAccess& Access=faRdOnly,
   const int& MxSegLen=-1){
    return PBlobBs(new TMBlobBs(BlobBsFNm, Access, MxSegLen));}
  ~TMBlobBs();

  TMBlobBs& operator=(const TMBlobBs&){Fail; return *this;}

  TStr GetVersionStr() const {
    return TStr("Multiple-File Blob Base Format 1.0");}
  TBlobPt PutBlob(const PSIn& SIn);
  TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn);
  PSIn GetBlob(const TBlobPt& BlobPt);
  void DelBlob(const TBlobPt& BlobPt);

  TBlobPt GetFirstBlobPt();
  TBlobPt FFirstBlobPt();
  bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn);

  static bool Exists(const TStr& BlobBsFNm);

  const TBlobBsStats& GetStats();
  void ResetStats();
};

