/**
 * \author  A. Loreti
 * \date    2021-July
 * Studies of the delta_phi12 and delta_z12 distributions of hits at second innermost detector (layer 2)
 * when the particle is traveling outward and re-curling back.
 * updates:
 *   hesketh, Aug 2021: bug fix to require hit in layers before calculating dphi, dz
 */

#include <cmath>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <vector>

using namespace std;


/* USEFUL FUNCTIONS */
template<typename T>
inline T _wrap_periodic(T value, T start, T range)
{
    using std::floor;
    return value - range * floor((value - start) / range);
}

/** Calculate the equivalent angle in the [0, 2*pi) range */
template<typename T>
inline T radian_pos(T x) { return _wrap_periodic<T>(x, T(0), T(2*M_PI)); }


inline double Delta_PHI(double phi1, double phi2){
    double p1=radian_pos(phi1);
    double p0= radian_pos(phi2);
    double d_phi_ = p1 - p0;
    if(d_phi_>=M_PI)
      d_phi_ = 2*M_PI-d_phi_;
    if(d_phi_<=-M_PI)
      d_phi_ = -2*M_PI - d_phi_;
    return d_phi_;
}
/*end of USEFUL FUNTIONS*/



void Draw(TH1* plot){
   TCanvas* c = new TCanvas();
   plot->SetLineColor(kBlue);
   plot->SetFillColorAlpha(kGreen,0.50);;
   plot->SetFillStyle(3003);
   plot->DrawCopy();
   c->Print( (TString)(plot->GetName())+".pdf");
   delete c;
}



