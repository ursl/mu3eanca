//

#pragma once

#include <mu3e/util/math.hpp>
#include <mu3e/util/units.hpp>

#include <boost/property_tree/ptree.hpp>

#include <csignal>

namespace mu3e::rec {

struct conf_t {

    /**
     * Parse detector configuration.
     */
    void read_detector_conf(const boost::property_tree::ptree& ptree);
    void read_digi_conf(const boost::property_tree::ptree& ptree);
    void read_sort_conf(const boost::property_tree::ptree& ptree);

    void read_ptree(const boost::property_tree::ptree& ptree);

    /**
     * >= 0 - FATAL, ERROR
     * >= 1 - WARN
     * >= 2 - INFO
     * >= 3 - DEBUG
     */
    int verbose = 0;

    int test = 0;

    struct {
        int64_t frame = -1;
        volatile std::sig_atomic_t sigint_cnt = 0;

        std::string input, output;
        std::string input_mid;
        std::vector<std::string> confFiles;
        int n = -1, skip = 0;
    } runtime;

    struct {
        int mode = 0;

        std::string hitsFile;
        std::string truthFile;

        uint32_t n_hits_max = 96;
        uint32_t n_comb_max = 1024;
    } gpu;

    float B = -1 * mu3e::units::Tesla; // magnetic field
    float frameLength = 64; // [ns]

    struct {
        std::string sensors = "alignment/sensors";
        std::string fibres = "alignment/fibres";
        std::string mppcs = "alignment/mppcs";
        std::string tiles = "alignment/tiles";
    } alignment;

    struct {
      // bool useCDB = false;
      // std::string dbconn = "unset";
      bool useCDB = true;
      std::string dbconn = "/data/experiment/mu3e/code/cdb/json";
      std::string globalTag = "mcidealhead";
      //std::string globalTag = "mcidealv5.0";
    } conddb;
  
    float triplet_rt_min = 30; // [mm]
    float triplet_rt_max = 250; // [mm]

    float seg4_z01_max = 30; // [mm]
    float seg4_z12_max = 1.0;
    float seg4_phi01_max = 0.8f; // [radian]
    float seg4_phi12_max = 0.8f; // [radian]
    float seg4_dphi3_max = 0.05f; // [radian]
    float seg4_dz3_max = 5; // [mm]

    float seg6_dphi4_max = 0.35f;
    float seg6_dz4_max = 100;
    float seg6_dphi5_max = 0.035f;
    float seg6_dz5_max = 3;

    float L8o_dz4_max = 100;
    float L8o_dphi4_max = 0.35f;
    float L8i_dz4_max = 10;
    float L8i_dphi4_max = 0.2f;

    float seg3_chi2_max = 100;
    float seg4_chi2_max = 32; // = 8.0f * 2 * (4 - 2)
    float seg6_chi2_max = 48; // = 6.0f * 2 * (6 - 2)
    float seg8_chi2_max = 48; // = 4.0f * 2 * (8 - 2)

    int rec_version = 5;

    // rec fb, match mppc hits to S4
    // # 1 - use mppc hits
    // # 2 - use mppc asic hits
    int rec_fb = 1;
    // rec tl and match to seg6s
    int rec_tl = 1;

    int rec_vertex_xy = 0;

    // alg
    struct {
        int merge_mode = 1;

        int collapse_mode = 0;

        int resolve_mode = 1;
        int resolve_ordering = 1;
        int resolve_n_min = 3;
        int resolve_n_max = 0;
        int resolve_k_max = 0;
        int resolve_timeout_ms = 100;
    } alg;

    // root
    struct {
        // write extra info
        int ex = 0;
        // print status
        int status = 0;

        // write triplets
        int segs3 = 0;
        int segs3_gpu = 0;
        // write short 4-hit segments
        int segs4 = 0;
        int segs4_gpu = 0;
        // write long 6-hit segments
        int segs6 = 0;
        // write long 8-hit segments
        int segs8 = 0;

        int segs8_ = 0;

        // don't write empty frames
        int frames_skip_empty = 0;
    } root;

    // fb
    struct {
        float cluster_dt_max = 80; // [ns]
        float cluster_recl_dt = -4.5; // [ns], off if <= 0

        // 0 - simple (nh_min <= nh <= nh_max, d <= d_max)
        // 1 - phi_min/phi_max
        // 2 - advanced
        int link_method = 2;

