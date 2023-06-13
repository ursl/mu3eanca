// ----------------------------------------------------------------------
void csv(string filename = "sensors.csv") {
  TTree *t = (TTree*)gFile->Get("alignment/sensors");
  double vx, vy, vz;
  UInt_t sensor;
  t->SetBranchAddress("vx", &vx);
  t->SetBranchAddress("vy", &vy);
  t->SetBranchAddress("vz", &vz);
  t->SetBranchAddress("sensor", &sensor);

  ofstream ONS;
  ONS.open(filename);
  
  for (unsigned int i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);
    cout << "sensor = " << sensor
         << " vx/vy/vz = " << vx << "/" << vy << "/" << vz
         << endl;
    ONS << sensor << ","
        << vx << ","
        << vy << ","
        << vz << ","
        << "0.0,"
        << "0.0,"
        << "0.0,"
        << endl;
  }

}