void two_layer_deltaDistrib() {

   // Input path and file names
   string path_ = "data/";
   string file_names[1] = {"mu3e_run_000001.root"};


   enum layer_type {
       LAYER_1 = 1,
       LAYER_2,
       LAYER_3,
       LAYER_4

   };

   // Histograms
   TFile * output = new TFile("plots.root", "RECREATE");
   TH1D* h_layer = new TH1D ("hit_distribution", "; layer ; entries", 11,0,10 );

   TH1D* h_dphi_L12 = new TH1D ("delta_phi12", "; #Delta #phi(1,2) ; entries", 181, -1.1*M_PI, 1.1*M_PI);
   TH1D* h_dphi_L23 = new TH1D ("delta_phi23", "; #Delta #phi(2,3) ; entries", 181, -1.1*M_PI, 1.1*M_PI);
   TH1D* h_dphi_L34 = new TH1D ("delta_phi34", "; #Delta #phi(3,4) ; entries", 181, -1.1*M_PI, 1.1*M_PI);

   TH1D *h_dz_L12 = new TH1D("delta_z12", "; #Delta z (1,2)" , 301, -150,150);
   TH1D *h_dz_L23 = new TH1D("delta_z23", "; #Delta z (2,3)" , 301, -150,150);
   TH1D *h_dz_L34 = new TH1D("delta_z34", "; #Delta z (3,4)" , 301, -150,150);

   // LOOP OVER ALL ROOT FILES
   for (auto name : file_names){
       string file_tmp = path_ + name;
       char file[file_tmp.size() +1];
       strcpy(file, file_tmp.c_str());

       cout << " File = " << file <<'\n';

       /// Input trees and file
       TFile* f0 = new TFile(file);
       TTree* t0 = (TTree*)f0->Get("mu3e");
       TTree* mchits = (TTree*)f0->Get("mu3e_mchits");

       ///Read TTree
       int Ntrajectories;
       vector<int>* traj_type = 0;
       vector<int>* traj_ID = 0;
       vector<int>* traj_mid = 0;
       vector<double>* traj_vx = 0;
       vector<double>* traj_vy = 0;
       vector<double>* traj_vz = 0;
       vector<double>* traj_px = 0;
       vector<double>* traj_py = 0;
       vector<double>* traj_pz = 0;
       //       vector<double>* traj_time = 0;
       t0->SetBranchAddress("Ntrajectories",&Ntrajectories);
       t0->SetBranchAddress("traj_type",&traj_type);
       t0->SetBranchAddress("traj_mother",&traj_mid);
       t0->SetBranchAddress("traj_ID",&traj_ID);
       //       t0->SetBranchAddress("traj_time",&traj_time);
       t0->SetBranchAddress("traj_vx",&traj_vx);
       t0->SetBranchAddress("traj_vy",&traj_vy);
       t0->SetBranchAddress("traj_vz",&traj_vz);
       t0->SetBranchAddress("traj_px",&traj_px);
       t0->SetBranchAddress("traj_py",&traj_py);
       t0->SetBranchAddress("traj_pz",&traj_pz);


       int nhits;
       vector<int>* hit_index =0;
       vector<int>* hit_pixelid =0;
       t0->SetBranchAddress("Nhit", &nhits);
       t0->SetBranchAddress("hit_mc_i", &hit_index);
       t0->SetBranchAddress("hit_pixelid", &hit_pixelid);

       int hid;
       int tid;
       double x,y,z;
       mchits->SetBranchAddress("hid", &hid);
       mchits->SetBranchAddress("tid", &tid);
       mchits->SetBranchAddress("pos_g_x", &x);
       mchits->SetBranchAddress("pos_g_y", &y);
       mchits->SetBranchAddress("pos_g_z", &z);

       // phi and z coordinates at the I and II layer of the detector.
       // a outward traj. b recurling traj.
       double phi[4], lx[4], ly[4], lz[4];


       for(unsigned int entry = 0; entry < t0->GetEntries(); entry++) {
         if(!(entry%1000))  cout << "Loaded entry " << entry << '\t'<< "of " << t0->GetEntries() << endl;
         t0->GetEntry(entry);

	 for (int i=0; i<Ntrajectories; i++){
	   /// cout << "(*traj_type)[i] = " << (*traj_type)[i] << endl;
	   /// get rid of particles formed at the chamber walls
	   if((*traj_type)[i] == 11 &&
	      sqrt((*traj_vx)[i]*(*traj_vx)[i] +(*traj_vy)[i]*(*traj_vy)[i]) < 400 ){

	     for(int ii=0; ii<4;ii++) phi[ii]=lx[ii]=ly[ii]=lz[ii]=-99;

	     for(int j=0; j<nhits; j++){
	       mchits->GetEntry((*hit_index)[j]);

	       if(tid == (*traj_ID)[i]){
		 //	 layers++;
		 // I LAYER
		 if((*hit_pixelid)[j]>> 26 == 0 ){
		   h_layer->Fill(LAYER_1);

		   // Outaward direction
		   if(hid==1){
		     lz[0]=z;
		     lx[0]=x;
		     ly[0]=y;
		     phi[0]=atan2(y,x);
		   }
		   // Recurling
		   if(hid==-4){
		     lz[3]=z;
		     lx[3]=x;
		     ly[3]=y;
		     phi[3]=atan2(y,x);
		   }
		 }


		 // II LAYER
		 if((*hit_pixelid)[j]>> 26 == 1 ){
		   h_layer->Fill(LAYER_2);

		   // Outward motion
		   if(hid==2){
		     lz[1]=z;
		     lx[1]=x;
		     ly[1]=y;
		     phi[1]=atan2(y,x);
		   }

		   // Recurling
		   if(hid==-3){
		     lz[2]=z;
		     lx[2]=x;
		     ly[2]=y;
		     phi[2]=atan2(y,x);
		   }
		 }


		 // III LAYER
		 if((*hit_pixelid)[j]>> 26 == 2 ){h_layer->Fill(LAYER_3);}

		 // IV LAYER
		 if((*hit_pixelid)[j]>> 26 == 3 ){h_layer->Fill(LAYER_4);}


	       }
	     }




		 // Calculates increments and fills histograms
		 double delta_phi12 = Delta_PHI(phi[0],phi[1]);
		 double delta_phi23 = Delta_PHI(phi[1],phi[2]);
		 double delta_phi34 = Delta_PHI(phi[2],phi[3]);

		 if(phi[0]>-99 && phi[1]>-99) {
		   double delta_phi = Delta_PHI(phi[0],phi[1]);
		   h_dphi_L12->Fill(delta_phi);
		   h_dz_L12->Fill(lz[0]-lz[1]);
		 }

		 if(phi[1]>-99 && phi[2]>-99) {
		   double delta_phi = Delta_PHI(phi[1],phi[2]);
		   h_dphi_L23->Fill(delta_phi);
		   h_dz_L23->Fill(lz[1]-lz[2]);
		 }

		 if(phi[2]>-99 && phi[3]>-99) {
		   double delta_phi = Delta_PHI(phi[2],phi[3]);
		   h_dphi_L34->Fill(delta_phi);
		   h_dz_L34->Fill(lz[2]-lz[3]);
		 }

		 //	     if(print) cout<<"Nhits: "<<layers<<endl;



	   }
         }
       }
   }







   // 1,2 LAYER
   Draw(h_dphi_L12);
   Draw(h_dphi_L23);
   Draw(h_dphi_L34);

   Draw(h_dz_L12);
   Draw(h_dz_L23);
   Draw(h_dz_L34);

   Draw(h_layer);


   output->Write();

   return;
}



void Distributions() {
  two_layer_deltaDistrib();
}
