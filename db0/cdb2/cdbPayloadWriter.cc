#include "cdbPayloadWriter.hh"

#include "cdbUtil.hh"
#include "base64.hh"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <iomanip>
#include <chrono>
#include <glob.h>

#include <TFile.h>
#include <TTree.h>
#include <map>

#include "calPixelAlignment.hh"
#include "calFibreAlignment.hh"
#include "calMppcAlignment.hh"
#include "calTileAlignment.hh"
#include "calPixelQualityLM.hh"
#include "calPixelEfficiency.hh"
#include "calFibreQuality.hh"
#include "calTileQuality.hh"
#include "calDetSetupV1.hh"
#include "calEventStuffV1.hh"
#include "calPixelTimeCalibration.hh"

using namespace std;

// ----------------------------------------------------------------------
cdbPayloadWriter::cdbPayloadWriter() {
  fChipIDs.clear();
  
  // -----------------------------------------------------------------
  // -- The code for this section can be obtained by running 
  //    ./bin/cdbWriteIdealInputFiles -m createchipidsperlayer -f ascii/mu3e_alignment_mcidealv6.5.root
  // -----------------------------------------------------------------
  
  // ->cdbPayloadWriter> read 48 layer 1 chip IDs
  fLayer1ChipIDs = {
    1, 2, 3, 4, 5, 6, 33, 34, 35, 36, 37, 38, 
    65, 66, 67, 68, 69, 70, 97, 98, 99, 100, 101, 102, 
    129, 130, 131, 132, 133, 134, 161, 162, 163, 164, 165, 166, 
    193, 194, 195, 196, 197, 198, 225, 226, 227, 228, 229, 230
  };
  
  
  // ->cdbPayloadWriter> read 60 layer 2 chip IDs
  fLayer2ChipIDs = {
    1025, 1026, 1027, 1028, 1029, 1030, 1057, 1058, 1059, 1060, 1061, 1062, 
    1089, 1090, 1091, 1092, 1093, 1094, 1121, 1122, 1123, 1124, 1125, 1126, 
    1153, 1154, 1155, 1156, 1157, 1158, 1185, 1186, 1187, 1188, 1189, 1190, 
    1217, 1218, 1219, 1220, 1221, 1222, 1249, 1250, 1251, 1252, 1253, 1254, 
    1281, 1282, 1283, 1284, 1285, 1286, 1313, 1314, 1315, 1316, 1317, 1318
  };
  
  
  // ->cdbPayloadWriter> read 408 layer 3 station 0 (central) chip IDs
  fLayer3Station0ChipIDs = {
    2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 
    2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2096, 2097, 2098, 2099, 2100, 2101, 2102, 2103, 2104, 
    2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 2136, 
    2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167, 2168, 
    2184, 2185, 2186, 2187, 2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 2200, 
    2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227, 2228, 2229, 2230, 2231, 2232, 
    2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263, 2264, 
    2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294, 2295, 2296, 
    2312, 2313, 2314, 2315, 2316, 2317, 2318, 2319, 2320, 2321, 2322, 2323, 2324, 2325, 2326, 2327, 2328, 
    2344, 2345, 2346, 2347, 2348, 2349, 2350, 2351, 2352, 2353, 2354, 2355, 2356, 2357, 2358, 2359, 2360, 
    2376, 2377, 2378, 2379, 2380, 2381, 2382, 2383, 2384, 2385, 2386, 2387, 2388, 2389, 2390, 2391, 2392, 
    2408, 2409, 2410, 2411, 2412, 2413, 2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421, 2422, 2423, 2424, 
    2440, 2441, 2442, 2443, 2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451, 2452, 2453, 2454, 2455, 2456, 
    2472, 2473, 2474, 2475, 2476, 2477, 2478, 2479, 2480, 2481, 2482, 2483, 2484, 2485, 2486, 2487, 2488, 
    2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519, 2520, 
    2536, 2537, 2538, 2539, 2540, 2541, 2542, 2543, 2544, 2545, 2546, 2547, 2548, 2549, 2550, 2551, 2552, 
    2568, 2569, 2570, 2571, 2572, 2573, 2574, 2575, 2576, 2577, 2578, 2579, 2580, 2581, 2582, 2583, 2584, 
    2600, 2601, 2602, 2603, 2604, 2605, 2606, 2607, 2608, 2609, 2610, 2611, 2612, 2613, 2614, 2615, 2616, 
    2632, 2633, 2634, 2635, 2636, 2637, 2638, 2639, 2640, 2641, 2642, 2643, 2644, 2645, 2646, 2647, 2648, 
    2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671, 2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 
    2696, 2697, 2698, 2699, 2700, 2701, 2702, 2703, 2704, 2705, 2706, 2707, 2708, 2709, 2710, 2711, 2712, 
    2728, 2729, 2730, 2731, 2732, 2733, 2734, 2735, 2736, 2737, 2738, 2739, 2740, 2741, 2742, 2743, 2744, 
    2760, 2761, 2762, 2763, 2764, 2765, 2766, 2767, 2768, 2769, 2770, 2771, 2772, 2773, 2774, 2775, 2776, 
    2792, 2793, 2794, 2795, 2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805, 2806, 2807, 2808
  };
  
  
  // ->cdbPayloadWriter> read 408 layer 3 station 1 (US) chip IDs
  fLayer3Station1ChipIDs = {
    6152, 6153, 6154, 6155, 6156, 6157, 6158, 6159, 6160, 6161, 6162, 6163, 6164, 6165, 6166, 6167, 6168, 
    6184, 6185, 6186, 6187, 6188, 6189, 6190, 6191, 6192, 6193, 6194, 6195, 6196, 6197, 6198, 6199, 6200, 
    6216, 6217, 6218, 6219, 6220, 6221, 6222, 6223, 6224, 6225, 6226, 6227, 6228, 6229, 6230, 6231, 6232, 
    6248, 6249, 6250, 6251, 6252, 6253, 6254, 6255, 6256, 6257, 6258, 6259, 6260, 6261, 6262, 6263, 6264, 
    6280, 6281, 6282, 6283, 6284, 6285, 6286, 6287, 6288, 6289, 6290, 6291, 6292, 6293, 6294, 6295, 6296, 
    6312, 6313, 6314, 6315, 6316, 6317, 6318, 6319, 6320, 6321, 6322, 6323, 6324, 6325, 6326, 6327, 6328, 
    6344, 6345, 6346, 6347, 6348, 6349, 6350, 6351, 6352, 6353, 6354, 6355, 6356, 6357, 6358, 6359, 6360, 
    6376, 6377, 6378, 6379, 6380, 6381, 6382, 6383, 6384, 6385, 6386, 6387, 6388, 6389, 6390, 6391, 6392, 
    6408, 6409, 6410, 6411, 6412, 6413, 6414, 6415, 6416, 6417, 6418, 6419, 6420, 6421, 6422, 6423, 6424, 
    6440, 6441, 6442, 6443, 6444, 6445, 6446, 6447, 6448, 6449, 6450, 6451, 6452, 6453, 6454, 6455, 6456, 
    6472, 6473, 6474, 6475, 6476, 6477, 6478, 6479, 6480, 6481, 6482, 6483, 6484, 6485, 6486, 6487, 6488, 
    6504, 6505, 6506, 6507, 6508, 6509, 6510, 6511, 6512, 6513, 6514, 6515, 6516, 6517, 6518, 6519, 6520, 
    6536, 6537, 6538, 6539, 6540, 6541, 6542, 6543, 6544, 6545, 6546, 6547, 6548, 6549, 6550, 6551, 6552, 
    6568, 6569, 6570, 6571, 6572, 6573, 6574, 6575, 6576, 6577, 6578, 6579, 6580, 6581, 6582, 6583, 6584, 
    6600, 6601, 6602, 6603, 6604, 6605, 6606, 6607, 6608, 6609, 6610, 6611, 6612, 6613, 6614, 6615, 6616, 
    6632, 6633, 6634, 6635, 6636, 6637, 6638, 6639, 6640, 6641, 6642, 6643, 6644, 6645, 6646, 6647, 6648, 
    6664, 6665, 6666, 6667, 6668, 6669, 6670, 6671, 6672, 6673, 6674, 6675, 6676, 6677, 6678, 6679, 6680, 
    6696, 6697, 6698, 6699, 6700, 6701, 6702, 6703, 6704, 6705, 6706, 6707, 6708, 6709, 6710, 6711, 6712, 
    6728, 6729, 6730, 6731, 6732, 6733, 6734, 6735, 6736, 6737, 6738, 6739, 6740, 6741, 6742, 6743, 6744, 
    6760, 6761, 6762, 6763, 6764, 6765, 6766, 6767, 6768, 6769, 6770, 6771, 6772, 6773, 6774, 6775, 6776, 
    6792, 6793, 6794, 6795, 6796, 6797, 6798, 6799, 6800, 6801, 6802, 6803, 6804, 6805, 6806, 6807, 6808, 
    6824, 6825, 6826, 6827, 6828, 6829, 6830, 6831, 6832, 6833, 6834, 6835, 6836, 6837, 6838, 6839, 6840, 
    6856, 6857, 6858, 6859, 6860, 6861, 6862, 6863, 6864, 6865, 6866, 6867, 6868, 6869, 6870, 6871, 6872, 
    6888, 6889, 6890, 6891, 6892, 6893, 6894, 6895, 6896, 6897, 6898, 6899, 6900, 6901, 6902, 6903, 6904
  };
  
  
  // ->cdbPayloadWriter> read 408 layer 3 station 2 (DS) chip IDs
  fLayer3Station2ChipIDs = {
    10248, 10249, 10250, 10251, 10252, 10253, 10254, 10255, 10256, 10257, 10258, 10259, 10260, 10261, 10262, 10263, 10264, 
    10280, 10281, 10282, 10283, 10284, 10285, 10286, 10287, 10288, 10289, 10290, 10291, 10292, 10293, 10294, 10295, 10296, 
    10312, 10313, 10314, 10315, 10316, 10317, 10318, 10319, 10320, 10321, 10322, 10323, 10324, 10325, 10326, 10327, 10328, 
    10344, 10345, 10346, 10347, 10348, 10349, 10350, 10351, 10352, 10353, 10354, 10355, 10356, 10357, 10358, 10359, 10360, 
    10376, 10377, 10378, 10379, 10380, 10381, 10382, 10383, 10384, 10385, 10386, 10387, 10388, 10389, 10390, 10391, 10392, 
    10408, 10409, 10410, 10411, 10412, 10413, 10414, 10415, 10416, 10417, 10418, 10419, 10420, 10421, 10422, 10423, 10424, 
    10440, 10441, 10442, 10443, 10444, 10445, 10446, 10447, 10448, 10449, 10450, 10451, 10452, 10453, 10454, 10455, 10456, 
    10472, 10473, 10474, 10475, 10476, 10477, 10478, 10479, 10480, 10481, 10482, 10483, 10484, 10485, 10486, 10487, 10488, 
    10504, 10505, 10506, 10507, 10508, 10509, 10510, 10511, 10512, 10513, 10514, 10515, 10516, 10517, 10518, 10519, 10520, 
    10536, 10537, 10538, 10539, 10540, 10541, 10542, 10543, 10544, 10545, 10546, 10547, 10548, 10549, 10550, 10551, 10552, 
    10568, 10569, 10570, 10571, 10572, 10573, 10574, 10575, 10576, 10577, 10578, 10579, 10580, 10581, 10582, 10583, 10584, 
    10600, 10601, 10602, 10603, 10604, 10605, 10606, 10607, 10608, 10609, 10610, 10611, 10612, 10613, 10614, 10615, 10616, 
    10632, 10633, 10634, 10635, 10636, 10637, 10638, 10639, 10640, 10641, 10642, 10643, 10644, 10645, 10646, 10647, 10648, 
    10664, 10665, 10666, 10667, 10668, 10669, 10670, 10671, 10672, 10673, 10674, 10675, 10676, 10677, 10678, 10679, 10680, 
    10696, 10697, 10698, 10699, 10700, 10701, 10702, 10703, 10704, 10705, 10706, 10707, 10708, 10709, 10710, 10711, 10712, 
    10728, 10729, 10730, 10731, 10732, 10733, 10734, 10735, 10736, 10737, 10738, 10739, 10740, 10741, 10742, 10743, 10744, 
    10760, 10761, 10762, 10763, 10764, 10765, 10766, 10767, 10768, 10769, 10770, 10771, 10772, 10773, 10774, 10775, 10776, 
    10792, 10793, 10794, 10795, 10796, 10797, 10798, 10799, 10800, 10801, 10802, 10803, 10804, 10805, 10806, 10807, 10808, 
    10824, 10825, 10826, 10827, 10828, 10829, 10830, 10831, 10832, 10833, 10834, 10835, 10836, 10837, 10838, 10839, 10840, 
    10856, 10857, 10858, 10859, 10860, 10861, 10862, 10863, 10864, 10865, 10866, 10867, 10868, 10869, 10870, 10871, 10872, 
    10888, 10889, 10890, 10891, 10892, 10893, 10894, 10895, 10896, 10897, 10898, 10899, 10900, 10901, 10902, 10903, 10904, 
    10920, 10921, 10922, 10923, 10924, 10925, 10926, 10927, 10928, 10929, 10930, 10931, 10932, 10933, 10934, 10935, 10936, 
    10952, 10953, 10954, 10955, 10956, 10957, 10958, 10959, 10960, 10961, 10962, 10963, 10964, 10965, 10966, 10967, 10968, 
    10984, 10985, 10986, 10987, 10988, 10989, 10990, 10991, 10992, 10993, 10994, 10995, 10996, 10997, 10998, 10999, 11000
  };
  
  
  // ->cdbPayloadWriter> read 504 layer 4 station 0 (central) chip IDs
  fLayer4Station0ChipIDs = {
    3079, 3080, 3081, 3082, 3083, 3084, 3085, 3086, 3087, 3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3096, 
    3111, 3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 
    3143, 3144, 3145, 3146, 3147, 3148, 3149, 3150, 3151, 3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 
    3175, 3176, 3177, 3178, 3179, 3180, 3181, 3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190, 3191, 3192, 
    3207, 3208, 3209, 3210, 3211, 3212, 3213, 3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221, 3222, 3223, 3224, 
    3239, 3240, 3241, 3242, 3243, 3244, 3245, 3246, 3247, 3248, 3249, 3250, 3251, 3252, 3253, 3254, 3255, 3256, 
    3271, 3272, 3273, 3274, 3275, 3276, 3277, 3278, 3279, 3280, 3281, 3282, 3283, 3284, 3285, 3286, 3287, 3288, 
    3303, 3304, 3305, 3306, 3307, 3308, 3309, 3310, 3311, 3312, 3313, 3314, 3315, 3316, 3317, 3318, 3319, 3320, 
    3335, 3336, 3337, 3338, 3339, 3340, 3341, 3342, 3343, 3344, 3345, 3346, 3347, 3348, 3349, 3350, 3351, 3352, 
    3367, 3368, 3369, 3370, 3371, 3372, 3373, 3374, 3375, 3376, 3377, 3378, 3379, 3380, 3381, 3382, 3383, 3384, 
    3399, 3400, 3401, 3402, 3403, 3404, 3405, 3406, 3407, 3408, 3409, 3410, 3411, 3412, 3413, 3414, 3415, 3416, 
    3431, 3432, 3433, 3434, 3435, 3436, 3437, 3438, 3439, 3440, 3441, 3442, 3443, 3444, 3445, 3446, 3447, 3448, 
    3463, 3464, 3465, 3466, 3467, 3468, 3469, 3470, 3471, 3472, 3473, 3474, 3475, 3476, 3477, 3478, 3479, 3480, 
    3495, 3496, 3497, 3498, 3499, 3500, 3501, 3502, 3503, 3504, 3505, 3506, 3507, 3508, 3509, 3510, 3511, 3512, 
    3527, 3528, 3529, 3530, 3531, 3532, 3533, 3534, 3535, 3536, 3537, 3538, 3539, 3540, 3541, 3542, 3543, 3544, 
    3559, 3560, 3561, 3562, 3563, 3564, 3565, 3566, 3567, 3568, 3569, 3570, 3571, 3572, 3573, 3574, 3575, 3576, 
    3591, 3592, 3593, 3594, 3595, 3596, 3597, 3598, 3599, 3600, 3601, 3602, 3603, 3604, 3605, 3606, 3607, 3608, 
    3623, 3624, 3625, 3626, 3627, 3628, 3629, 3630, 3631, 3632, 3633, 3634, 3635, 3636, 3637, 3638, 3639, 3640, 
    3655, 3656, 3657, 3658, 3659, 3660, 3661, 3662, 3663, 3664, 3665, 3666, 3667, 3668, 3669, 3670, 3671, 3672, 
    3687, 3688, 3689, 3690, 3691, 3692, 3693, 3694, 3695, 3696, 3697, 3698, 3699, 3700, 3701, 3702, 3703, 3704, 
    3719, 3720, 3721, 3722, 3723, 3724, 3725, 3726, 3727, 3728, 3729, 3730, 3731, 3732, 3733, 3734, 3735, 3736, 
    3751, 3752, 3753, 3754, 3755, 3756, 3757, 3758, 3759, 3760, 3761, 3762, 3763, 3764, 3765, 3766, 3767, 3768, 
    3783, 3784, 3785, 3786, 3787, 3788, 3789, 3790, 3791, 3792, 3793, 3794, 3795, 3796, 3797, 3798, 3799, 3800, 
    3815, 3816, 3817, 3818, 3819, 3820, 3821, 3822, 3823, 3824, 3825, 3826, 3827, 3828, 3829, 3830, 3831, 3832, 
    3847, 3848, 3849, 3850, 3851, 3852, 3853, 3854, 3855, 3856, 3857, 3858, 3859, 3860, 3861, 3862, 3863, 3864, 
    3879, 3880, 3881, 3882, 3883, 3884, 3885, 3886, 3887, 3888, 3889, 3890, 3891, 3892, 3893, 3894, 3895, 3896, 
    3911, 3912, 3913, 3914, 3915, 3916, 3917, 3918, 3919, 3920, 3921, 3922, 3923, 3924, 3925, 3926, 3927, 3928, 
    3943, 3944, 3945, 3946, 3947, 3948, 3949, 3950, 3951, 3952, 3953, 3954, 3955, 3956, 3957, 3958, 3959, 3960
  };
  
  
  // ->cdbPayloadWriter> read 504 layer 4 station 1 (US) chip IDs
  fLayer4Station1ChipIDs = {
    7175, 7176, 7177, 7178, 7179, 7180, 7181, 7182, 7183, 7184, 7185, 7186, 7187, 7188, 7189, 7190, 7191, 7192, 
    7207, 7208, 7209, 7210, 7211, 7212, 7213, 7214, 7215, 7216, 7217, 7218, 7219, 7220, 7221, 7222, 7223, 7224, 
    7239, 7240, 7241, 7242, 7243, 7244, 7245, 7246, 7247, 7248, 7249, 7250, 7251, 7252, 7253, 7254, 7255, 7256, 
    7271, 7272, 7273, 7274, 7275, 7276, 7277, 7278, 7279, 7280, 7281, 7282, 7283, 7284, 7285, 7286, 7287, 7288, 
    7303, 7304, 7305, 7306, 7307, 7308, 7309, 7310, 7311, 7312, 7313, 7314, 7315, 7316, 7317, 7318, 7319, 7320, 
    7335, 7336, 7337, 7338, 7339, 7340, 7341, 7342, 7343, 7344, 7345, 7346, 7347, 7348, 7349, 7350, 7351, 7352, 
    7367, 7368, 7369, 7370, 7371, 7372, 7373, 7374, 7375, 7376, 7377, 7378, 7379, 7380, 7381, 7382, 7383, 7384, 
    7399, 7400, 7401, 7402, 7403, 7404, 7405, 7406, 7407, 7408, 7409, 7410, 7411, 7412, 7413, 7414, 7415, 7416, 
    7431, 7432, 7433, 7434, 7435, 7436, 7437, 7438, 7439, 7440, 7441, 7442, 7443, 7444, 7445, 7446, 7447, 7448, 
    7463, 7464, 7465, 7466, 7467, 7468, 7469, 7470, 7471, 7472, 7473, 7474, 7475, 7476, 7477, 7478, 7479, 7480, 
    7495, 7496, 7497, 7498, 7499, 7500, 7501, 7502, 7503, 7504, 7505, 7506, 7507, 7508, 7509, 7510, 7511, 7512, 
    7527, 7528, 7529, 7530, 7531, 7532, 7533, 7534, 7535, 7536, 7537, 7538, 7539, 7540, 7541, 7542, 7543, 7544, 
    7559, 7560, 7561, 7562, 7563, 7564, 7565, 7566, 7567, 7568, 7569, 7570, 7571, 7572, 7573, 7574, 7575, 7576, 
    7591, 7592, 7593, 7594, 7595, 7596, 7597, 7598, 7599, 7600, 7601, 7602, 7603, 7604, 7605, 7606, 7607, 7608, 
    7623, 7624, 7625, 7626, 7627, 7628, 7629, 7630, 7631, 7632, 7633, 7634, 7635, 7636, 7637, 7638, 7639, 7640, 
    7655, 7656, 7657, 7658, 7659, 7660, 7661, 7662, 7663, 7664, 7665, 7666, 7667, 7668, 7669, 7670, 7671, 7672, 
    7687, 7688, 7689, 7690, 7691, 7692, 7693, 7694, 7695, 7696, 7697, 7698, 7699, 7700, 7701, 7702, 7703, 7704, 
    7719, 7720, 7721, 7722, 7723, 7724, 7725, 7726, 7727, 7728, 7729, 7730, 7731, 7732, 7733, 7734, 7735, 7736, 
    7751, 7752, 7753, 7754, 7755, 7756, 7757, 7758, 7759, 7760, 7761, 7762, 7763, 7764, 7765, 7766, 7767, 7768, 
    7783, 7784, 7785, 7786, 7787, 7788, 7789, 7790, 7791, 7792, 7793, 7794, 7795, 7796, 7797, 7798, 7799, 7800, 
    7815, 7816, 7817, 7818, 7819, 7820, 7821, 7822, 7823, 7824, 7825, 7826, 7827, 7828, 7829, 7830, 7831, 7832, 
    7847, 7848, 7849, 7850, 7851, 7852, 7853, 7854, 7855, 7856, 7857, 7858, 7859, 7860, 7861, 7862, 7863, 7864, 
    7879, 7880, 7881, 7882, 7883, 7884, 7885, 7886, 7887, 7888, 7889, 7890, 7891, 7892, 7893, 7894, 7895, 7896, 
    7911, 7912, 7913, 7914, 7915, 7916, 7917, 7918, 7919, 7920, 7921, 7922, 7923, 7924, 7925, 7926, 7927, 7928, 
    7943, 7944, 7945, 7946, 7947, 7948, 7949, 7950, 7951, 7952, 7953, 7954, 7955, 7956, 7957, 7958, 7959, 7960, 
    7975, 7976, 7977, 7978, 7979, 7980, 7981, 7982, 7983, 7984, 7985, 7986, 7987, 7988, 7989, 7990, 7991, 7992, 
    8007, 8008, 8009, 8010, 8011, 8012, 8013, 8014, 8015, 8016, 8017, 8018, 8019, 8020, 8021, 8022, 8023, 8024, 
    8039, 8040, 8041, 8042, 8043, 8044, 8045, 8046, 8047, 8048, 8049, 8050, 8051, 8052, 8053, 8054, 8055, 8056
  };
  
  
  // ->cdbPayloadWriter> read 504 layer 4 station 2 (DS) chip IDs
  fLayer4Station2ChipIDs = {
    11271, 11272, 11273, 11274, 11275, 11276, 11277, 11278, 11279, 11280, 11281, 11282, 11283, 11284, 11285, 11286, 11287, 11288, 
    11303, 11304, 11305, 11306, 11307, 11308, 11309, 11310, 11311, 11312, 11313, 11314, 11315, 11316, 11317, 11318, 11319, 11320, 
    11335, 11336, 11337, 11338, 11339, 11340, 11341, 11342, 11343, 11344, 11345, 11346, 11347, 11348, 11349, 11350, 11351, 11352, 
    11367, 11368, 11369, 11370, 11371, 11372, 11373, 11374, 11375, 11376, 11377, 11378, 11379, 11380, 11381, 11382, 11383, 11384, 
    11399, 11400, 11401, 11402, 11403, 11404, 11405, 11406, 11407, 11408, 11409, 11410, 11411, 11412, 11413, 11414, 11415, 11416, 
    11431, 11432, 11433, 11434, 11435, 11436, 11437, 11438, 11439, 11440, 11441, 11442, 11443, 11444, 11445, 11446, 11447, 11448, 
    11463, 11464, 11465, 11466, 11467, 11468, 11469, 11470, 11471, 11472, 11473, 11474, 11475, 11476, 11477, 11478, 11479, 11480, 
    11495, 11496, 11497, 11498, 11499, 11500, 11501, 11502, 11503, 11504, 11505, 11506, 11507, 11508, 11509, 11510, 11511, 11512, 
    11527, 11528, 11529, 11530, 11531, 11532, 11533, 11534, 11535, 11536, 11537, 11538, 11539, 11540, 11541, 11542, 11543, 11544, 
    11559, 11560, 11561, 11562, 11563, 11564, 11565, 11566, 11567, 11568, 11569, 11570, 11571, 11572, 11573, 11574, 11575, 11576, 
    11591, 11592, 11593, 11594, 11595, 11596, 11597, 11598, 11599, 11600, 11601, 11602, 11603, 11604, 11605, 11606, 11607, 11608, 
    11623, 11624, 11625, 11626, 11627, 11628, 11629, 11630, 11631, 11632, 11633, 11634, 11635, 11636, 11637, 11638, 11639, 11640, 
    11655, 11656, 11657, 11658, 11659, 11660, 11661, 11662, 11663, 11664, 11665, 11666, 11667, 11668, 11669, 11670, 11671, 11672, 
    11687, 11688, 11689, 11690, 11691, 11692, 11693, 11694, 11695, 11696, 11697, 11698, 11699, 11700, 11701, 11702, 11703, 11704, 
    11719, 11720, 11721, 11722, 11723, 11724, 11725, 11726, 11727, 11728, 11729, 11730, 11731, 11732, 11733, 11734, 11735, 11736, 
    11751, 11752, 11753, 11754, 11755, 11756, 11757, 11758, 11759, 11760, 11761, 11762, 11763, 11764, 11765, 11766, 11767, 11768, 
    11783, 11784, 11785, 11786, 11787, 11788, 11789, 11790, 11791, 11792, 11793, 11794, 11795, 11796, 11797, 11798, 11799, 11800, 
    11815, 11816, 11817, 11818, 11819, 11820, 11821, 11822, 11823, 11824, 11825, 11826, 11827, 11828, 11829, 11830, 11831, 11832, 
    11847, 11848, 11849, 11850, 11851, 11852, 11853, 11854, 11855, 11856, 11857, 11858, 11859, 11860, 11861, 11862, 11863, 11864, 
    11879, 11880, 11881, 11882, 11883, 11884, 11885, 11886, 11887, 11888, 11889, 11890, 11891, 11892, 11893, 11894, 11895, 11896, 
    11911, 11912, 11913, 11914, 11915, 11916, 11917, 11918, 11919, 11920, 11921, 11922, 11923, 11924, 11925, 11926, 11927, 11928, 
    11943, 11944, 11945, 11946, 11947, 11948, 11949, 11950, 11951, 11952, 11953, 11954, 11955, 11956, 11957, 11958, 11959, 11960, 
    11975, 11976, 11977, 11978, 11979, 11980, 11981, 11982, 11983, 11984, 11985, 11986, 11987, 11988, 11989, 11990, 11991, 11992, 
    12007, 12008, 12009, 12010, 12011, 12012, 12013, 12014, 12015, 12016, 12017, 12018, 12019, 12020, 12021, 12022, 12023, 12024, 
    12039, 12040, 12041, 12042, 12043, 12044, 12045, 12046, 12047, 12048, 12049, 12050, 12051, 12052, 12053, 12054, 12055, 12056, 
    12071, 12072, 12073, 12074, 12075, 12076, 12077, 12078, 12079, 12080, 12081, 12082, 12083, 12084, 12085, 12086, 12087, 12088, 
    12103, 12104, 12105, 12106, 12107, 12108, 12109, 12110, 12111, 12112, 12113, 12114, 12115, 12116, 12117, 12118, 12119, 12120, 
    12135, 12136, 12137, 12138, 12139, 12140, 12141, 12142, 12143, 12144, 12145, 12146, 12147, 12148, 12149, 12150, 12151, 12152
  };
  
  
  // -- combination
  fChipIDs.insert(fChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer3Station1ChipIDs.begin(), fLayer3Station1ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer3Station2ChipIDs.begin(), fLayer3Station2ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer4Station0ChipIDs.begin(), fLayer4Station0ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer4Station1ChipIDs.begin(), fLayer4Station1ChipIDs.end());
  fChipIDs.insert(fChipIDs.end(), fLayer4Station2ChipIDs.begin(), fLayer4Station2ChipIDs.end());
  
  fCentral3LayerChipIDs.insert(fCentral3LayerChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
  fCentral3LayerChipIDs.insert(fCentral3LayerChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
  fCentral3LayerChipIDs.insert(fCentral3LayerChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
  
  fCentral4LayerChipIDs.insert(fCentral4LayerChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
  fCentral4LayerChipIDs.insert(fCentral4LayerChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
  fCentral4LayerChipIDs.insert(fCentral4LayerChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
  fCentral4LayerChipIDs.insert(fCentral4LayerChipIDs.end(), fLayer4Station0ChipIDs.begin(), fLayer4Station0ChipIDs.end());
}



// ----------------------------------------------------------------------
void cdbPayloadWriter::run(int argc, const char* argv[]) {
  string cal(""), payloaddir("."), inputfiledir(""), annotation(""), gt(""), filename("");
  int iov(1);
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-a"))  {annotation = argv[++i];}
    if (!strcmp(argv[i], "-c"))  {cal = argv[++i];}
    if (!strcmp(argv[i], "-d"))  {inputfiledir = argv[++i];}
    if (!strcmp(argv[i], "-i"))  {iov = atoi(argv[++i]);}
    if (!strcmp(argv[i], "-g"))  {gt = argv[++i];}
    if (!strcmp(argv[i], "-f"))  {filename = argv[++i];}
    if (!strcmp(argv[i], "-p"))  {payloaddir = argv[++i];}
  }
  
  cout << "======================" << endl;
  cout << "== cdbPayloadWriter ==" << endl;
  cout << "======================" << endl;
  cout << "== writing payload " << cal << " for global tag " << gt << endl;
  cout << "== installing in directory " << payloaddir << endl;
  cout << "== filename " << filename << endl;
  cout << "== iov " << iov << endl;
  cout << "== annotation " << annotation << endl << endl;
  
  if (inputfiledir != "") {
    vector<string> vfiles;
    DIR *folder = opendir(inputfiledir.c_str());
    if (folder == NULL) {
      cout << "Error failed to open " << inputfiledir << endl;
      return;
    }
    struct dirent *entry;
    while ((entry = readdir(folder))) {
      if (entry->d_type == DT_REG) {
        if ((cal == "eventstuffv1") && (string::npos == string(entry->d_name).find(".mid.lz4.json"))) continue;
        vfiles.push_back(inputfiledir + "/" + entry->d_name);
      }
    }
    closedir(folder);
    for (auto it: vfiles) {
      filename = it;
      string srun = filename;
      replaceAll(srun, ".mid.lz4.json", "");
      replaceAll(srun, inputfiledir, "");
      replaceAll(srun, "/", "");
      replaceAll(srun, "run", "");
      int irun = ::stoi(srun);
      cout << "filename = " << filename << " srun ->" << srun << "<- run = " << irun << endl;
      writeEventStuffV1Payloads(payloaddir, gt, filename, annotation, irun);
    }
    return;
  }
  
  if (string::npos != cal.find("alignment")) {
    writeAlignmentPayloads(payloaddir, gt, cal, filename, annotation, iov);
  }
  if ("alignment" == cal) {
    writeAlignmentPayloads(payloaddir, gt, "pixelalignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "tilealignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "fibrealignment", filename, annotation, iov);
    writeAlignmentPayloads(payloaddir, gt, "mppcalignment", filename, annotation, iov);
  }
  if (string::npos != cal.find("pixelqualitylm")) {
    writePixelQualityLMPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("fibrequality")) {
    writeFibreQualityPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("tilequality")) {
    writeTileQualityPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("pixelefficiency")) {
    writePixelEfficiencyPayloads(payloaddir, gt, filename, annotation, iov);
  }
  if (string::npos != cal.find("eventstuffv1")) {
    writeEventStuffV1Payloads(payloaddir, gt, filename, annotation, iov);
  }
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelTimeCalibrationIdealInput(string filename, string mode) {
  cout << "   ->cdbInitGT> writing local template pixeltimecalibration ideal input for mode = " << mode << endl;
  ofstream ONS;
  ONS.open(filename);

  vector<unsigned int> vChipIDs;
  fillChipIDs(vChipIDs, mode);

  cout << "   ->cdbPayloadWriter::writePixelTimeCalibrationIdealInput> writing " << vChipIDs.size() << " chipIDs to file " << filename << endl;
  for (auto &id : vChipIDs) {
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 32; j++) {
        ONS << id << " " << i << " " << j << " 0 0 0 0" << endl;
      }
    }
  }
  ONS.close();
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelEfficiencyPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelefficiency payloads" << endl;
  calPixelEfficiency *cpe = new calPixelEfficiency();
  string tmpFilename = "pixelefficiency_tmp.csv";
  if (string::npos != filename.find(".root")) {
    cout << "   ->cdbWritePayload> reading pixel chipIDs from root file " << filename << endl;
    TFile *file = TFile::Open(filename.c_str());
    TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
    unsigned int id;
    vector<unsigned int> vChipIDs;
    ta->SetBranchAddress("id", &id);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      vChipIDs.push_back(id);
    }
    cout << "   ->cdbWritePayload> read " << vChipIDs.size() << " chipIDs from tree with " << ta->GetEntries() << " entries" << endl;
    file->Close();
    ofstream ONS;
    ONS.open(tmpFilename);
    for (auto &id : vChipIDs) {
      ONS << id << "," << 18 << ",";
      for (int i = 0; i < 18; i++) {
        ONS << 1.000;
        if (i < 17) ONS << ",";
        else ONS << endl;
      }
    }
    ONS.close();
    filename = tmpFilename;
  }
  cpe->readCsv(filename);
  string spl = cpe->makeBLOB();
  string hash = "tag_pixelefficiency_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cpe->getSchema();
  pl.fBLOB = spl;
  cpe->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << endl;
  delete cpe;
  if (filename == tmpFilename) { 
    cout << "->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
    remove(tmpFilename.c_str()); 
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeDetSetupV1Payloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template detsetupv1 payloads" << endl;
  calDetSetupV1 *cdc = new calDetSetupV1();
  string result = cdc->readJSON(filename);
  string templateHash = "detsetupv1_";
  string hash = "tag_" + templateHash + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cdc->getSchema();
  pl.fBLOB = cdc->makeBLOB();
  cdc->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cdc;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeEventStuffV1Payloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template eventstuffv1 payloads" << endl;
  calEventStuffV1 *ces = new calEventStuffV1();
  ces->readJSON(filename);
  string hash = "tag_eventstuffv1_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = ces->getSchema();
  pl.fBLOB = ces->makeBLOB();
  ces->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and schema " << pl.fSchema << endl;
  delete ces;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelQualityLMPayloads(string payloaddir, string tag, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template pixelqualitylm payloads" 
  << " from file " << filename << " for tag " << tag << endl;
  calPixelQualityLM *cpq = new calPixelQualityLM();
  
  cpq->readCsv(filename);
  string spl = cpq->makeBLOB();
  string hash = "tag_pixelqualitylm_" + tag + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation + string(". ") + cpq->getStatusDocumentation();
  pl.fSchema  = cpq->getSchema();
  pl.fBLOB = spl;
  cpq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cpq;
  if (string::npos == filename.find(".root")) { 
    cout << "->cdbWritePayload> removing temporary file " << filename << endl; 
    remove(filename.c_str()); 
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelQualityLMIdealInput(string filename, std::string mode) {
  cout << "   ->cdbWritePayload::writePixelQualityLMIdealInput> writing pixel chipIDs to file " << filename 
      << " for mode " << mode
       << endl;
  
  vector<unsigned int> vChipIDs;
  fillChipIDs(vChipIDs, mode);
  
  ofstream ONS;
  ONS.open(filename);
  for (auto &id : vChipIDs) {
    ONS << id << "," << 31 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;
  }
  ONS.close();
  
};



// ----------------------------------------------------------------------
void cdbPayloadWriter::writeFibreQualityPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template fibrequality payloads" << endl;
  calFibreQuality *cfq = new calFibreQuality();
  cfq->readCSV(filename);
  string spl = cfq->makeBLOB();
  string hash = "tag_fibrequality_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cfq->getSchema();
  pl.fBLOB = spl;
  cfq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cfq;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writeTileQualityPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbInitGT> writing local template tilequality payloads" << endl;
  calTileQuality *ctq = new calTileQuality();
  ctq->readJSON(filename);
  string spl = ctq->makeBLOB();
  string hash = "tag_tilequality_" + gt + "_iov_" + to_string(iov);
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = ctq->getSchema();
  pl.fBLOB = spl;
  ctq->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbWritePayload> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete ctq;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::writePixelTimeCalibrationPayloads(string payloaddir, string gt, string filename, string annotation, int iov) {
  cout << "   ->cdbPayloadWriter::writePixelTimeCalibrationPayloads> writing local template pixeltimecalibration payloads" << endl;
  cout << "   ->cdbPayloadWriter::writePixelTimeCalibrationPayloads> reading pixeltimecalibration from file " << filename << endl;
  calPixelTimeCalibration *cpt = new calPixelTimeCalibration();
  cpt->readTxtFile(filename);
  string spl = cpt->makeBLOB();
  string hash = "tag_pixeltimecalibration_" + gt + "_iov_" + to_string(iov);
  
  payload pl;
  pl.fHash = hash;
  pl.fComment = annotation;
  pl.fSchema  = cpt->getSchema();
  pl.fBLOB = spl;
  cpt->writePayloadToFile(hash, payloaddir, pl);
  cout << "   ->cdbPayloadWriter::writePixelTimeCalibrationPayloads> writing IOV " << iov << " with " << hash << " and comment " << pl.fComment << endl;
  delete cpt;
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::writeAlignmentPayloads(string payloaddir, string gt, string type, string ifilename, string annotation, int iov) {
  cout << "   ->cdbWritePayload> writing alignment " << type << " from file " << ifilename 
  << " tag: " << gt << endl
  << " type: " << type << endl
  << " annotation: " << annotation << endl
  << " iov: " << iov << endl
  << " payloaddir: " << payloaddir << endl  
  << " ifilename: " << ifilename 
  << endl;
  

  string tmpFilename("");
  if (string::npos != ifilename.find(".root")) {
    tmpFilename = ifilename;
    size_t pos = tmpFilename.find(".root");
    if (pos != string::npos) tmpFilename.replace(pos, 5, "_tmp.csv");
    size_t lastSlash = tmpFilename.find_last_of("/");
    if (lastSlash != string::npos) tmpFilename = tmpFilename.substr(lastSlash + 1);
    cout << "   ->cdbWritePayload> temporary file " << tmpFilename << endl;
  }
  if (string::npos != type.find("pixelalignment")) {
    bool doFilter(false);
    vector<unsigned int> vFilter;
    if (string::npos != type.find(",")) {
      replaceAll(type, "pixelalignment", ""); 
      replaceAll(type, ",", ""); 
      fillChipIDs(vFilter, type);
      // if (string::npos != type.find("vtx") || string::npos != type.find("=2025")) {
      //   doFilter = true;
      //   vFilter.insert(vFilter.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
      //   vFilter.insert(vFilter.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
      // } else if (string::npos != type.find("central3")) {
      //   doFilter = true;
      //   vFilter.insert(vFilter.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
      //   vFilter.insert(vFilter.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
      //   vFilter.insert(vFilter.end(), fCentral3LayerChipIDs.begin(), fCentral3LayerChipIDs.end());
      // } else if (string::npos != type.find("central4")) {
      //   doFilter = true;
      //   vFilter.insert(vFilter.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
      //   vFilter.insert(vFilter.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
      //   vFilter.insert(vFilter.end(), fCentral3LayerChipIDs.begin(), fCentral3LayerChipIDs.end());
      //   vFilter.insert(vFilter.end(), fCentral4LayerChipIDs.begin(), fCentral4LayerChipIDs.end());
      // }
    }
    if (doFilter) {
      cout << "   ->cdbWritePayload> filtering pixelalignment for " << type << endl;
      cout << "   ->cdbWritePayload> number of chips to filter: " << vFilter.size() << endl;
    }
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading pixelalignment from root file " << ifilename << endl;
      struct sensor { unsigned int id; double vx, vy, vz; double rowx, rowy, rowz; double colx, coly, colz; int nrow, ncol; double width, length, thickness, pixelSize; };
      map<unsigned int, sensor> sensors;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
      struct sensor a;
      ta->SetBranchAddress("id", &a.id);
      ta->SetBranchAddress("vx", &a.vx); 
      ta->SetBranchAddress("vy", &a.vy); 
      ta->SetBranchAddress("vz", &a.vz);
      ta->SetBranchAddress("rowx", &a.rowx); 
      ta->SetBranchAddress("rowy", &a.rowy); 
      ta->SetBranchAddress("rowz", &a.rowz);
      ta->SetBranchAddress("colx", &a.colx); 
      ta->SetBranchAddress("coly", &a.coly); 
      ta->SetBranchAddress("colz", &a.colz);
      ta->SetBranchAddress("nrow", &a.nrow); 
      ta->SetBranchAddress("ncol", &a.ncol);
      ta->SetBranchAddress("width", &a.width); 
      ta->SetBranchAddress("length", &a.length);
      ta->SetBranchAddress("thickness", &a.thickness); 
      ta->SetBranchAddress("pixelSize", &a.pixelSize);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        if (doFilter) {
          if (find(vFilter.begin(), vFilter.end(), a.id) == vFilter.end()) continue;
        }
        sensors.insert(make_pair(a.id, a)); 
      }
      cout << "   ->cdbWritePayload> read " << sensors.size() << " sensors" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &s : sensors) {
        ONS << s.first << "," << std::setprecision(15) << s.second.vx << "," << s.second.vy << "," << s.second.vz << ","
        << s.second.rowx << "," << s.second.rowy << "," << s.second.rowz << "," << s.second.colx << "," << s.second.coly << "," << s.second.colz
        << "," << s.second.nrow << "," << s.second.ncol << "," << s.second.width << "," << s.second.length << "," << s.second.thickness << "," << s.second.pixelSize << endl;
      }
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calPixelAlignment *cpa = new calPixelAlignment();
    string result = cpa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cpa->makeBLOB();
      string hash = "tag_pixelalignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cpa->getSchema(); pl.fBLOB = spl;
      cpa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; remove(tmpFilename.c_str()); }
  }
  
  if (string::npos != type.find("mppcalignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading mppcalignment from root file " << ifilename << endl;
      struct mppc { unsigned int mppc; double vx, vy, vz; double colx, coly, colz; int ncol; };
      map<unsigned int, mppc> mppcs;
      struct mppc m;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/mppcs"));
      ta->SetBranchAddress("mppc", &m.mppc); 
      ta->SetBranchAddress("vx", &m.vx); 
      ta->SetBranchAddress("vy", &m.vy); 
      ta->SetBranchAddress("vz", &m.vz);
      ta->SetBranchAddress("colx", &m.colx); 
      ta->SetBranchAddress("coly", &m.coly); 
      ta->SetBranchAddress("colz", &m.colz); 
      ta->SetBranchAddress("ncol", &m.ncol);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        mppcs.insert(make_pair(m.mppc, m)); 
      }
      cout << "   ->cdbWritePayload> read " << mppcs.size() << " mppcs" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &m : mppcs) ONS << m.first << "," << std::setprecision(15) << m.second.vx << "," << m.second.vy << "," << m.second.vz << "," << m.second.colx << "," << m.second.coly << "," << m.second.colz << "," << m.second.ncol << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calMppcAlignment *cmp = new calMppcAlignment();
    string result = cmp->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cmp->makeBLOB();
      string hash = "tag_mppcalignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cmp->getSchema(); pl.fBLOB = spl;
      cmp->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cmp;
  }
  
  if (string::npos != type.find("tilealignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading tilealignment from root file " << ifilename << endl;
      struct tile { unsigned int id; double posx, posy, posz; double dirx, diry, dirz; };
      map<unsigned int, tile> tiles;
      struct tile t;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/tiles"));
      ta->SetBranchAddress("id", &t.id); 
      ta->SetBranchAddress("posx", &t.posx); 
      ta->SetBranchAddress("posy", &t.posy); 
      ta->SetBranchAddress("posz", &t.posz);
      ta->SetBranchAddress("dirx", &t.dirx); 
      ta->SetBranchAddress("diry", &t.diry); 
      ta->SetBranchAddress("dirz", &t.dirz);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        tiles.insert(make_pair(t.id, t)); 
      }
      cout << "   ->cdbWritePayload> read " << tiles.size() << " tiles" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &t : tiles) ONS << t.first << "," << t.first << "," << std::setprecision(15) << t.second.posx << "," << t.second.posy << "," << t.second.posz << "," << t.second.dirx << "," << t.second.diry << "," << t.second.dirz << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calTileAlignment *cta = new calTileAlignment();
    string result = cta->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cta->makeBLOB();
      string hash = "tag_tilealignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cta->getSchema(); pl.fBLOB = spl;
      cta->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cta;
  }
  
  if (string::npos != type.find("fibrealignment")) {
    if (string::npos != ifilename.find(".root")) {
      cout << "   ->cdbWritePayload> reading fibrealignment from root file " << ifilename << endl;
      struct fibre { unsigned int fibre; double cx, cy, cz; double fx, fy, fz; bool round, square; double diameter; };
      map<unsigned int, fibre> fibres;
      struct fibre f;
      TFile *file = TFile::Open(ifilename.c_str());
      TTree *ta = static_cast<TTree*>(file->Get("alignment/fibres"));
      ta->SetBranchAddress("fibre", &f.fibre); 
      ta->SetBranchAddress("cx", &f.cx); 
      ta->SetBranchAddress("cy", &f.cy); 
      ta->SetBranchAddress("cz", &f.cz);
      ta->SetBranchAddress("fx", &f.fx); 
      ta->SetBranchAddress("fy", &f.fy); 
      ta->SetBranchAddress("fz", &f.fz);
      f.round = true; 
      f.square = false; 
      ta->SetBranchAddress("diameter", &f.diameter);
      for (int i = 0; i < ta->GetEntries(); ++i) { 
        ta->GetEntry(i); 
        fibres.insert(make_pair(f.fibre, f)); 
      }
      cout << "   ->cdbWritePayload> read " << fibres.size() << " fibres" << endl;
      ofstream ONS; ONS.open(tmpFilename);
      for (auto &f : fibres) ONS << f.first << "," << std::setprecision(15) << f.second.cx << "," << f.second.cy << "," << f.second.cz << "," << f.second.fx << "," << f.second.fy << "," << f.second.fz << "," << f.second.round << "," << f.second.square << "," << f.second.diameter << endl;
      ONS.close(); file->Close();
    }
    if (string::npos != ifilename.find(".root")) ifilename = tmpFilename;
    calFibreAlignment *cfa = new calFibreAlignment();
    string result = cfa->readCsv(ifilename);
    if (string::npos == result.find("Error")) {
      string spl = cfa->makeBLOB();
      string hash = "tag_fibrealignment_" + gt + "_iov_" + to_string(iov);
      payload pl; pl.fHash = hash; pl.fComment = annotation; pl.fSchema = cfa->getSchema(); pl.fBLOB = spl;
      cfa->writePayloadToFile(hash, payloaddir, pl);
    }
    if (string::npos != ifilename.find(".root")) { 
      cout << "   ->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
      remove(tmpFilename.c_str()); 
    }
    delete cfa;
  }
  if (string::npos == ifilename.find(".root")) { 
    cout << "->cdbWritePayload> removing temporary file " << tmpFilename << endl; 
    remove(tmpFilename.c_str()); 
  }
}


// ----------------------------------------------------------------------
void cdbPayloadWriter::fillChipIDs(std::vector<unsigned int> &vChipIDs, std::string mode) {
  vChipIDs.clear();
  // -- Note: the order is significant. First years and other restrictions. Then "ideal"
  if (mode == "all") {
    vChipIDs.insert(vChipIDs.end(), fChipIDs.begin(), fChipIDs.end());
  } else if ((mode.find("vtx") != string::npos) || (mode.find("2025") != string::npos)) {
    vChipIDs.insert(vChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
  } else if (mode.find("central3") != string::npos) {
    vChipIDs.insert(vChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
  } else if (mode.find("central4") != string::npos) {
    vChipIDs.insert(vChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer4Station0ChipIDs.begin(), fLayer4Station0ChipIDs.end());
  } else if (mode.find("ideal") != string::npos) {
    vChipIDs.insert(vChipIDs.end(), fLayer1ChipIDs.begin(), fLayer1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer2ChipIDs.begin(), fLayer2ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer3Station0ChipIDs.begin(), fLayer3Station0ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer3Station1ChipIDs.begin(), fLayer3Station1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer3Station2ChipIDs.begin(), fLayer3Station2ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer4Station0ChipIDs.begin(), fLayer4Station0ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer4Station1ChipIDs.begin(), fLayer4Station1ChipIDs.end());
    vChipIDs.insert(vChipIDs.end(), fLayer4Station2ChipIDs.begin(), fLayer4Station2ChipIDs.end());
  } else {
    cout << "Error: invalid mode " << mode << endl;
    return;
  }
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::createChipIDsPerLayer(string inputfilename) {
  cout << "   ->cdbPayloadWriter> creating chip IDs per layer from input file " << inputfilename << endl;
  
  fLayer1ChipIDs.clear();
  fLayer2ChipIDs.clear();
  fLayer3Station0ChipIDs.clear();
  fLayer3Station1ChipIDs.clear();
  fLayer3Station2ChipIDs.clear();
  fLayer4Station0ChipIDs.clear();
  fLayer4Station1ChipIDs.clear();
  fLayer4Station2ChipIDs.clear();
  if (string::npos != inputfilename.find(".csv")) {
    cout << "Error: input filename " << inputfilename << " is a .csv file" << endl;
    cout << "Usage: cdbWriteIdealInputFiles -m createchipidsperlayer -f filename.root" << endl;
    return;
  } else if (string::npos != inputfilename.find(".root")) {
    TFile *file = TFile::Open(inputfilename.c_str());
    TTree *ta = static_cast<TTree*>(file->Get("alignment/sensors"));
    unsigned int id, station, z, zPrime;
    double vz;
    ta->SetBranchAddress("id", &id);
    ta->SetBranchAddress("vz", &vz);
    for (int i = 0; i < ta->GetEntries(); ++i) {
      ta->GetEntry(i);
      // -- specbook: Section 4.1.7 "Pixel numbering and naming"
      station = (id/4096);
      int layer = (id/1024)%4 + 1;     
      zPrime = id % 32;
      
      if (layer == 1) {
        fLayer1ChipIDs.push_back(id);
        z = zPrime;
      } else if (layer == 2) {
        fLayer2ChipIDs.push_back(id);
        z = zPrime;
      } else if (layer == 3) {
        if (station == 0) {
          fLayer3Station0ChipIDs.push_back(id);
        } else if (station == 1) {
          fLayer3Station1ChipIDs.push_back(id);
        } else if (station == 2) {
          fLayer3Station2ChipIDs.push_back(id);
        }
        z = zPrime - 7;
      } else if (layer == 4) {
        if (station == 0) {
          fLayer4Station0ChipIDs.push_back(id);
        } else if (station == 1) {
          fLayer4Station1ChipIDs.push_back(id);
        } else if (station == 2) {
          fLayer4Station2ChipIDs.push_back(id);
        }
        z = zPrime - 6;
      }
      cout << "id = " << id << " station = " << station << " z = " << z << " vz = " << vz << endl;
    }
    file->Close();
  } else {
    cout << "Error: input filename " << inputfilename << " is not a .csv or .root file" << endl;
    return;
  } 
  
  cout << "  // ->cdbPayloadWriter> read " << fLayer1ChipIDs.size() << " layer 1 chip IDs" << endl;
  cout << "  fLayer1ChipIDs = {" << endl << "    ";
  unsigned long cnt(0);
  for (auto &id : fLayer1ChipIDs) {
    if (cnt < fLayer1ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 12) == 0) cout << endl << "    ";
  }
  cout << endl;
  
  cout << "  // ->cdbPayloadWriter> read " << fLayer2ChipIDs.size() << " layer 2 chip IDs" << endl;
  cout << "  fLayer2ChipIDs = {" << endl << "    ";
  cnt = 0; 
  for (auto &id : fLayer2ChipIDs) {
    if (cnt < fLayer2ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 12) == 0) cout << endl << "    ";
  }
  cout << endl;
  
  cout << "  // ->cdbPayloadWriter> read " << fLayer3Station0ChipIDs.size() << " layer 3 station 0 (central) chip IDs" << endl;
  cout << "  fLayer3Station0ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer3Station0ChipIDs) {
    if (cnt < fLayer3Station0ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 17) == 0) cout << endl << "    ";
  }
  cout << endl;
  cout << "  // ->cdbPayloadWriter> read " << fLayer3Station1ChipIDs.size() << " layer 3 station 1 (US) chip IDs" << endl;
  cout << "  fLayer3Station1ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer3Station1ChipIDs) {
    if (cnt < fLayer3Station1ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 17) == 0) cout << endl << "    ";
  }
  cout << endl;
  cout << "  // ->cdbPayloadWriter> read " << fLayer3Station2ChipIDs.size() << " layer 3 station 2 (DS) chip IDs" << endl;
  cout << "  fLayer3Station2ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer3Station2ChipIDs) {
    if (cnt < fLayer3Station2ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 17) == 0) cout << endl << "    ";
  }
  cout << endl;
  
  cout << "  // ->cdbPayloadWriter> read " << fLayer4Station0ChipIDs.size() << " layer 4 station 0 (central) chip IDs" << endl;
  cout << "  fLayer4Station0ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer4Station0ChipIDs) {
    if (cnt < fLayer4Station0ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 18) == 0) cout << endl << "    ";
  }
  cout << endl;
  cout << "  // ->cdbPayloadWriter> read " << fLayer4Station1ChipIDs.size() << " layer 4 station 1 (US) chip IDs" << endl;
  cout << "  fLayer4Station1ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer4Station1ChipIDs) {
    if (cnt < fLayer4Station1ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 18) == 0) cout << endl << "    ";
  }
  cout << endl;
  cout << "  // ->cdbPayloadWriter> read " << fLayer4Station2ChipIDs.size() << " layer 4 station 2 (DS) chip IDs" << endl;
  cout << "  fLayer4Station2ChipIDs = {" << endl << "    ";
  cnt = 0;
  for (auto &id : fLayer4Station2ChipIDs) {
    if (cnt < fLayer4Station2ChipIDs.size() - 1) cout << id << ", ";
    else cout << id << endl << "  };" << endl;
    cnt++;
    if ((cnt % 18) == 0) cout << endl << "    ";
  }
  cout << endl;
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::createFibreIDs(string inputfilename) {
  cout << "   ->cdbPayloadWriter> creating fibre IDs from input file " << inputfilename << endl;
  //  fFibreIDs.clear();
}

// ----------------------------------------------------------------------
void cdbPayloadWriter::createTileIDs(string inputfilename) {
  cout << "   ->cdbPayloadWriter> creating tile IDs from input file " << inputfilename << endl;
  //  fTileIDs.clear();
}