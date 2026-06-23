#include <dirent.h>
#include <iostream>
#include <vector>
#include <string>

// root [0] .L anaSkippedHeader.C
// root [1] anaSkippedHeader(5592, 6244, "TS")
// root [1] anaSkippedHeader(3000, 6300, "sig")

using namespace::std;

// ----------------------------------------------------------------------
void anaSkippedHeader(int minRun = 3000, int maxRun = 9600, string suffix = "all") {

  // -- get all files
  vector<string> vfiles;
  DIR *folder = opendir("/Users/ursl/data/mu3e/run2025/rawv2");
  if (folder == NULL) {
    cout << "Error failed to open /Users/ursl/data/mu3e/run2025/rawv2" << endl;
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(folder))) {
    if (entry->d_type == DT_REG) {
      vfiles.push_back(string("/Users/ursl/data/mu3e/run2025/rawv2/") + string(entry->d_name));
    }
  }
  closedir(folder);
  for (auto &file : vfiles) {
    cout << "file = " << file << endl;
  }


  vector<int> vbadRuns = {6277,6252,6179,6031,5959,4755,4622,4621,4612,4611,4601,4600,4599,4598,4597,4596,4595,4594,4593,4592,4576,4495,4494,3823,3803,3802,3800,3658,3656,3650,3648,3643,3623,3571,3489,3485,3481,3473,3469,3310,3309,3031,3030,3029,3028,3027,3023,3017,3014,3013,3009,3008};

  vector<int> vTSRuns = {6244,6243,6242,6241,6239,6237,6236,6235,6234,6233,6232,6231,6230,6229,6225,6224,6220,6218,6216,6215,6214,6213,6212,6211,6209,6208,6207,6206,6205,6203,6202,6200,6198,6197,6196,6195,6193,6192,6186,6185,6184,6183,6182,6181,6180,6178,6177,6176,6174,6173,6172,6171,6170,6169,6166,6165,6164,6163,6162,6115,6114,6113,6112,6111,6110,6109,6108,6107,6106,6105,6104,6103,6102,6101,6100,6099,6097,6096,6095,6094,6093,6092,6091,6090,6089,6088,6087,6086,6085,6084,6083,6082,6081,6080,6079,6077,6076,6074,6073,6072,6071,6070,6069,6068,6067,6066,6065,6064,6063,6061,6060,6059,6058,6057,6056,6055,6054,6053,6052,6050,6049,6048,6047,6046,6040,6039,6038,6037,6036,6035,6034,6033,6032,6029,6022,6021,6020,6019,6014,6012,6010,6008,6006,6005,6004,6003,6002,6001,6000,5999,5998,5996,5995,5994,5993,5992,5991,5990,5988,5987,5986,5985,5984,5983,5982,5981,5980,5979,5978,5977,5976,5974,5973,5972,5971,5970,5969,5967,5966,5965,5964,5962,5961,5960,5949,5948,5947,5946,5945,5944,5943,5942,5941,5940,5939,5938,5937,5936,5935,5934,5933,5932,5931,5930,5929,5928,5927,5926,5925,5924,5922,5921,5920,5918,5917,5916,5915,5914,5913,5912,5911,5910,5909,5907,5906,5905,5904,5903,5902,5901,5834,5812,5811,5810,5809,5808,5807,5806,5805,5804,5803,5802,5801,5800,5799,5798,5797,5796,5795,5794,5793,5792,5791,5790,5789,5788,5787,5786,5785,5784,5783,5782,5781,5780,5779,5778,5777,5776,5775,5774,5773,5772,5771,5770,5769,5768,5767,5766,5765,5764,5763,5762,5761,5760,5759,5758,5757,5756,5755,5754,5753,5752,5751,5750,5749,5748,5747,5746,5745,5739,5738,5737,5736,5735,5734,5733,5732,5731,5730,5729,5728,5727,5726,5725,5724,5723,5722,5720,5719,5718,5717,5716,5715,5714,5713,5712,5711,5710,5709,5708,5707,5706,5705,5704,5703,5702,5701,5700,5699,5698,5697,5696,5695,5694,5693,5692,5691,5690,5689,5688,5686,5685,5684,5683,5682,5681,5680,5679,5678,5677,5676,5675,5674,5673,5672,5671,5670,5669,5668,5667,5666,5665,5664,5663,5662,5661,5659,5658,5652,5651,5650,5649,5647,5645,5644,5643,5642,5641,5640,5638,5637,5636,5635,5634,5633,5632,5631,5629,5628,5627,5626,5625,5624,5623,5622,5620,5619,5618,5617,5616,5613,5612,5611,5610,5609,5608,5607,5606,5605,5604,5600,5599,5598,5597,5596,5595,5594,5592
  };

  vector<int> vSigRuns ={6300,6299,6298,6297,6296,6295,6294,6293,6292,6291,6290,6289,6288,6287,6286,6285,6284,6283,6282,6281,6280,6279,6278,6277,6276,6275,6274,6273,6272,6271,6270,6269,6268,6267,6266,6265,6264,6263,6262,6261,6260,6259,6258,6257,6256,6255,6254,6253,6252,6244,6243,6242,6241,6240,6239,6238,6237,6236,6235,6234,6233,6232,6231,6230,6229,6228,6227,6226,6225,6224,6223,6222,6221,6220,6219,6218,6217,6216,6215,6214,6213,6212,6211,6210,6209,6208,6207,6206,6205,6204,6203,6202,6201,6200,6199,6198,6197,6196,6195,6194,6193,6192,6191,6190,6189,6188,6187,6186,6185,6184,6183,6182,6181,6180,6179,6178,6177,6176,6175,6174,6173,6172,6171,6170,6169,6166,6165,6164,6163,6162,6161,6160,6159,6158,6157,6116,6115,6114,6113,6112,6111,6110,6109,6108,6107,6106,6105,6104,6103,6102,6101,6100,6099,6098,6097,6096,6095,6094,6093,6092,6091,6090,6089,6088,6087,6086,6085,6084,6083,6082,6081,6080,6079,6078,6077,6076,6075,6074,6073,6072,6071,6070,6069,6068,6067,6066,6065,6064,6063,6062,6061,6060,6059,6058,6057,6056,6055,6054,6053,6052,6051,6050,6049,6048,6047,6046,6040,6039,6038,6037,6036,6035,6034,6033,6032,6031,6030,6029,6028,6027,6026,6025,6024,6023,6022,6021,6020,6019,6018,6017,6016,6015,6014,6013,6012,6011,6010,6009,6008,6007,6006,6005,6004,6003,6002,6001,6000,5999,5998,5997,5996,5995,5994,5993,5992,5991,5990,5989,5988,5987,5986,5985,5984,5983,5982,5981,5980,5979,5978,5977,5976,5975,5974,5973,5972,5971,5970,5969,5968,5967,5966,5965,5964,5963,5962,5961,5960,5959,5958,5957,5956,5955,5954,5953,5950,5949,5948,5947,5946,5945,5944,5943,5942,5941,5940,5939,5938,5937,5936,5935,5934,5933,5932,5931,5930,5929,5928,5927,5926,5925,5924,5923,5922,5921,5920,5919,5918,5917,5916,5915,5914,5913,5912,5911,5910,5909,5908,5907,5906,5905,5904,5903,5902,5901,5844,5843,5842,5841,5840,5839,5838,5837,5836,5835,5834,5813,5812,5811,5810,5809,5808,5807,5806,5805,5804,5803,5802,5801,5800,5799,5798,5797,5796,5795,5794,5793,5792,5791,5790,5789,5788,5787,5786,5785,5784,5783,5782,5781,5780,5779,5778,5777,5776,5775,5774,5773,5772,5771,5770,5769,5768,5767,5766,5765,5764,5763,5762,5761,5760,5759,5758,5757,5756,5755,5754,5753,5752,5751,5750,5749,5748,5747,5746,5745,5739,5738,5737,5736,5735,5734,5733,5732,5731,5730,5729,5728,5727,5726,5725,5724,5723,5722,5721,5720,5719,5718,5717,5716,5715,5714,5713,5712,5711,5710,5709,5708,5707,5706,5705,5704,5703,5702,5701,5700,5699,5698,5697,5696,5695,5694,5693,5692,5691,5690,5689,5688,5687,5686,5685,5684,5683,5682,5681,5680,5679,5678,5677,5676,5675,5674,5673,5672,5671,5670,5669,5668,5667,5666,5665,5664,5663,5662,5661,5660,5659,5658,5652,5651,5650,5649,5648,5647,5646,5645,5644,5643,5642,5641,5640,5639,5638,5637,5636,5635,5634,5633,5632,5631,5630,5629,5628,5627,5626,5625,5624,5623,5622,5621,5620,5619,5618,5617,5616,5615,5614,5613,5612,5611,5610,5609,5608,5607,5606,5605,5604,5603,5602,5601,5600,5599,5598,5597,5596,5595,5594,5593,5592,5106,5103,5102,4912,4910,4909,4908,4906,4905,4901,4900,4899,4898,4897,4896,4895,4894,4893,4892,4891,4890,4889,4888,4887,4886,4885,4884,4883,4882,4881,4880,4879,4878,4877,4876,4875,4874,4873,4872,4871,4870,4869,4868,4867,4866,4865,4864,4863,4823,4822,4821,4820,4819,4818,4816,4814,4812,4811,4810,4809,4807,4806,4805,4804,4803,4802,4801,4800,4799,4797,4795,4794,4793,4792,4791,4790,4789,4788,4787,4786,4785,4784,4783,4782,4780,4779,4776,4775,4774,4773,4771,4770,4769,4768,4767,4765,4764,4763,4760,4759,4758,4757,4756,4755,4749,4748,4747,4746,4745,4744,4743,4742,4740,4739,4731,4729,4728,4727,4726,4725,4724,4723,4722,4721,4719,4718,4715,4714,4712,4708,4707,4706,4705,4703,4702,4701,4700,4699,4698,4697,4696,4694,4693,4692,4691,4690,4689,4688,4687,4686,4685,4684,4683,4681,4680,4679,4678,4677,4676,4675,4672,4671,4670,4669,4668,4667,4665,4664,4663,4662,4661,4660,4659,4658,4657,4655,4653,4652,4651,4648,4647,4644,4643,4642,4638,4637,4636,4635,4634,4631,4630,4629,4626,4625,4624,4623,4622,4621,4620,4619,4618,4617,4616,4615,4614,4613,4612,4611,4610,4609,4608,4607,4606,4605,4604,4603,4602,4601,4600,4599,4598,4597,4596,4595,4594,4593,4592,4591,4590,4589,4588,4587,4586,4585,4584,4583,4582,4581,4580,4579,4578,4577,4576,4575,4574,4573,4572,4571,4570,4569,4568,4567,4566,4565,4564,4563,4562,4561,4560,4559,4558,4557,4555,4554,4553,4552,4551,4550,4549,4548,4547,4546,4545,4544,4543,4542,4541,4540,4539,4538,4537,4536,4535,4534,4532,4531,4530,4529,4528,4527,4526,4525,4524,4522,4520,4519,4514,4513,4512,4511,4510,4509,4508,4507,4506,4505,4504,4503,4502,4501,4500,4499,4498,4497,4496,4495,4494,4493,4492,4491,4490,3823,3822,3820,3819,3818,3817,3816,3815,3814,3813,3812,3803,3802,3801,3800,3799,3798,3797,3796,3795,3794,3658,3657,3656,3655,3654,3653,3652,3651,3650,3649,3648,3647,3646,3645,3644,3643,3642,3641,3640,3638,3637,3636,3635,3634,3633,3630,3629,3628,3627,3626,3625,3624,3623,3622,3621,3620,3619,3618,3606,3604,3603,3602,3587,3586,3585,3584,3583,3582,3581,3578,3577,3576,3575,3574,3573,3572,3571,3570,3569,3568,3567,3523,3521,3520,3518,3517,3516,3514,3512,3510,3509,3508,3506,3505,3504,3503,3502,3501,3500,3499,3498,3497,3496,3495,3494,3493,3492,3491,3490,3489,3488,3487,3486,3485,3484,3483,3482,3481,3477,3476,3475,3474,3473,3472,3471,3470,3469,3468,3467,3466,3465,3463,3461,3457,3452,3450,3449,3447,3446,3445,3444,3443,3442,3441,3440,3439,3438,3437,3433,3432,3431,3430,3429,3428,3426,3424,3422,3420,3419,3418,3417,3416,3415,3413,3412,3411,3410,3409,3408,3407,3406,3404,3403,3402,3400,3399,3398,3395,3394,3393,3392,3391,3385,3384,3382,3381,3380,3379,3378,3376,3375,3374,3370,3369,3368,3367,3366,3361,3356,3347,3346,3345,3344,3339,3333,3332,3326,3316,3315,3314,3313,3312,3311,3310,3309,3308,3307,3306,3305,3304,3303,3302,3301,3299,3296,3295,3294,3104,3102,3101,3097,3092,3031,3030,3029,3028,3027,3025,3024,3023,3017,3015,3014,3013,3012,3011,3010,3009,3008};

  vector<string> vRunNumbers;
  stringstream ssRunNumbersHeader;
  ssRunNumbersHeader << "run," << " goodPixelDataFraction," 
                    << " goodPixelDataUnsortedFraction," << " goodPixelDataStart," 
                    << " goodPixelDataEnd," << " goodPixelDataUnsorted," 
                    << " lastFrame";
  vRunNumbers.push_back(ssRunNumbersHeader.str());

  // -- read each file
  TH1D *hgoodPixelData = new TH1D("hgoodPixelData", "good pixel data fraction", 102, -0.01, 1.01);
  TH1D *hgoodPixelDataUnsortedFraction = new TH1D("hgoodPixelDataUnsortedFraction", "good pixel data fraction: one pixel FEB unsorted", 102, -0.01, 1.01);
  TH1D *hgoodPixelDataVsRunNumber = new TH1D("hgoodPixelDataVsRunNumber", "good pixel data  fraction (vs run number)", maxRun - minRun, minRun, maxRun);
  hgoodPixelDataVsRunNumber->SetMarkerStyle(24);
  hgoodPixelDataVsRunNumber->SetMarkerSize(0.5);
  TH1D *hgoodPixelDataUnsortedFractionVsRunNumber = new TH1D("hgoodPixelDataUnsortedFractionVsRunNumber", "good pixel data fraction: one pixel FEB unsorted (vs run number)", maxRun - minRun, minRun, maxRun);
  hgoodPixelDataUnsortedFractionVsRunNumber->SetMarkerStyle(24);
  hgoodPixelDataUnsortedFractionVsRunNumber->SetMarkerSize(0.5);

  int nruns(0);
  for (auto &file : vfiles) {
    cout << "reading file = " << file << endl;
    int runnumber = ::stoi(file.substr(file.rfind("run") + 3, 5));
    cout << "runnumber = " << runnumber << endl;
    if (runnumber < minRun || runnumber > maxRun) continue;
    if (0 && find(vbadRuns.begin(), vbadRuns.end(), runnumber) != vbadRuns.end()) continue;
    if (suffix == "TS" && find(vTSRuns.begin(), vTSRuns.end(), runnumber) == vTSRuns.end()) continue;
    if (suffix == "sig" && find(vSigRuns.begin(), vSigRuns.end(), runnumber) == vSigRuns.end()) continue;


    ifstream INS;
    INS.open(file.c_str());
    string line;
    long long goodPixelDataStart(0), goodPixelDataEnd(0), goodPixelDataUnsorted(0), lastFrame(0);
    while (getline(INS, line)) {
      if (line.find("first_frame_at_least_one_pixel_FEB_had_unsorted_hit_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " unsorted ->" << sframe << "<- " << endl;
        goodPixelDataUnsorted = ::stol(sframe);
        // int iframe = ::stoi(sframe);
        // cout << "iframe = " << iframe << endl;
      }
      if (line.find("last_frame_of_the_run") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " lastframe ->" << sframe << "<- " << endl;
        lastFrame = ::stol(sframe);
      }
      if (line.find("start_frame_good_pixel_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " start ->" << sframe << "<- " << endl;
        goodPixelDataStart = ::stol(sframe);
      }
      if (line.find("end_frame_good_pixel_data") != string::npos) {
        size_t pos = line.rfind(":");
        string sframe = line.substr(pos+2, line.size() - pos - 3);
        cout << " endpixel ->" << sframe << "<- " << endl;
        goodPixelDataEnd = ::stol(sframe);
      }
    }
    if (lastFrame < 0) continue;
    if (goodPixelDataEnd > lastFrame) continue;

    if (goodPixelDataEnd < 0) goodPixelDataEnd = lastFrame;
    if (goodPixelDataStart < 0) goodPixelDataStart = 0;
    if (goodPixelDataUnsorted < 0) goodPixelDataUnsorted = lastFrame;

    double goodPixelDataFraction = (goodPixelDataEnd - goodPixelDataStart + 1.) / (lastFrame - goodPixelDataStart + 1.);
    cout << "goodPixelDataFraction = " << goodPixelDataFraction << " " 
    << goodPixelDataStart << " "
    << goodPixelDataEnd << " "
    << lastFrame << " "
    << goodPixelDataFraction << endl;
    // -- catch numerical instabilities
    if (goodPixelDataFraction < 1.e-3) goodPixelDataFraction = 0.002;
    hgoodPixelData->Fill(goodPixelDataFraction);

    int ibin = hgoodPixelDataVsRunNumber->FindBin(runnumber);
    hgoodPixelDataVsRunNumber->SetBinContent(ibin, goodPixelDataFraction);


    double goodPixelDataUnsortedFraction = (goodPixelDataUnsorted - goodPixelDataStart + 1.) / (lastFrame - goodPixelDataStart + 1.);
    // -- catch numerical instabilities
    if (goodPixelDataUnsortedFraction < 1.e-3) goodPixelDataUnsortedFraction = 0.002;
    cout << "goodPixelDataUnsortedFraction = " << goodPixelDataUnsortedFraction << endl;
    hgoodPixelDataUnsortedFraction->Fill(goodPixelDataUnsortedFraction);
   
    ibin = hgoodPixelDataUnsortedFractionVsRunNumber->FindBin(runnumber);
    hgoodPixelDataUnsortedFractionVsRunNumber->SetBinContent(ibin, goodPixelDataUnsortedFraction);
   
    INS.close();
    stringstream ss;
    ss << runnumber << ", " << goodPixelDataFraction << ", " << goodPixelDataUnsortedFraction 
       << ", " << goodPixelDataStart << ", " << goodPixelDataEnd << ", " << goodPixelDataUnsorted << ", " << lastFrame;
    vRunNumbers.push_back(ss.str());
    nruns++;
  }

  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 1000);
  c1->SetLogy(0);
  hgoodPixelData->Draw("hist");
  tl->SetTextSize(0.03);
  tl->DrawLatexNDC(0.2, 0.8, Form("runMin = %d, runMax = %d", minRun, maxRun));
  tl->DrawLatexNDC(0.2, 0.7, Form("mean: %5.2f", hgoodPixelData->GetMean()));
  c1->SaveAs(Form("hgoodPixelData-%d-%d-%s.pdf", minRun, maxRun, suffix.c_str()));
  c1->SaveAs(Form("hgoodPixelData-%d-%d-%s.png", minRun, maxRun, suffix.c_str()));
 
  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataUnsortedFraction->Draw("hist");
  tl->SetTextSize(0.03);
  tl->DrawLatexNDC(0.2, 0.8, Form("runMin = %d, runMax = %d", minRun, maxRun));
  tl->DrawLatexNDC(0.2, 0.7, Form("mean: %5.2f", hgoodPixelDataUnsortedFraction->GetMean()));
  c1->SaveAs(Form("hgoodPixelDataUnsortedFraction-%d-%d-%s.pdf", minRun, maxRun, suffix.c_str()));
  c1->SaveAs(Form("hgoodPixelDataUnsortedFraction-%d-%d-%s.png", minRun, maxRun, suffix.c_str()));

  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataVsRunNumber->SetMinimum(0.001);
  hgoodPixelDataVsRunNumber->Draw("p");
  c1->SaveAs(Form("hgoodPixelDataFractionVsRunNumber-%d-%d-%s.pdf", minRun, maxRun, suffix.c_str()));
  c1->SaveAs(Form("hgoodPixelDataFractionVsRunNumber-%d-%d-%s.png", minRun, maxRun, suffix.c_str()));

  c1->Clear();
  c1->SetLogy(0);
  hgoodPixelDataUnsortedFractionVsRunNumber->SetMinimum(0.001);
  hgoodPixelDataUnsortedFractionVsRunNumber->Draw("p");
  c1->SaveAs(Form("hgoodPixelDataUnsortedFractionVsRunNumber-%d-%d-%s.pdf", minRun, maxRun, suffix.c_str()));
  c1->SaveAs(Form("hgoodPixelDataUnsortedFractionVsRunNumber-%d-%d-%s.png", minRun, maxRun, suffix.c_str()));
  delete c1;
  delete hgoodPixelData;
  delete hgoodPixelDataUnsortedFraction;
  delete hgoodPixelDataVsRunNumber;
  delete hgoodPixelDataUnsortedFractionVsRunNumber;
  cout << "nruns = " << nruns << endl;


  sort(vRunNumbers.begin()+1, vRunNumbers.end());
  ofstream outfile(Form("runNumbers-%d-%d-%s.txt", minRun, maxRun, suffix.c_str()));
  for (auto &runnumber : vRunNumbers) {
    outfile << runnumber << endl;
  }
  outfile.close();

}


// ----------------------------------------------------------------------
void runAll() {
  anaSkippedHeader(3000, 9600);
  anaSkippedHeader(3000, 6000);
  anaSkippedHeader(7000, 9600);
}