/// \file

// mu3eDetectorCfg.h
// Detector parameters
#ifndef mu3eDetectorCfg_h
#define mu3eDetectorCfg_h

#include <mu3e/util/log.hpp>
#include <mu3e/util/RootFile.h>

/// Class containing the parameters of the detector
/** Per Geant4 standard, measures are in mm, MeV and Tesla
 *
 */
class Mu3eDetectorCfg : public mu3e::util::config_t {
public:
    int verbose = 0;

    /// Get an instance of the singleto class
    static Mu3eDetectorCfg& Instance(){
        static Mu3eDetectorCfg singleton;
        return singleton;
    }

    template <typename T>
    T get(const std::string& name) const {
        std::string prefix = "detector.";

        std::string key = prefix + name;
        auto value = ptree.get_optional<std::string>(key);
        if(!value) {
            mu3e::log::fatal("[Mu3eDetectorCfg::get] option '%s' not found\n", key.c_str());
            mu3e::log::print_backtrace();
            exit(1);
        }

        boost::optional<std::string> info;
        if(auto options = ptree.get_child_optional(key + "_options")) {
            info = options->get_optional<std::string>(*value);
            if(!info) {
                mu3e::log::fatal("[Mu3eDetectorCfg::get] invalid value '%s'\n", value->c_str());
                mu3e::log::print_backtrace();
                exit(1);
            }
        }

        if(verbose > 0) {
            printf("%s = %s", key.c_str(), value->c_str());
            if(info) printf(" # %s", info->c_str());
            printf("\n");
        }

        return ptree.get<T>(key);
    }

    /// Spacing between small sensors in z
    double getSmallSensorZSpacing() const {
        return get<double>("pixels.small.zSpacing");
    }

    /// Spacing between large sensors in z
    double getLargeSensorZSpacing() const {
        return get<double>("pixels.large.zSpacing");
    }

    /// Number of sensors in Z on layer 3
    int getZSensors3() const {
        return get<int>("pixels.large.nZSensors3");
    }

    /// Number of sensors in Z on layer 4
    int getZSensors4() const {
        return get<int>("pixels.large.nZSensors4");
    }

    /// Number of sensors in Z on the small ladders
    int getZSensorsSmall() const {
        return get<int>("pixels.small.nZSensors");
    }

    /// Get the z position of the first collimator
    double getZCollimator1() const {
        return get<double>("beampipe.collimators.1.z");
    }

    ///Get inner radius of the first collimator
    double getRCollimator1() const {
        return get<double>("beampipe.collimators.1.r");
    }

    ///Get inner radius of the first collimator
    double getTCollimator1() const {
        return get<double>("beampipe.collimators.1.t");
    }

    /// get z position of window relative to beampipe end
    double getWindowZ() const {
        return get<double>("beampipe.window.z");
    }

    /// Get vacuum window thickness
    double getWindowThickness() const {
        return get<double>("beampipe.window.thickness");
    }

    /// Get the z Position of the degrader (offset to collimator)
    double getZOffsetDegrader() const {
        return get<double>("beampipe.degraders.1.zOffset");
    }

    /// Get the thickness of degrader
    double getTDegrader() const {
        return get<double>("beampipe.degraders.1.thickness");
    }

    /// Get the field strength of the transport solenoid
    double getTransportFieldStrength() const {
        return get<double>("magnet.field.transportFieldStrength");
    }

    /// Get the scale factor for the compensation coils
    double getCompensationCoilScale() const {
        return get<double>("magnet.field.compensationCoilScale");
    }

    /// Get the height of the simulated world
    double getWorldHeight() const {
        return get<double>("world.height");
    }

    /// Get the length of the simulated world
    double getWorldLength() const {
        return get<double>("world.length");
    }

    /// Get the width of the simulated world
    double getWorldWidth() const {
        return get<double>("world.length");
    }

    /// Get the distance of the beampipe endpoint from the origin
    double getBeampipeEndpointUpstream() const {
        return get<double>("beampipe.endpoint_us");
    }

    /// Get the distance of the beampipe endpoint from the origin
    double getBeampipeEndpointDownstream() const {
        return get<double>("beampipe.endpoint_ds");
    }

    /// Get the inner radius of the beampipe
    double getBeampipeInnerRadius() const {
        return get<double>("beampipe.radius.inner");
    }

    /// Get the outer radius of the beampipe
    double getBeampipeOuterRadius() const {
        return get<double>("beampipe.radius.outer");
    }

    /// Get the filename with the fieldmap
    std::string getFieldmap() const {
        return get<std::string>("magnet.field.map");
    }

    /// Get the number of recurl scintillator tiles in phi
    int getTileNPhi() const {
        return get<int>("tiles.nPhi");
    }

