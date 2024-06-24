TH1D* plotProductsVsTime(string what, string stype = "product") {
  TTree *t = (TTree*)gFile->Get("pdb");

  /* string id, type, tag; */
  /* int revision; */
  /* int pn, lot, item, lotsize, stocksize; */

  /* int nstatus; */
  /* string  stState[10]; */
  /* TDatime stTime[10]; */

  TTreeReader reader("pdb", gFile);
  TTreeReaderValue<int> vpn(reader,   "pn");
  TTreeReaderValue<string> vtype(reader, "type");
  TTreeReaderValue<string> vtag(reader,  "tag");
  TTreeReaderValue<std::vector<unsigned int>> vstTime(reader,  "stTime");

  TDatime T1(2017,  1, 1,0,0,0); double X1 = T1.Convert();
  TDatime T2(2024,  7,31,0,0,0); double X2 = T2.Convert();

  TH1D *h1 = new TH1D("h1", Form("%ss (%s)", stype.c_str(), what.c_str()), 60, X1, X2);
  setFilledHist(h1, kBlue, kYellow, 1000, 2);

  if (stype == "item") {
    setTitles(h1, "time", "number", 0.05, 0.8, 1.8);
  } else {
    setTitles(h1, "time", "number", 0.05, 0.8, 1.3);
  }
  h1->SetTitleSize(0.12, "t");
  h1->GetXaxis()->SetTimeDisplay(1);
  h1->GetXaxis()->SetTimeFormat("%m\/%y%F1970-01-01 00:00:00");
  h1->SetNdivisions(-205, "X");

  while (reader.Next()) {
    if ((what == "all") || (*vtag == what)) {
      if (*vtype == stype) {
        TDatime a = TDatime(vstTime->at(0));
        cout << "pn = " << *vpn << " type = " << *vtype << " tag = " << *vtag
             << " time = " << a.AsString()
             << endl;
        h1->Fill(a.Convert());
      }
    }
  }

  return h1;
}


// ----------------------------------------------------------------------
void plotParts(string stype = "product") {
  TH1D *hpix =  plotProductsVsTime("pix", stype);
  TH1D *htil =  plotProductsVsTime("sciti", stype);
  TH1D *hfib =  plotProductsVsTime("scifi", stype);
  TH1D *hdaq =  plotProductsVsTime("daq", stype);

  gStyle->SetOptStat(0);
  gStyle->SetTitleSize(0.08,"t");

  zone(2,2);

  shrinkPad(0.1, 0.17);
  hpix->Draw();
  TH1* hcpix = hpix->GetCumulative();
  setHist(hcpix, kRed, 20, 1., 3.);
  hcpix->Scale(1.05*hpix->GetMaximum()/hcpix->GetMaximum());
  hcpix->Draw("samehist");

  c0.cd(2);
  shrinkPad(0.1, 0.17);
  htil->Draw();
  TH1* hctil = htil->GetCumulative();
  setHist(hctil, kRed, 20, 1., 3.);
  hctil->Scale(1.05*htil->GetMaximum()/hctil->GetMaximum());
  hctil->Draw("samehist");

  c0.cd(3);
  shrinkPad(0.1, 0.17);
  hfib->Draw();
  TH1* hcfib = hfib->GetCumulative();
  setHist(hcfib, kRed, 20, 1., 3.);
  hcfib->Scale(1.05*hfib->GetMaximum()/hcfib->GetMaximum());
  hcfib->Draw("samehist");

  c0.cd(4);
  shrinkPad(0.1, 0.17);
  hdaq->Draw();
  TH1* hcdaq = hdaq->GetCumulative();
  setHist(hcdaq, kRed, 20, 1., 3.);
  hcdaq->Scale(1.05*hdaq->GetMaximum()/hcdaq->GetMaximum());
  hcdaq->Draw("samehist");

  c0.SaveAs(Form("%ss-vs-time.pdf", stype.c_str()));

  zone(1);
  shrinkPad(0.1, 0.17);
  TH1D *hall = plotProductsVsTime("all", stype);
  TH1* hcall = hall->GetCumulative();
  setHist(hcall, kRed, 20, 1., 3.);
  hcall->Scale(1.05*hall->GetMaximum()/hcall->GetMaximum());
  hall->Draw();
  hcall->Draw("samehist");

  c0.SaveAs(Form("all%ss-vs-time.pdf", stype.c_str()));

}


// ----------------------------------------------------------------------
void plotItemStatus(string stype = "item", string what = "all") {

  TTreeReader reader("pdb", gFile);
  TTreeReaderValue<int> vpn(reader,   "pn");
  TTreeReaderValue<string> vtype(reader, "type");
  TTreeReaderValue<string> vtag(reader,  "tag");
  TTreeReaderValue<std::vector<unsigned int>> vstTime(reader,  "stTime");
  TTreeReaderValue<std::vector<string>> vstState(reader,  "stState");

  TH1D *h1 = new TH1D("h1", Form("%ss (%s)", stype.c_str(), what.c_str()), 12, 0., 12.);
  setFilledHist(h1, kBlue, kYellow, 1000, 2);


  h1->GetXaxis()->SetBinLabel(1, "planned");
  h1->GetXaxis()->SetBinLabel(2, "ordered");
  h1->GetXaxis()->SetBinLabel(3, "inproduction");
  h1->GetXaxis()->SetBinLabel(4, "intransit");
  h1->GetXaxis()->SetBinLabel(5, "delivered");
  h1->GetXaxis()->SetBinLabel(6, "tested");
  h1->GetXaxis()->SetBinLabel(7, "available");
  h1->GetXaxis()->SetBinLabel(8, "used");
  h1->GetXaxis()->SetBinLabel(9, "cancelled");
  h1->GetXaxis()->SetBinLabel(10, "damaged");
  h1->GetXaxis()->SetBinLabel(11, "lost");

  if (stype == "item") {
    setTitles(h1, "status", "number", 0.04, 1.1, 1.8, 0.04);
  }


  while (reader.Next()) {
    if ((what == "all") || (*vtag == what)) {
      if (*vtype == stype) {
        TDatime a = TDatime(vstTime->at(0));
        string stat = vstState->at(0);
        cout << "pn = " << *vpn << " type = " << *vtype << " tag = " << *vtag
             << " time = " << a.AsString()
             << " state = " << stat
             << endl;
        for (unsigned int is = 0; is < vstState->size(); ++is) {
          for (int ibin = 1; ibin < h1->GetNbinsX(); ++ibin) {
            if (vstState->at(is) == h1->GetXaxis()->GetBinLabel(ibin)) h1->Fill(ibin-0.1);
          }
          cout << vstState->at(is) << " ";
        }
        cout << endl;
      }
    }
  }

  zone(1);
  gPad->SetLogy(1);
  gStyle->SetOptStat(0);
  shrinkPad(0.12, 0.17);
  h1->Draw();
  c0.SaveAs(Form("itemstatus-%s.pdf", what.c_str()));
}



