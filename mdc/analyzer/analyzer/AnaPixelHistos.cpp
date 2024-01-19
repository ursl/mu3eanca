#include "AnaPixelHistos.h"

#include <iostream>

#include "HitVectorFlowEvent.h"

#include "TH1F.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TH2D.h"

#include "root_helpers.h"

using namespace std;

// ----------------------------------------------------------------------    
AnaPixelHistos::AnaPixelHistos(TARunInfo* runinfo):
  TARunObject(runinfo){
  fModuleName = "PixelHistos";
}

// ----------------------------------------------------------------------    
AnaPixelHistos::~AnaPixelHistos(){};

// ----------------------------------------------------------------------    
void AnaPixelHistos::BeginRun(TARunInfo* runinfo) {
  printf("PixelHistos::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

  char xfilename[256];
  sprintf(xfilename, "root_output_files/pixel_histos%05d.root", runinfo->fRunNo);
  dataFile = new TFile(xfilename, "RECREATE");

  // select correct ROOT directory
  /*TDirectory *d_root =*/ make_or_get_dir("", gDirectory);

  // histograms
  chipID          = new TH1F("chipID", "chipID", 1024, 0, 1<<16);

  vector<unsigned int> chipids{
    2,3,4,5,6,7,34,35,36,37,38,39,
    66,67,68,69,70,71,98,99,100,101,102,103,
    130,131,132,133,134,135,162,163,164,165,166,167,
    194,195,196,197,198,199,226,227,228,229,230,231,
    1026,1027,1028,1029,1030,1031,1058,1059,1060,1061,1062,1063,
    1090,1091,1092,1093,1094,1095,1122,1123,1124,1125,1126,1127,
    1154,1155,1156,1157,1158,1159,1186,1187,1188,1189,1190,1191,
    1218,1219,1220,1221,1222,1223,1250,1251,1252,1253,1254,1255,
    1282,1283,1284,1285,1286,1287,1314,1315,1316,1317,1318,1319,
    2057,2058,2059,2060,2061,2062,2063,2064,2065,2066,2067,2068,
    2069,2070,2071,2072,2073,2089,2090,2091,2092,2093,2094,2095,
    2096,2097,2098,2099,2100,2101,2102,2103,2104,2105,2121,2122,
    2123,2124,2125,2126,2127,2128,2129,2130,2131,2132,2133,2134,
    2135,2136,2137,2153,2154,2155,2156,2157,2158,2159,2160,2161,
    2162,2163,2164,2165,2166,2167,2168,2169,2185,2186,2187,2188,
    2189,2190,2191,2192,2193,2194,2195,2196,2197,2198,2199,2200,
    2201,2217,2218,2219,2220,2221,2222,2223,2224,2225,2226,2227,
    2228,2229,2230,2231,2232,2233,2249,2250,2251,2252,2253,2254,
    2255,2256,2257,2258,2259,2260,2261,2262,2263,2264,2265,2281,
    2282,2283,2284,2285,2286,2287,2288,2289,2290,2291,2292,2293,
    2294,2295,2296,2297,2313,2314,2315,2316,2317,2318,2319,2320,
    2321,2322,2323,2324,2325,2326,2327,2328,2329,2345,2346,2347,
    2348,2349,2350,2351,2352,2353,2354,2355,2356,2357,2358,2359,
    2360,2361,2377,2378,2379,2380,2381,2382,2383,2384,2385,2386,
    2387,2388,2389,2390,2391,2392,2393,2409,2410,2411,2412,2413,
    2414,2415,2416,2417,2418,2419,2420,2421,2422,2423,2424,2425,
    2441,2442,2443,2444,2445,2446,2447,2448,2449,2450,2451,2452,
    2453,2454,2455,2456,2457,2473,2474,2475,2476,2477,2478,2479,
    2480,2481,2482,2483,2484,2485,2486,2487,2488,2489,2505,2506,
    2507,2508,2509,2510,2511,2512,2513,2514,2515,2516,2517,2518,
    2519,2520,2521,2537,2538,2539,2540,2541,2542,2543,2544,2545,
    2546,2547,2548,2549,2550,2551,2552,2553,2569,2570,2571,2572,
    2573,2574,2575,2576,2577,2578,2579,2580,2581,2582,2583,2584,
    2585,2601,2602,2603,2604,2605,2606,2607,2608,2609,2610,2611,
    2612,2613,2614,2615,2616,2617,2633,2634,2635,2636,2637,2638,
    2639,2640,2641,2642,2643,2644,2645,2646,2647,2648,2649,2665,
    2666,2667,2668,2669,2670,2671,2672,2673,2674,2675,2676,2677,
    2678,2679,2680,2681,2697,2698,2699,2700,2701,2702,2703,2704,
    2705,2706,2707,2708,2709,2710,2711,2712,2713,2729,2730,2731,
    2732,2733,2734,2735,2736,2737,2738,2739,2740,2741,2742,2743,
    2744,2745,2761,2762,2763,2764,2765,2766,2767,2768,2769,2770,
    2771,2772,2773,2774,2775,2776,2777,2793,2794,2795,2796,2797,
    2798,2799,2800,2801,2802,2803,2804,2805,2806,2807,2808,2809,
    3080,3081,3082,3083,3084,3085,3086,3087,3088,3089,3090,3091,
    3092,3093,3094,3095,3096,3097,3112,3113,3114,3115,3116,3117,
    3118,3119,3120,3121,3122,3123,3124,3125,3126,3127,3128,3129,
    3144,3145,3146,3147,3148,3149,3150,3151,3152,3153,3154,3155,
    3156,3157,3158,3159,3160,3161,3176,3177,3178,3179,3180,3181,
    3182,3183,3184,3185,3186,3187,3188,3189,3190,3191,3192,3193,
    3208,3209,3210,3211,3212,3213,3214,3215,3216,3217,3218,3219,
    3220,3221,3222,3223,3224,3225,3240,3241,3242,3243,3244,3245,
    3246,3247,3248,3249,3250,3251,3252,3253,3254,3255,3256,3257,
    3272,3273,3274,3275,3276,3277,3278,3279,3280,3281,3282,3283,
    3284,3285,3286,3287,3288,3289,3304,3305,3306,3307,3308,3309,
    3310,3311,3312,3313,3314,3315,3316,3317,3318,3319,3320,3321,
    3336,3337,3338,3339,3340,3341,3342,3343,3344,3345,3346,3347,
    3348,3349,3350,3351,3352,3353,3368,3369,3370,3371,3372,3373,
    3374,3375,3376,3377,3378,3379,3380,3381,3382,3383,3384,3385,
    3400,3401,3402,3403,3404,3405,3406,3407,3408,3409,3410,3411,
    3412,3413,3414,3415,3416,3417,3432,3433,3434,3435,3436,3437,
    3438,3439,3440,3441,3442,3443,3444,3445,3446,3447,3448,3449,
    3464,3465,3466,3467,3468,3469,3470,3471,3472,3473,3474,3475,
    3476,3477,3478,3479,3480,3481,3496,3497,3498,3499,3500,3501,
    3502,3503,3504,3505,3506,3507,3508,3509,3510,3511,3512,3513,
    3528,3529,3530,3531,3532,3533,3534,3535,3536,3537,3538,3539,
    3540,3541,3542,3543,3544,3545,3560,3561,3562,3563,3564,3565,
    3566,3567,3568,3569,3570,3571,3572,3573,3574,3575,3576,3577,
    3592,3593,3594,3595,3596,3597,3598,3599,3600,3601,3602,3603,
    3604,3605,3606,3607,3608,3609,3624,3625,3626,3627,3628,3629,
    3630,3631,3632,3633,3634,3635,3636,3637,3638,3639,3640,3641,
    3656,3657,3658,3659,3660,3661,3662,3663,3664,3665,3666,3667,
    3668,3669,3670,3671,3672,3673,3688,3689,3690,3691,3692,3693,
    3694,3695,3696,3697,3698,3699,3700,3701,3702,3703,3704,3705,
    3720,3721,3722,3723,3724,3725,3726,3727,3728,3729,3730,3731,
    3732,3733,3734,3735,3736,3737,3752,3753,3754,3755,3756,3757,
    3758,3759,3760,3761,3762,3763,3764,3765,3766,3767,3768,3769,
    3784,3785,3786,3787,3788,3789,3790,3791,3792,3793,3794,3795,
    3796,3797,3798,3799,3800,3801,3816,3817,3818,3819,3820,3821,
    3822,3823,3824,3825,3826,3827,3828,3829,3830,3831,3832,3833,
    3848,3849,3850,3851,3852,3853,3854,3855,3856,3857,3858,3859,
    3860,3861,3862,3863,3864,3865,3880,3881,3882,3883,3884,3885,
    3886,3887,3888,3889,3890,3891,3892,3893,3894,3895,3896,3897,
    3912,3913,3914,3915,3916,3917,3918,3919,3920,3921,3922,3923,
    3924,3925,3926,3927,3928,3929,3944,3945,3946,3947,3948,3949,
    3950,3951,3952,3953,3954,3955,3956,3957,3958,3959,3960,3961,
    10249,10250,10251,10252,10253,10254,10255,10256,10257,10258,10259,10260,
    10261,10262,10263,10264,10265,10281,10282,10283,10284,10285,10286,10287,
    10288,10289,10290,10291,10292,10293,10294,10295,10296,10297,10313,10314,
    10315,10316,10317,10318,10319,10320,10321,10322,10323,10324,10325,10326,
    10327,10328,10329,10345,10346,10347,10348,10349,10350,10351,10352,10353,
    10354,10355,10356,10357,10358,10359,10360,10361,10377,10378,10379,10380,
    10381,10382,10383,10384,10385,10386,10387,10388,10389,10390,10391,10392,
    10393,10409,10410,10411,10412,10413,10414,10415,10416,10417,10418,10419,
    10420,10421,10422,10423,10424,10425,10441,10442,10443,10444,10445,10446,
    10447,10448,10449,10450,10451,10452,10453,10454,10455,10456,10457,10473,
    10474,10475,10476,10477,10478,10479,10480,10481,10482,10483,10484,10485,
    10486,10487,10488,10489,10505,10506,10507,10508,10509,10510,10511,10512,
    10513,10514,10515,10516,10517,10518,10519,10520,10521,10537,10538,10539,
    10540,10541,10542,10543,10544,10545,10546,10547,10548,10549,10550,10551,
    10552,10553,10569,10570,10571,10572,10573,10574,10575,10576,10577,10578,
    10579,10580,10581,10582,10583,10584,10585,10601,10602,10603,10604,10605,
    10606,10607,10608,10609,10610,10611,10612,10613,10614,10615,10616,10617,
    10633,10634,10635,10636,10637,10638,10639,10640,10641,10642,10643,10644,
    10645,10646,10647,10648,10649,10665,10666,10667,10668,10669,10670,10671,
    10672,10673,10674,10675,10676,10677,10678,10679,10680,10681,10697,10698,
    10699,10700,10701,10702,10703,10704,10705,10706,10707,10708,10709,10710,
    10711,10712,10713,10729,10730,10731,10732,10733,10734,10735,10736,10737,
    10738,10739,10740,10741,10742,10743,10744,10745,10761,10762,10763,10764,
    10765,10766,10767,10768,10769,10770,10771,10772,10773,10774,10775,10776,
    10777,10793,10794,10795,10796,10797,10798,10799,10800,10801,10802,10803,
    10804,10805,10806,10807,10808,10809,10825,10826,10827,10828,10829,10830,
    10831,10832,10833,10834,10835,10836,10837,10838,10839,10840,10841,10857,
    10858,10859,10860,10861,10862,10863,10864,10865,10866,10867,10868,10869,
    10870,10871,10872,10873,10889,10890,10891,10892,10893,10894,10895,10896,
    10897,10898,10899,10900,10901,10902,10903,10904,10905,10921,10922,10923,
    10924,10925,10926,10927,10928,10929,10930,10931,10932,10933,10934,10935,
    10936,10937,10953,10954,10955,10956,10957,10958,10959,10960,10961,10962,
    10963,10964,10965,10966,10967,10968,10969,10985,10986,10987,10988,10989,
    10990,10991,10992,10993,10994,10995,10996,10997,10998,10999,11000,11001,
    11272,11273,11274,11275,11276,11277,11278,11279,11280,11281,11282,11283,
    11284,11285,11286,11287,11288,11289,11304,11305,11306,11307,11308,11309,
    11310,11311,11312,11313,11314,11315,11316,11317,11318,11319,11320,11321,
    11336,11337,11338,11339,11340,11341,11342,11343,11344,11345,11346,11347,
    11348,11349,11350,11351,11352,11353,11368,11369,11370,11371,11372,11373,
    11374,11375,11376,11377,11378,11379,11380,11381,11382,11383,11384,11385,
    11400,11401,11402,11403,11404,11405,11406,11407,11408,11409,11410,11411,
    11412,11413,11414,11415,11416,11417,11432,11433,11434,11435,11436,11437,
    11438,11439,11440,11441,11442,11443,11444,11445,11446,11447,11448,11449,
    11464,11465,11466,11467,11468,11469,11470,11471,11472,11473,11474,11475,
    11476,11477,11478,11479,11480,11481,11496,11497,11498,11499,11500,11501,
    11502,11503,11504,11505,11506,11507,11508,11509,11510,11511,11512,11513,
    11528,11529,11530,11531,11532,11533,11534,11535,11536,11537,11538,11539,
    11540,11541,11542,11543,11544,11545,11560,11561,11562,11563,11564,11565,
    11566,11567,11568,11569,11570,11571,11572,11573,11574,11575,11576,11577,
    11592,11593,11594,11595,11596,11597,11598,11599,11600,11601,11602,11603,
    11604,11605,11606,11607,11608,11609,11624,11625,11626,11627,11628,11629,
    11630,11631,11632,11633,11634,11635,11636,11637,11638,11639,11640,11641,
    11656,11657,11658,11659,11660,11661,11662,11663,11664,11665,11666,11667,
    11668,11669,11670,11671,11672,11673,11688,11689,11690,11691,11692,11693,
    11694,11695,11696,11697,11698,11699,11700,11701,11702,11703,11704,11705,
    11720,11721,11722,11723,11724,11725,11726,11727,11728,11729,11730,11731,
    11732,11733,11734,11735,11736,11737,11752,11753,11754,11755,11756,11757,
    11758,11759,11760,11761,11762,11763,11764,11765,11766,11767,11768,11769,
    11784,11785,11786,11787,11788,11789,11790,11791,11792,11793,11794,11795,
    11796,11797,11798,11799,11800,11801,11816,11817,11818,11819,11820,11821,
    11822,11823,11824,11825,11826,11827,11828,11829,11830,11831,11832,11833,
    11848,11849,11850,11851,11852,11853,11854,11855,11856,11857,11858,11859,
    11860,11861,11862,11863,11864,11865,11880,11881,11882,11883,11884,11885,
    11886,11887,11888,11889,11890,11891,11892,11893,11894,11895,11896,11897,
    11912,11913,11914,11915,11916,11917,11918,11919,11920,11921,11922,11923,
    11924,11925,11926,11927,11928,11929,11944,11945,11946,11947,11948,11949,
    11950,11951,11952,11953,11954,11955,11956,11957,11958,11959,11960,11961,
    11976,11977,11978,11979,11980,11981,11982,11983,11984,11985,11986,11987,
    11988,11989,11990,11991,11992,11993,12008,12009,12010,12011,12012,12013,
    12014,12015,12016,12017,12018,12019,12020,12021,12022,12023,12024,12025,
    12040,12041,12042,12043,12044,12045,12046,12047,12048,12049,12050,12051,
    12052,12053,12054,12055,12056,12057,12072,12073,12074,12075,12076,12077,
    12078,12079,12080,12081,12082,12083,12084,12085,12086,12087,12088,12089,
    12104,12105,12106,12107,12108,12109,12110,12111,12112,12113,12114,12115,
    12116,12117,12118,12119,12120,12121,12136,12137,12138,12139,12140,12141,
    12142,12143,12144,12145,12146,12147,12148,12149,12150,12151,12152,12153,
    14345,14346,14347,14348,14349,14350,14351,14352,14353,14354,14355,14356,
    14357,14358,14359,14360,14361,14377,14378,14379,14380,14381,14382,14383,
    14384,14385,14386,14387,14388,14389,14390,14391,14392,14393,14409,14410,
    14411,14412,14413,14414,14415,14416,14417,14418,14419,14420,14421,14422,
    14423,14424,14425,14441,14442,14443,14444,14445,14446,14447,14448,14449,
    14450,14451,14452,14453,14454,14455,14456,14457,14473,14474,14475,14476,
    14477,14478,14479,14480,14481,14482,14483,14484,14485,14486,14487,14488,
    14489,14505,14506,14507,14508,14509,14510,14511,14512,14513,14514,14515,
    14516,14517,14518,14519,14520,14521,14537,14538,14539,14540,14541,14542,
    14543,14544,14545,14546,14547,14548,14549,14550,14551,14552,14553,14569,
    14570,14571,14572,14573,14574,14575,14576,14577,14578,14579,14580,14581,
    14582,14583,14584,14585,14601,14602,14603,14604,14605,14606,14607,14608,
    14609,14610,14611,14612,14613,14614,14615,14616,14617,14633,14634,14635,
    14636,14637,14638,14639,14640,14641,14642,14643,14644,14645,14646,14647,
    14648,14649,14665,14666,14667,14668,14669,14670,14671,14672,14673,14674,
    14675,14676,14677,14678,14679,14680,14681,14697,14698,14699,14700,14701,
    14702,14703,14704,14705,14706,14707,14708,14709,14710,14711,14712,14713,
    14729,14730,14731,14732,14733,14734,14735,14736,14737,14738,14739,14740,
    14741,14742,14743,14744,14745,14761,14762,14763,14764,14765,14766,14767,
    14768,14769,14770,14771,14772,14773,14774,14775,14776,14777,14793,14794,
    14795,14796,14797,14798,14799,14800,14801,14802,14803,14804,14805,14806,
    14807,14808,14809,14825,14826,14827,14828,14829,14830,14831,14832,14833,
    14834,14835,14836,14837,14838,14839,14840,14841,14857,14858,14859,14860,
    14861,14862,14863,14864,14865,14866,14867,14868,14869,14870,14871,14872,
    14873,14889,14890,14891,14892,14893,14894,14895,14896,14897,14898,14899,
    14900,14901,14902,14903,14904,14905,14921,14922,14923,14924,14925,14926,
    14927,14928,14929,14930,14931,14932,14933,14934,14935,14936,14937,14953,
    14954,14955,14956,14957,14958,14959,14960,14961,14962,14963,14964,14965,
    14966,14967,14968,14969,14985,14986,14987,14988,14989,14990,14991,14992,
    14993,14994,14995,14996,14997,14998,14999,15000,15001,15017,15018,15019,
    15020,15021,15022,15023,15024,15025,15026,15027,15028,15029,15030,15031,
    15032,15033,15049,15050,15051,15052,15053,15054,15055,15056,15057,15058,
    15059,15060,15061,15062,15063,15064,15065,15081,15082,15083,15084,15085,
    15086,15087,15088,15089,15090,15091,15092,15093,15094,15095,15096,15097,
    15368,15369,15370,15371,15372,15373,15374,15375,15376,15377,15378,15379,
    15380,15381,15382,15383,15384,15385,15400,15401,15402,15403,15404,15405,
    15406,15407,15408,15409,15410,15411,15412,15413,15414,15415,15416,15417,
    15432,15433,15434,15435,15436,15437,15438,15439,15440,15441,15442,15443,
    15444,15445,15446,15447,15448,15449,15464,15465,15466,15467,15468,15469,
    15470,15471,15472,15473,15474,15475,15476,15477,15478,15479,15480,15481,
    15496,15497,15498,15499,15500,15501,15502,15503,15504,15505,15506,15507,
    15508,15509,15510,15511,15512,15513,15528,15529,15530,15531,15532,15533,
    15534,15535,15536,15537,15538,15539,15540,15541,15542,15543,15544,15545,
    15560,15561,15562,15563,15564,15565,15566,15567,15568,15569,15570,15571,
    15572,15573,15574,15575,15576,15577,15592,15593,15594,15595,15596,15597,
    15598,15599,15600,15601,15602,15603,15604,15605,15606,15607,15608,15609,
    15624,15625,15626,15627,15628,15629,15630,15631,15632,15633,15634,15635,
    15636,15637,15638,15639,15640,15641,15656,15657,15658,15659,15660,15661,
    15662,15663,15664,15665,15666,15667,15668,15669,15670,15671,15672,15673,
    15688,15689,15690,15691,15692,15693,15694,15695,15696,15697,15698,15699,
    15700,15701,15702,15703,15704,15705,15720,15721,15722,15723,15724,15725,
    15726,15727,15728,15729,15730,15731,15732,15733,15734,15735,15736,15737,
    15752,15753,15754,15755,15756,15757,15758,15759,15760,15761,15762,15763,
    15764,15765,15766,15767,15768,15769,15784,15785,15786,15787,15788,15789,
    15790,15791,15792,15793,15794,15795,15796,15797,15798,15799,15800,15801,
    15816,15817,15818,15819,15820,15821,15822,15823,15824,15825,15826,15827,
    15828,15829,15830,15831,15832,15833,15848,15849,15850,15851,15852,15853,
    15854,15855,15856,15857,15858,15859,15860,15861,15862,15863,15864,15865,
    15880,15881,15882,15883,15884,15885,15886,15887,15888,15889,15890,15891,
    15892,15893,15894,15895,15896,15897,15912,15913,15914,15915,15916,15917,
    15918,15919,15920,15921,15922,15923,15924,15925,15926,15927,15928,15929,
    15944,15945,15946,15947,15948,15949,15950,15951,15952,15953,15954,15955,
    15956,15957,15958,15959,15960,15961,15976,15977,15978,15979,15980,15981,
    15982,15983,15984,15985,15986,15987,15988,15989,15990,15991,15992,15993,
    16008,16009,16010,16011,16012,16013,16014,16015,16016,16017,16018,16019,
    16020,16021,16022,16023,16024,16025,16040,16041,16042,16043,16044,16045,
    16046,16047,16048,16049,16050,16051,16052,16053,16054,16055,16056,16057,
    16072,16073,16074,16075,16076,16077,16078,16079,16080,16081,16082,16083,
    16084,16085,16086,16087,16088,16089,16104,16105,16106,16107,16108,16109,
    16110,16111,16112,16113,16114,16115,16116,16117,16118,16119,16120,16121,
    16136,16137,16138,16139,16140,16141,16142,16143,16144,16145,16146,16147,
    16148,16149,16150,16151,16152,16153,16168,16169,16170,16171,16172,16173,
    16174,16175,16176,16177,16178,16179,16180,16181,16182,16183,16184,16185,
    16200,16201,16202,16203,16204,16205,16206,16207,16208,16209,16210,16211,
    16212,16213,16214,16215,16216,16217,16232,16233,16234,16235,16236,16237,
    16238,16239,16240,16241,16242,16243,16244,16245,16246,16247,16248,16249
  };
        
  for (unsigned int i = 0; i < chipids.size(); ++i) {
    hPixelHitMap.insert(make_pair(chipids[i], new TH2F(Form("chip%d", chipids[i]), Form("chip%d", chipids[i]),
                                                       256, 0., 256., 250, 0., 250.)));
  }

}


// ----------------------------------------------------------------------    
void AnaPixelHistos::EndRun(TARunInfo* runinfo)  {
  printf("PixelHistos::EndRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

  dataFile->Write();
  dataFile->Close();
}

// ----------------------------------------------------------------------    
TAFlowEvent* AnaPixelHistos::AnalyzeFlowEvent(TARunInfo*, TAFlags*, TAFlowEvent* flow) {
  if (!flow)
    return flow;

  HitVectorFlowEvent * hitevent = flow->Find<HitVectorFlowEvent>();

  if(!hitevent)
    return flow;

  ++fEvtCounter;
  if (0 == fEvtCounter%10000) cout << "AnaPixelHistos> Event counter " << fEvtCounter << endl;
  
  for(auto & hit: hitevent->pixelhits){
    chipID->Fill(hit.chipid());
    unsigned int chipid = static_cast<unsigned int>(hit.chipid());

    if (hPixelHitMap.find(chipid) == hPixelHitMap.end()) {
      cout << "AnaPixelHistos> XXX ERROR XXX chipID " << chipid << " not known" << endl;
    } else {
      // std::cout << "filling chipID " << chipid << endl;
      hPixelHitMap[chipid]->Fill(hit.col(), hit.row());
    }

  }

  return flow;
}

// ----------------------------------------------------------------------    
void AnaPixelHistoFactory::Init(const std::vector<std::string> &args) {

}
    
// ----------------------------------------------------------------------    
void AnaPixelHistoFactory::Finish() {

}