    /// Get the number of recurl scintillator tiles in z
    int getTileNZ() const {
        return get<int>("tiles.nZ");
    }

    /// Get the number of modules per tile recurl station
    int getTileNModules() const {
        return get<int>("tiles.nModules");
    }

    /// Get the total length of a tile recurl station
    double getTileStationLength() const {
        return get<double>("tiles.stationLength");
    }

    /// Get the active length of a tile recurl station
    double getTileStationActiveLength() const {
        return get<double>("tiles.stationActiveLength");
    }

    /// Get the height of the scintillator tiles
    double getTileHeight() const {
        return get<double>("tiles.tile.height");
    }

    /// Get the gap size within a tile matrix in phi
    double getTileGapPhi() const {
        return get<double>("tiles.tile.gapPhi");
    }

    /// Get the gap size within a tile matrix in z
    double getTileGapZ() const {
        return get<double>("tiles.tile.gapZ");
    }

    /// Get the gap size between the tile matrices in phi
    double getTileMatrixGapPhi() const {
        return get<double>("tiles.tile.matrixGapPhi");
    }

    /// Get the gap size between the tile matrices in z
    double getTileMatrixGapZ() const {
        return get<double>("tiles.tile.matrixGapZ");
    }

    /// Get the innermost radius of the tile recurl station
    double getTileRadiusInner() const {
        return get<double>("tiles.radius.inner");
    }

    /// Get the outermost radius of the tile recurl station
    double getTileRadiusOuter() const {
        return get<double>("tiles.radius.outer");
    }

    /// Get the inner radius of the tile matrix
    double getTileRadiusMatrixInner() const {
        return get<double>("tiles.radius.matrixInner");
    }

    /// Get the inner radius of the support plate
    double getTileRadiusSupportInner() const {
        return get<double>("tiles.radius.supportInner");
    }

    /// Get the inner radius of the endrings
    double getTileRadiusEndringInner() const {
        return get<double>("tiles.radius.endringInner");
    }

    /// Get the thickness of the tile SiPMs
    double getTileSipmThickness() const {
        return get<double>("tiles.sipms.thickness");
    }

    /// Get the width (=length) of the tile SiPMs
    double getTileSipmWidth() const {
        return get<double>("tiles.sipms.width");
    }

    /// Get the thickness of the tile SiPM PCB
    double getTileSipmPCBThickness() const {
        return get<double>("tiles.sipms.PCBthickness");
    }

    /// Get the MuTRiG thickness
    double getTileMutrigThickness() const {
        return get<double>("tiles.mutrig.thickness");
    }

    /// Get the MuTRiG width
    double getTileMutrigWidth() const {
        return get<double>("tiles.mutrig.width");
    }

    /// Get the Tile Module Board thickness
    double getTileTMBThickness() const {
        return get<double>("tiles.tmb.thickness");
    }

    /// Get the Tile Module Board width
    double getTileTMBWidth() const {
        return get<double>("tiles.tmb.width");
    }

    /// Get the tile support plate width
    double getTileSupportWidth() const {
        return get<double>("tiles.support.width");
    }

    /// Get the tile support plate height
    double getTileSupportHeight() const {
        return get<double>("tiles.support.height");
    }

    /// Get the tile endring length
    double getTileEndringLength() const {
        return get<double>("tiles.support.endring");
    }

    /// Get the tile cooling pipe inner diameter
    double getTilePipeID() const {
        return get<double>("tiles.support.pipeID");
    }

    /// Get the tile cooling pipe outer diameter
    double getTilePipeOD() const {
        return get<double>("tiles.support.pipeOD");
    }

    /// Get the tile cooling pipe distance centre-centre
    double getTilePipePitch() const {
        return get<double>("tiles.support.pipePitch");
    }

    /// Get the width of the dead area of the large sensor
    double getLargeSensorDeadWidth() const {
        return get<double>("pixels.large.deadWidth");
    }

    /// Get the guard ring offset and scribe line size of the large sensor
    double getLargeSensorInactiveFrameSize() const {
        return get<double>("pixels.large.inactiveFrameSize");
    }

    /// Get the pixel size for the large sensors
    double getLargeSensorPixelSize() const {
        return get<double>("pixels.large.pixelSize");
    }

    /// Get the width of the dead area of the small sensor
    double getSmallSensorDeadWidth() const {
        return get<double>("pixels.small.deadWidth");
    }

    /// Get the guard ring offset and scribe line size of the small sensor
    double getSmallSensorInactiveFrameSize() const {
        return get<double>("pixels.small.inactiveFrameSize");
    }

    /// Get the pixel size for the small sensors
    double getSmallSensorPixelSize() const {
        return get<double>("pixels.small.pixelSize");
    }