        // only link clusters inside min/max number of hits
        int link_nh_min = 2;
        int link_nh_max = 128;
        // 4.0 * 0.250mm / 2.0 * 0.250mm for advanced method
        float link_d_max = 0.5; // [mm]
        // linking clusters from both sides , D/R = 0.250/60 = 0.0042
        float link_sides_tolerance = 0; // [radian]
        // tolerance in cols on the side of the cluster (link_method = 1)
        int link_col_tollerance = 3;

        // merge clusters from both sides
        bool merge_sides = false;

        // number of additional timestamp bits
        int n_fine_bits;
        // ibre cluster resolution (sigma)
        float resolution = 0.25; // ns
        // fibre daq delay in fine bins (50ps)
        int delay = 20;

        // Geometry
        bool square;
        float length;
        float diameter;
        float deadWidth;
        int nLayers;
        int nColumns;               ///< number of columns of fibre sipm arrays
        int nRibbons;               ///< number of ribbons
        float refidx;
        float thickness;
    } fb;

    // tl
    struct {
        int n_fine_bits;       ///< number of additional timestamp bits
        float resolution;           ///< tile hit resolution (sigma) of single tile hit

        // Geometry
        int nPhi;                   ///< number of tiles in phi-direction
        int nZ;                     ///< number of tiles in z-direction
        int nModules;               ///< number of modules in phi-direction

        float tile_l_z;             ///< [mm] tile dimension z direction

        float r_outer;              ///< tile detector outer radius
        float r_inner;              ///< tile detector inner radius
    } tl;

    // si
    struct {
        int n_fine_bits;       ///< number of additional timestamp bits
        float resolution;           ///< si hit resolution (sigma) of single tile hit
    } si;

    // expert
    struct {
        float noise = 0;
    } expert;

    // cosmic
    struct {
        int mode = 0;
        float p_min = 0;
        float p_max = 0;
    } cosmic;

    // twolayer
    struct {
        int mode = 0;

        float seg4_outer_chi2_max;
        float seg4_outer_z01_max;
        float seg4_outer_dphi01_max;
        float seg4_outer_dphi12_max;
        float seg4_outer_dphi23_max;
        float seg4_outer_dz2_max;
        float seg4_outer_dz3_max;
        float seg4_inner_chi2_max;
        float seg4_inner_z01_max;
        float seg4_inner_dphi01_max;
        float seg4_inner_dphi12_max;
        float seg4_inner_dphi23_max;
        float seg4_inner_dz2_max;
        float seg4_inner_dz3_max;
    } twolayer;

    // vertex
    struct {
        // target description
        float target_r = 19;
        float target_half_length = 50;
        float target_region = 25;

        // combinatorics cuts
        int n_tracks_max = 64;

        // margin when comparing rt to check for intersection of circles
        float circle_intersection_margin = 20;

        // minimum momentum of track
        float p_min = 0;//10; // [MeV]
        // max. momentum of track
        float p_max = 10000;//65 // [MeV]

        // signal selection cuts

        // min. total energy of all three tracks
        float E_tot_min = 0;//80;//90; // [MeV]
        // magnitude of 3-momentum
        float p_mag_max = 1000000;//30;//40; // [MeV]
        // max distance of vertex to target: outside
        float targetdist_max = 1000000;//1.5;//10; // [mm]
        float chi2_max = 10000000;//80;
        // max. cosine of opening angle between the two positron tracks
        float posprodcut = +1;//+0.99;
        // min. cosine of opening angle between positron and electron track
        float negprodcut = -1;//-0.99;
        // momentum difference between tracks
        float delta_p_min = 0;//1; // [MeV]

        float chi2_xy_max = 10000000;//3000;//40;
        float chi2_rz_max = 10000000;//5000;//70;
    } vertex;

    // Status of fibre detector (present or not).
    int fbDet = 0;

    // display
    struct {
        int mode = 0;
        float scale = 2;
        struct {
            int w = 900;
            int h = 900;
        } viewXY;
        struct {
            int w = 2700;
            int h = 900;
        } viewRZ;
        std::string file;
    } display;

};

// use mu3e::rec::conf object to access configs
extern conf_t conf;

} // namespace mu3e::rec

struct Conf {
    // [[deprecated(use mu3e::rec::conf)]]
    static mu3e::rec::conf_t& inst;
};
