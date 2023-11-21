//

#include "Conf.h"

#include <mu3e/util/conf_parser.hpp>
#include <mu3e/util/math.hpp>

namespace pt = boost::property_tree;

mu3e::rec::conf_t mu3e::rec::conf;
mu3e::rec::conf_t& Conf::inst = mu3e::rec::conf;

/**
 * Adjust var by unit (mm, ns, etc.).
 */
template <typename T, typename U>
T adjust_var_unit(T var, U unit) {
    assert(unit > 0);
    return var * static_cast<T>(unit);
}
/**
 * Ignore unit for boolean vars.
 */
template <>
bool adjust_var_unit<bool, int>(bool var, int) { return var; }

template < typename T >
void conf_update(const char* name, T& value, const T& new_value) {
    if(value == new_value) return;
    fmt::print("  {} = {} <= {}\n", name, value, new_value);
    value = new_value;
}

#define CONF_GET(name, conf_name, unit) \
    do { \
        auto value = ptree.get<decltype(name)>(prefix + conf_name); \
        if(std::abs(mu3e::units::unit) > 0) value = adjust_var_unit(value, mu3e::units::unit); \
        conf_update(BOOST_STRINGIZE(name), this->name, value); \
    } while(0)

void mu3e::rec::conf_t::read_detector_conf(const boost::property_tree::ptree& ptree) {
    std::string prefix = "detector.";

    CONF_GET(B                    , "magnet.field.strength"   , Tesla);
    CONF_GET(fb.square            , "fibres.square"           , _);
    CONF_GET(fb.length            , "fibres.length"           , mm);
    CONF_GET(fb.diameter          , "fibres.diameter"         , mm);
    CONF_GET(fb.deadWidth         , "fibres.deadWidth"        , mm);
    CONF_GET(fb.nLayers           , "fibres.nLayers"          , _);
    CONF_GET(fb.nColumns          , "fibres.maxFibresPerLayer", _);
    CONF_GET(fb.nRibbons          , "fibres.ribbons.n"        , _);
    CONF_GET(fb.refidx            , "fibres.refractiveIndex"  , _);
    CONF_GET(tl.r_outer           , "tiles.radius.outer"      , mm);
    CONF_GET(tl.r_inner           , "tiles.radius.inner"      , mm);
    CONF_GET(tl.nPhi              , "tiles.nPhi"              , _);
    CONF_GET(tl.nZ                , "tiles.nZ"                , _);
    CONF_GET(tl.nModules          , "tiles.nModules"          , _);

    fb.thickness = (fb.diameter + fb.deadWidth) * (fb.nLayers - 1);
    if(fb.square) {
        fb.thickness = fb.diameter + fb.thickness;
    }
    else {
        fb.thickness = fb.diameter + fb.thickness * std::sqrt(3.0f) / 2;
    }
    if(fb.nLayers == 0) fb.thickness = 0;

    tl.tile_l_z = ptree.get<float>(prefix + "tiles.stationActiveLength") * mu3e::units::mm / tl.nZ;
}

void mu3e::rec::conf_t::read_digi_conf(const boost::property_tree::ptree& ptree) {
    std::string prefix = "digi.";

    CONF_GET(frameLength   , "frameLength"        , ns);
    CONF_GET(tl.resolution , "tiles.timeRes"      , ns);
    CONF_GET(si.resolution , "tracker.timeRes"    , ns);
    CONF_GET(si.n_fine_bits, "tracker.addTimeBits", _);
    CONF_GET(tl.n_fine_bits, "mutrig.addTimeBits" , _);
    CONF_GET(fb.n_fine_bits, "mutrig.addTimeBits" , _);
}

void mu3e::rec::conf_t::read_sort_conf(const boost::property_tree::ptree& /*ptree*/) {
    std::string prefix = "sort.";

//    skippedFrames = ptree.get<decltype(skippedFrames)>(prefix + "skip");
//    std::cout << "  " << "skippedFrames" << " = " << skippedFrames << std::endl;
}

