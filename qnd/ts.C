// ----------------------------------------------------------------------
void tsSort(vector<unsigned int> *vPixelID, vector<unsigned int> *vTimeStamp) {
  
  cout << "--------------------" << endl;
  cout << "original ts vector: " << endl;
  for (unsigned int i = 0; i < vTimeStamp->size(); ++i) {
    cout << vTimeStamp->at(i) << " "; 
  }
  cout << endl;
  cout << "original id vector: " << endl;
  for (unsigned int i = 0; i < vPixelID->size(); ++i) {
    cout << vPixelID->at(i) << " "; 
  }
  cout << endl;

  // -- sort the two vectors such that timestamp is strictly increasing
  vector<unsigned int> vid, vts;
  vid.push_back(vPixelID->at(0)); 
  vts.push_back(vTimeStamp->at(0)); 
  for (unsigned int ihit = 1; ihit < vTimeStamp->size(); ++ihit) {
    for (unsigned int i = 0; i < vts.size(); ++i) {
      if ((vTimeStamp->at(ihit) < vts[i]) && (i < vts.size())) {
	cout << "inserting ->" << vTimeStamp->at(ihit) << "<- before i = " << i << " with ts = " << vts[i] << endl;
	auto it = vts.insert(vts.begin()+i, vTimeStamp->at(ihit));
	auto iu = vid.insert(vid.begin()+i, vPixelID->at(ihit));
	break;	
      }
      if (i+1 == vts.size()) {
	cout << "inserting ->" << vTimeStamp->at(ihit) << "<- at end i = " << i << " with ts = " << vts[i] << endl;
	vts.push_back(vTimeStamp->at(ihit));
	vid.push_back(vPixelID->at(ihit));
	break;	
      }
    }
  }

  cout << "sorted ts vector: " << endl;
  for (unsigned int i = 0; i < vts.size(); ++i) {
    cout << vts.at(i) << " "; 
  }
  cout << endl;

  cout << "sorted id vector: " << endl;
  for (unsigned int i = 0; i < vid.size(); ++i) {
    cout << vid.at(i) << " "; 
  }
  cout << endl;

}


// ----------------------------------------------------------------------
void tstest() {
  vector<unsigned int> vid, vts;
  // -- first: all in order
  vts.push_back(1501);  vid.push_back(1123);
  vts.push_back(1504);  vid.push_back(1132);
  vts.push_back(1507);  vid.push_back(1213);

  tsSort(&vid, &vts);

  // -- next: add one out of time 
  vts.push_back(1505);  vid.push_back(1312);
  tsSort(&vid, &vts);

  // -- next:  Add same out of time again
  vts.push_back(1505);  vid.push_back(1302);
  tsSort(&vid, &vts);

  // -- next:  Add same more
  vts.push_back(1502);  vid.push_back(1302);
  vts.push_back(1512);  vid.push_back(1302);
  vts.push_back(1522);  vid.push_back(1302);
  vts.push_back(1524);  vid.push_back(1302);
  tsSort(&vid, &vts);
  
  // -- next:  Add same more
  vts.push_back(1509);  vid.push_back(1309);
  vts.push_back(1508);  vid.push_back(1308);
  vts.push_back(1507);  vid.push_back(1307);
  vts.push_back(1506);  vid.push_back(1306);
  tsSort(&vid, &vts);

  
  
}