    /// Get the number of tracker layers
    int getTotalLayers() const {
        return get<int>("pixels.small.nLayers") +
               get<int>("pixels.large.nLayers");
    }

    /// Get the strenght of the magnetic field
    double getMagneticFieldStrength() const {
        return get<double>("magnet.field.strength");
    }

    /// Get the inner radius of the magnet
    double getMagnetInnerRadius() const {
        return get<double>("magnet.radius.inner");
    }

    /// Get the length of the magnet
    double getMagnetLength() const {
        return get<double>("magnet.length");
    }

    /// Get the outer radius of the magnet
    double getMagnetOuterRadius() const {
        return get<double>("magnet.radius.outer");
    }

    /// Get the thickness of the conductor on the kapton strips
    double getConductorThickness() const {
        return get<double>("pixels.conductor.thickness");
    }

    /// Get the width of the conductor on the kapton strips
    double getConductorWidth() const {
        return get<double>("pixels.conductor.width");
    }

    /// Get the diameter of the fibres for the inner fibre tracker

    double getFibreDiameter() const {
        return get<double>("fibres.diameter");
    }

    /// Get the dead area width between the fibres in a ribbon
    double getFibreDeadWidth() const {
        return get<double>("fibres.deadWidth");
    }

    /// Get the length of the inner fibres
    double getFibreLength() const {
        return get<double>("fibres.length");
    }

    /// Get the number of ribbons in the inner fibre tracker
    int getFibreRibbons() const {
        return get<int>("fibres.ribbons.n");
    }

    /// Get the pitch between two fibre ribbons
    double getFibreRibbonPitch() const {
        return get<double>("fibres.ribbons.pitch");
    }

    /// Get the number of fibre layers per ribbon in the inner fibre tracker
    int getFibreLayers() const {
        return get<int>("fibres.nLayers");
    }

    /// Get max number of fibres per ribbon layer in the inner fibre tracker
    int getMaxFibersPerLayer() const {
        return get<int>("fibres.maxFibresPerLayer");
    }

    /// Get the dead area width between the fibres in a ribbon
    int getFibreGeometry() const {
        return get<int>("fibres.ribbons.geometry");
    }

    /// Get the thickness of an optional additional aluminum couating
    double getFibreAlCoating() const {
        return get<double>("fibres.alCoating");
    }

    /// Get the thickness of an optional additional aluminum couating
    double getFibreROffset() const {
        return get<double>("fibres.ribbons.rOffset");
    }

    /// get the longitudinal amount of staggering of the fibre ribbons (total separation)
    double getFibreRibbonsStagger() const {
        return get<double>("fibres.ribbons.stagger");
    }

  /// Get if square fibres are used
    bool getFibreSquare() const {
        return get<bool>("fibres.square");
    }

    /// Get fibres refractive index
    double getFibreRefractiveIndex() const {
        return get<double>("fibres.refractiveIndex");
    }

    /// Get the length of the main kapton area reaching horizontally beyond the area filled with sensors (inner tracker)
    double getKaptonFpOverlengthInner() const {
        return get<double>("pixels.kapton.fpOverlengthInner");
    }

    /// Get the length of the main kapton area reaching horizontally beyond the area filled with sensors (inner outer)
    double getKaptonFpOverlengthOuter() const {
        return get<double>("pixels.kapton.fpOverlengthOuter");
    }

    /// Get the length of the kapton extension that are connected to endpieces (inner tracker)
    double getKaptonExtLengthInner() const {
        return get<double>("pixels.kapton.extLengthInner");
    }

    /// Get the length of the kapton extension that are connected to endpieces (outer tracker)
    double getKaptonExtLengthOuter() const {
        return get<double>("pixels.kapton.extLengthOuter");
    }

    /// Get the thickness of the kapton support
    double getKaptonThickness() const {
        return get<double>("pixels.kapton.thickness");
    }

    /// Get the thickness of the inner Mylar condom (around inner tracker)
    double getMylarCondomThicknessInner() const {
        return get<double>("pixels.mylarCondom.thickness_inner");
    }

    /// Get the thickness of the outer Mylar condom (around outer tracker)
    double getMylarCondomThicknessOuter() const {
        return get<double>("pixels.mylarCondom.thickness_outer");
    }

    /// Get the number of layers using the large sensors
    int getLargeLayers() const {
        return get<int>("pixels.large.nLayers");
    }

    /// Get the length of the large sensors
    double getLargeSensorLength() const {
        return get<double>("pixels.large.length");
    }

    /// Get the thickness of the large sensors
    double getLargeSensorThickness() const {
        return get<double>("pixels.large.thickness");
    }

    /// Get the width of the large sensors
    double getLargeSensorWidth() const {
        return get<double>("pixels.large.width");
    }

