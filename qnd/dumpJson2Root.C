void dumpJson2Root() {
  string test("This is a string");
  
  TFile *f = TFile::Open("root.root", "RECREATE");
  f->mkdir("json");
  f->cd("json");
  TNamed o("payload", test.c_str());
  o.Write();
  f->Close();
}
