// ----------------------------------------------------------------------
void csv(string filename = "sensors.csv") {
  TTree *t = (TTree*)gFile->Get("alignment/sensors");
  double vx, vy, vz;
  double rowx, rowy, rowz;
  double colx, coly, colz;
  UInt_t sensor;
  Int_t  nrow, ncol;
  double width, length, thickness, pixelSize;
  
  t->SetBranchAddress("sensor", &sensor);

  t->SetBranchAddress("vx", &vx);
  t->SetBranchAddress("vy", &vy);
  t->SetBranchAddress("vz", &vz);

  t->SetBranchAddress("rowx", &rowx);
  t->SetBranchAddress("rowy", &rowy);
  t->SetBranchAddress("rowz", &rowz);

  t->SetBranchAddress("colx", &colx);
  t->SetBranchAddress("coly", &coly);
  t->SetBranchAddress("colz", &colz);

  t->SetBranchAddress("nrow", &nrow);
  t->SetBranchAddress("ncol", &ncol);
  t->SetBranchAddress("width", &width);
  t->SetBranchAddress("length", &length);
  t->SetBranchAddress("thickness", &thickness);
  t->SetBranchAddress("pixelSize", &pixelSize);

  ofstream ONS;
  ONS.open(filename);
  
  for (unsigned int i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);
    cout << "sensor = " << sensor
         << " v x/y/z = " << vx << "/" << vy << "/" << vz
         << " row x/y/z = " << rowx << "/" << rowy << "/" << rowz
         << " col x/y/z = " << colx << "/" << coly << "/" << colz
         << " rest: " << nrow << "/" << ncol << "/" << width
         << "/" << length << "/" << thickness << "/" << pixelSize
         << endl;
    ONS << sensor << ","
        << vx << ","
        << vy << ","
        << vz << ","
        << rowx << ","
        << rowy << ","
        << rowz << ","
        << colx << ","
        << coly << ","
        << colz << ","
        << nrow << ","
        << ncol << ","
        << width << ","
        << length << ","
        << thickness << ","
        << pixelSize
        << endl;
  }

}