    /// Get the magnetic field configuration (0 for no field, 1 for solenoid, 2 for cobra-like)
    int getMagneticFieldConfiguration() const {
        return get<int>("magnet.field.configuration");
    }

    /// Get the number of sensors in azimuth in layer 1
    int getPhiSensorsLayer1() const {
        return get<int>("pixels.nPhiSensors.layer1");
    }

    /// Get the number of sensors in azimuth in layer 2
    int getPhiSensorsLayer2() const {
        return get<int>("pixels.nPhiSensors.layer2");
    }

    /// Get the number of sensors in azimuth in layer 3
    int getPhiSensorsLayer3() const {
        return get<int>("pixels.nPhiSensors.layer3");
    }

    /// Get the number of sensors in azimuth in layer 4
    int getPhiSensorsLayer4() const {
        return get<int>("pixels.nPhiSensors.layer4");
    }

    /// Get the number of sensors in azimuth in layer 5
    int getPhiSensorsLayer5() const {
        return get<int>("pixels.nPhiSensors.layer5");
    }

    /// Get the number of sensors in azimuth in layer 6
    int getPhiSensorsLayer6() const {
        return get<int>("pixels.nPhiSensors.layer6");
    }

    /// Get the number of sensors in azimuth in layer 7
    int getPhiSensorsLayer7() const {
        return get<int>("pixels.nPhiSensors.layer7");
    }

    /// Get the number of sensors in azimuth in layer 8
    int getPhiSensorsLayer8() const {
        return get<int>("pixels.nPhiSensors.layer8");
    }

    /// Get the number of layers using the small sensors
    int getSmallLayers() const {
        return get<int>("pixels.small.nLayers");
    }

    /// Get the length of the small senosrs
    double getSmallSensorLength() const {
        return get<double>("pixels.small.length");
    }

    /// Get the thickness of the small sensors
    double getSmallSensorThickness() const {
        return get<double>("pixels.small.thickness");
    }

    /// Get the width of the small sensors
    double getSmallSensorWidth() const {
        return get<double>("pixels.small.width");
    }

    /// Get the shape of the target
    /** 0 for double cone
     *  1 for plane
     *  2 for garland
     *  3 for reverse garland
     *  4 for 2-turn garland
     *  5 for reverse 2-turn garland
     */
    int getTargetShape() const {
        return get<int>("target.shape");
    }

    /// Get the length of the target (one cone)
    double getTargetLength() const {
        return get<double>("target.length");
    }

    /// Get the radius of the target
    double getTargetRadius() const {
        return get<double>("target.radius");
    }

    /// Get the thickness of the target (front part)
    double getTargetThickness1() const {
        return get<double>("target.thickness1");
    }

    /// Get the thickness of the target (back part)
    double getTargetThickness2() const {
        return get<double>("target.thickness2");
    }

    std::string getTargetMaterial() const {
        return get<std::string>("target.material");
    }

    /// Get the offset of the target in X
    double getTargetOffsetX() const{
        return get<double>("target.offset.x");
    }

    /// Get the offset of the target in Y
    double getTargetOffsetY() const {
        return get<double>("target.offset.y");
    }

    /// Get the offset of the target in Z
    double getTargetOffsetZ() const {
        return get<double>("target.offset.z");
    }

    /// Should a lead ball for Mott scattering be placed inside the target?
    int getMottScatteringBall() const {
        return get<int>("target.MottScattering.placeBall");
    }

    /// Radius of the Mott Scattering Ball
    double getMottScatteringBallRadius() const {
        return get<double>("target.MottScattering.ballRadius");
    }

    /// Z offset of the Mott Scattering Ball
    double getMottScatteringBallZOffset() const {
        return get<double>("target.MottScattering.ballZOffset");
    }

    /// Get the detector phase
    /** 0 for phase 1a
     * 1 for phase 1b
     * 2 for phase 2
     */
    double getDetectorPhase() const {
        return get<double>("phase");
    }

    /// get Fibres Mppc Epoxy Thickness
    double getFibreMppcEpoxyThickness() const {
        return get<double>("fibres.mppcs.epoxy.thickness");
    }
    /// get Fibres Mppc Thickness
    double getFibreMppcThickness() const {
        return get<double>("fibres.mppcs.thickness");
    }
    /// get Fibres Mppc Pcb Thickness
    double getFibreMppcPcbThickness() const {
        return get<double>("fibres.mppcs.pcb.thickness");
    }
    /// get Fibres Mppc Height
    double getFibreMppcHeight() const {
        return get<double>("fibres.mppcs.height");
    }
    /// get Fibres Mppc Width
    double getFibreMppcWidth() const {
        return get<double>("fibres.mppcs.width");
    }

protected:
    /// Constructor - protected to ensure singleton status
    Mu3eDetectorCfg() {}
};

#endif
