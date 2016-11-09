//root -l CompareSim2Meas.C
{
  
  TFile *file0 = TFile::Open("SimulatedMeasurement.Zscan.root");
  TFile *file1 = TFile::Open("output.root");
  
  TTree *trm=(TTree*) file0->Get("edge") ;
  TTree *trs=(TTree*) file1->Get("edge") ;
  
  trm->SetLineColor(1) ; trs->SetLineColor(2) ;  trm->SetLineWidth(2) ; trs->SetLineWidth(2) ;  
  trs->SetMarkerStyle(20) ;  trs->SetMarkerColor(2) ;  
  
  //Compare all in one go
  trm->Draw("volt:time","","l") ;
  trs->Draw("volt:time","","psame") ;
  gPad->Print("FitResult.pdf[");
  gPad->SetGrid(1);
  htemp->GetXaxis()->SetTitle("Time [ns]") ;   htemp->GetYaxis()->SetTitle("Signal [A]") ; 
  gPad->Print("FitResult.pdf");
  
  
  for (Int_t i=0 ; i<4;i++) {
    TString how=TString( Form("event==%d" , i)  ) ;
    trm->Draw("volt:time", how ,"l") ;
    trs->Draw("volt:time", how ,"lpsame") ;
    gPad->SetGrid(1);
    htemp->GetXaxis()->SetTitle("Time [ns]") ;   htemp->GetYaxis()->SetTitle("Signal [A]") ; 
    gPad->Print("FitResult.pdf");
  }   
  gPad->Print("FitResult.pdf]");
  
  
}