void mu3e::rec::conf_t::read_ptree(const boost::property_tree::ptree& ptree) {
    read_detector_conf(ptree);
    read_digi_conf(ptree);
    read_sort_conf(ptree);

#define MU3E_CONF_GET(name) \
    do { \
        auto value = ptree.get_optional<decltype(name)>("trirec."#name); \
        if(!value) break; \
        conf_update(#name, this->name, *value); \
    } while(0)

    MU3E_CONF_GET(alignment.sensors);
    MU3E_CONF_GET(alignment.fibres);
    MU3E_CONF_GET(alignment.mppcs);
    MU3E_CONF_GET(alignment.tiles);

    MU3E_CONF_GET(conddb.dbconn);

    MU3E_CONF_GET(triplet_rt_min);
    MU3E_CONF_GET(triplet_rt_max);
    if(!(triplet_rt_max > 0)) triplet_rt_max = FLT_MAX;

    MU3E_CONF_GET(seg4_z01_max);
    MU3E_CONF_GET(seg4_z12_max);
    MU3E_CONF_GET(seg4_phi01_max);
    MU3E_CONF_GET(seg4_phi12_max);
    MU3E_CONF_GET(seg4_dphi3_max);
    MU3E_CONF_GET(seg4_dz3_max);

    MU3E_CONF_GET(gpu.n_hits_max);
    MU3E_CONF_GET(gpu.n_comb_max);

    MU3E_CONF_GET(seg6_dphi4_max);
    MU3E_CONF_GET(seg6_dz4_max);
    MU3E_CONF_GET(seg6_dphi5_max);
    MU3E_CONF_GET(seg6_dz5_max);

    MU3E_CONF_GET(L8o_dphi4_max);
    MU3E_CONF_GET(L8o_dz4_max);
    MU3E_CONF_GET(L8i_dphi4_max);
    MU3E_CONF_GET(L8i_dz4_max);

    MU3E_CONF_GET(seg3_chi2_max);
    MU3E_CONF_GET(seg4_chi2_max);
    MU3E_CONF_GET(seg6_chi2_max);
    MU3E_CONF_GET(seg8_chi2_max);

    MU3E_CONF_GET(rec_version);

    MU3E_CONF_GET(rec_fb);
    MU3E_CONF_GET(rec_tl);

    MU3E_CONF_GET(rec_vertex_xy);

    MU3E_CONF_GET(alg.merge_mode);
    MU3E_CONF_GET(alg.collapse_mode);
    MU3E_CONF_GET(alg.resolve_mode);
    MU3E_CONF_GET(alg.resolve_ordering);
    MU3E_CONF_GET(alg.resolve_n_min);
    MU3E_CONF_GET(alg.resolve_n_max);
    MU3E_CONF_GET(alg.resolve_k_max);
    MU3E_CONF_GET(alg.resolve_timeout_ms);

    MU3E_CONF_GET(root.ex);
    MU3E_CONF_GET(root.status);
    MU3E_CONF_GET(root.segs3);
    MU3E_CONF_GET(root.segs3_gpu);
    MU3E_CONF_GET(root.segs4);
    MU3E_CONF_GET(root.segs4_gpu);
    MU3E_CONF_GET(root.segs6);
    MU3E_CONF_GET(root.segs8);
    MU3E_CONF_GET(root.frames_skip_empty);

    MU3E_CONF_GET(root.segs8_);

    MU3E_CONF_GET(fb.cluster_dt_max);
    MU3E_CONF_GET(fb.cluster_recl_dt);
    MU3E_CONF_GET(fb.link_method);
    MU3E_CONF_GET(fb.link_nh_min);
    MU3E_CONF_GET(fb.link_nh_max);
    MU3E_CONF_GET(fb.link_d_max);
    MU3E_CONF_GET(fb.link_sides_tolerance);
    MU3E_CONF_GET(fb.link_col_tollerance);
    MU3E_CONF_GET(fb.resolution);
    MU3E_CONF_GET(fb.merge_sides);
    MU3E_CONF_GET(fb.delay);

    // vertex
    MU3E_CONF_GET(vertex.n_tracks_max);
    MU3E_CONF_GET(vertex.delta_p_min);
    MU3E_CONF_GET(vertex.p_min);
    MU3E_CONF_GET(vertex.p_max);
    MU3E_CONF_GET(vertex.circle_intersection_margin);
    MU3E_CONF_GET(vertex.posprodcut);
    MU3E_CONF_GET(vertex.negprodcut);
    MU3E_CONF_GET(vertex.E_tot_min);
    MU3E_CONF_GET(vertex.p_mag_max);
    MU3E_CONF_GET(vertex.targetdist_max);
    MU3E_CONF_GET(vertex.target_r);
    MU3E_CONF_GET(vertex.target_half_length);
    MU3E_CONF_GET(vertex.target_region);
    MU3E_CONF_GET(vertex.chi2_xy_max);
    MU3E_CONF_GET(vertex.chi2_rz_max);
    MU3E_CONF_GET(vertex.chi2_max);

    MU3E_CONF_GET(twolayer.seg4_outer_chi2_max);
    MU3E_CONF_GET(twolayer.seg4_outer_z01_max);
    MU3E_CONF_GET(twolayer.seg4_outer_dphi01_max);
    MU3E_CONF_GET(twolayer.seg4_outer_dphi12_max);
    MU3E_CONF_GET(twolayer.seg4_outer_dphi23_max);
    MU3E_CONF_GET(twolayer.seg4_outer_dz2_max);
    MU3E_CONF_GET(twolayer.seg4_outer_dz3_max);
    MU3E_CONF_GET(twolayer.seg4_inner_chi2_max);
    MU3E_CONF_GET(twolayer.seg4_inner_z01_max);
    MU3E_CONF_GET(twolayer.seg4_inner_dphi01_max);
    MU3E_CONF_GET(twolayer.seg4_inner_dphi12_max);
    MU3E_CONF_GET(twolayer.seg4_inner_dphi23_max);
    MU3E_CONF_GET(twolayer.seg4_inner_dz2_max);
    MU3E_CONF_GET(twolayer.seg4_inner_dz3_max);

    MU3E_CONF_GET(display.scale);
    MU3E_CONF_GET(display.viewXY.w);
    MU3E_CONF_GET(display.viewXY.h);
    MU3E_CONF_GET(display.viewRZ.w);
    MU3E_CONF_GET(display.viewRZ.h);

#undef MU3E_CONF_GET
}
