//

#pragma once

#include "vars.h"

#include <fmt/printf.h>

namespace mu3e::root {

struct tree_t {
    TTree* tree;
    mu3e::util::var_t::list_t vars;

    explicit
    tree_t(TTree* tree = nullptr) : tree(tree) {
    }

    tree_t(TDirectory* dir, const char* name) {
        tree = new TTree(name, name);
        tree->SetDirectory(dir);
        //tree->SetAutoFlush(-128 * 1024 * 1024);
        //tree->SetAutoSave(-1024 * 1024 * 1024);
    }

    void set_branch_address() {
        for(auto& var : vars) {
            if(!var->set_branch_address(tree)) {
                fmt::print("W [{}] no branch '{}'\n", __FUNCTION__, var->name);
            }
        }
    }
};

struct alignment_sensors_t : tree_t {
    uint32_t M_VARS("", (id));
    double M_VARS("", (vx)(vy)(vz));
    double M_VARS("", (rowx)(rowy)(rowz));
    double M_VARS("", (colx)(coly)(colz));
    int M_VARS("", (nrow)(ncol));
    double M_VARS("", (width)(length)(thickness)(pixelSize));

    // surface deformation parameters
    double M_VARS("", (c20)(c21)(c22)(c30)(c31)(c32)(c33)(dTu)(dTv));

    explicit
    alignment_sensors_t(TTree* tree = nullptr) : tree_t(tree) {
        for(auto& var : vars) {
            var->clear();
        }
        dTu = dTv = 1;

        // fallback to deprecated branches
        if(tree && tree->GetBranch("id") == nullptr) var_T_id.name = "sensor";
    }

    void print() {
        fmt::print("sensors:\n");
        for(Long64_t entry = 0; entry < tree->GetEntries(); entry++) {
            tree->GetEntry(entry);
            fmt::print("id = {:08X}\n", id);
        }
    }

};

struct rec_frames_t : tree_t {
    int M_VARS("", (runId));

    // reconstructed values
    std::vector<double> M_VARS("",
        // coordinates of hit 0 (hit in first layer)
        (x0)(y0)(z0)
        // time estimate and uncertainty (sigma) of hit 0
        (t0)(t0_err)
        // time of hit 0 from tiles, fibres and pixels
        (t0_tl)(t0_fb)(t0_si)
        // largest time difference between timing hits (tiles and fibres) and si hits
        (dt)(dt_si)
        (r)(rerr2)(p)(perr2)(chi2)
        // tangent (xy and rz) at hit 0
        (tan01)(lam01)
        (n_shared_hits)(n_shared_segs)
    );
    std::vector<int32_t> M_VARS("",
        (nhit) // number of hits
    );
    std::vector<uint32_t> M_VARS("",
        (sid0) // sensor id of hit 0
    );

    // number of segments (`n` entries and subtypes)
    int M_VARS("", (n)(n3)(n4)(n6)(n8));

    // MC (`mc_*`) variables
    //
    // - in data
    //     - all MC vars are zero (or equivalent to zero)
    // - in MC
    //     - fake tracks have `mc` and `mc_prime` set to zero
    //     - tracks assembled from hits of same particle
    //       will have `mc_` vars set to values of that track


    int M_VARS("", (eventId));
    double M_VARS("", (weight));

    std::vector<int32_t> M_VARS("",
        // is truth particle and is first segment
        (mc)(mc_prime)
        // track id and mother track id
        (mc_tid)(mc_mid)
        // particle PDG code and type (see Mu3eTrajectory::getPType and getDType)
        (mc_pid)(mc_type)
    );

    std::vector<double> M_VARS("",
        (mc_weight)
        // momentum at vertex
        (mc_p)(mc_pt)(mc_phi)(mc_lam)(mc_theta)
        // vertex position
        (mc_vx)(mc_vy)(mc_vz)(mc_vr)(mc_vt)
        // time at hit 0
        (mc_t0)
    );
    std::vector<int32_t> M_VARS("",
        // hit id of hit 0
        (mc_hid0)
    );


    explicit
    rec_frames_t(TTree* tree = nullptr) : tree_t(tree) {
    }
};

} // namespace mu3e::root