// ----------------------------------------------------------------------
void plotTimeForStatusChange(string what = "all") {

  TTreeReader reader("pdb", gFile);
  TTreeReaderValue<int> vpn(reader,   "pn");
  TTreeReaderValue<string> vtype(reader, "type");
  TTreeReaderValue<string> vtag(reader,  "tag");
  TTreeReaderValue<string> vid(reader,  "id");
  TTreeReaderValue<std::vector<unsigned int>> vstTime(reader,  "stTime");
  TTreeReaderValue<std::vector<string>> vstState(reader,  "stState");

  TH1D *h12 = new TH1D("h12", Form("delivered - planned (%s)", what.c_str()), 50, 0., 500.);
  TH1D *h13 = new TH1D("h13", Form("tested - planned (%s)", what.c_str()), 50, 0., 500.);
  TH1D *h14 = new TH1D("h14", Form("available - planned (%s)", what.c_str()), 50, 0., 500.);
  TH1D *h15 = new TH1D("h15", Form("used - planned (%s)", what.c_str()), 50, 0., 500.);
  setFilledHist(h12, kBlack, kBlack, 0, 2);
  setFilledHist(h13, kRed, kRed, 3365, 2);
  setFilledHist(h14, kBlue, kBlue, 3354, 2);
  setFilledHist(h15, kBlue, kYellow, 1000, 2);

  setTitles(h12, "time [days]", "entries/bin", 0.04, 1.1, 1.8, 0.04);
  setTitles(h13, "time [days]", "entries/bin", 0.04, 1.1, 1.8, 0.04);
  setTitles(h14, "time [days]", "entries/bin", 0.04, 1.1, 1.8, 0.04);
  setTitles(h15, "time [days]", "entries/bin", 0.04, 1.1, 1.8, 0.04);

  while (reader.Next()) {
    if ((what == "all") || (*vtag == what)) {
      if (*vtype == "item") {
        TDatime a = TDatime(vstTime->at(0));
        string stat = vstState->at(0);
        if (0) cout << "pn = " << *vpn << " type = " << *vtype << " tag = " << *vtag
                    << " time = " << a.AsString()
                    << " state = " << stat
                    << " id = " << *vid
                    << endl;
        TDatime a0;
        for (unsigned int is = 0; is < vstState->size(); ++is) {
          TDatime a = TDatime(vstTime->at(is));
          if (0) cout << "  " << vstState->at(is) << " time: " << a.AsString() << endl;
          if ("planned" == vstState->at(is)) {
            a0 = TDatime(vstTime->at(is));
          }
        }
        bool used(true), tested(true), available(true), delivered(true);
        for (unsigned int js = 0; js < vstState->size(); ++js) {
          TDatime a1 = TDatime(vstTime->at(js));
          unsigned int deltaT = a1.Convert() - a0.Convert();
          if (delivered && vstState->at(js) == "delivered") {
            cout << "delta(t; delivered) = " << deltaT << " -> " << deltaT/86400 << endl;
            h12->Fill(deltaT/86400);
            delivered = false;
          }
          if (tested && vstState->at(js) == "tested") {
            cout << "delta(t; tested) = " << deltaT << " -> " << deltaT/86400 << endl;
            h13->Fill(deltaT/86400);
            tested = false;
          }
          if (available && vstState->at(js) == "available") {
            cout << "delta(t; available) = " << deltaT << " -> " << deltaT/86400 << endl;
            h14->Fill(deltaT/86400);
            available = false;
          }
          if (used && vstState->at(js) == "used") {
            cout << "delta(t; used) = " << deltaT << " -> " << deltaT/86400 << endl;
            h15->Fill(deltaT/86400);
            used = false;
          }
        }
      }
    }
  }
  zone(1);
  gPad->SetLogy(1);
  gStyle->SetOptStat(0);
  shrinkPad(0.12, 0.17);
  h12->SetTitle("");
  h12->Draw();
  h13->Draw("samehist");
  h14->Draw("samehist");
  h15->Draw("samehist");

  TLegend *leg = newLegend(0.4, 0.6, 0.85, 0.9);
  leg->SetHeader("time from #bf{planned} to ");
  leg->AddEntry(h12, "delivered", "l");
  leg->AddEntry(h13, "tested", "f");
  leg->AddEntry(h14, "available", "f");
  leg->AddEntry(h15, "used", "f");
  leg->Draw();

  c0.SaveAs(Form("deltaT-all-%s.pdf", what.c_str()));

}
