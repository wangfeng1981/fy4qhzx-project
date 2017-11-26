// mengjiala_monitor.cpp : 定义控制台应用程序的入口点。
//孟加拉湾sst监测、出图、入库 2017-11-12
//孟加拉剖面 经度90E 纬度-5到+20
#include <iostream>
#include <ctime>
#include <fstream>
#include "../../sharedcodes/wftools.h"
#include <vector>
#include <string>
#include "gdal_priv.h"
using namespace std;




//2017-11-26 const int fy4_90e_num = 669;
//const int fy490eindex[] = { 1 };
//const float fy490elat[] = { 0 };
//2017-11-26 const int fy490eindex[] = { 2314816, 2317564, 2320312, 2323060, 2325808, 2328556, 2331303, 2334051, 2336799, 2339547, 2342295, 2345043, 2347791, 2350539, 2353287, 2356034, 2358782, 2361530, 2364278, 2367026, 2369774, 2372522, 2375270, 2378018, 2380766, 2383513, 2386261, 2389009, 2391757, 2394505, 2397253, 2400001, 2402749, 2405497, 2408244, 2410992, 2413740, 2416488, 2419236, 2421984, 2424732, 2427480, 2430228, 2432976, 2435723, 2438471, 2441219, 2443967, 2446715, 2449463, 2452211, 2454959, 2457707, 2460455, 2463203, 2465950, 2468698, 2471446, 2474194, 2476942, 2479690, 2482438, 2485186, 2487934, 2490682, 2493429, 2496177, 2498925, 2501673, 2504421, 2507169, 2509917, 2512665, 2515413, 2518161, 2520909, 2523656, 2526404, 2529152, 2531900, 2534648, 2537396, 2540144, 2542892, 2545640, 2548388, 2551136, 2553883, 2556631, 2559379, 2562127, 2564875, 2567623, 2570371, 2573119, 2575867, 2578615, 2581363, 2584110, 2586858, 2589606, 2592354, 2595102, 2597850, 2600598, 2603346, 2606094, 2608842, 2611590, 2614338, 2617085, 2619833, 2622581, 2625329, 2628077, 2630825, 2633573, 2636321, 2639069, 2641817, 2644565, 2647313, 2650060, 2652808, 2655556, 2658304, 2661052, 2663800, 2666548, 2669296, 2672044, 2674792, 2677540, 2680288, 2683036, 2685783, 2688531, 2691279, 2694027, 2696775, 2699523, 2702271, 2705019, 2707767, 2710515, 2713263, 2716011, 2718759, 2721506, 2724254, 2727002, 2729750, 2732498, 2735246, 2737994, 2740742, 2743490, 2746238, 2748986, 2751734, 2754482, 2757229, 2759977, 2762725, 2765473, 2768221, 2770969, 2773717, 2776465, 2779213, 2781961, 2784709, 2787457, 2790205, 2792953, 2795700, 2798448, 2801196, 2803944, 2806692, 2809440, 2812188, 2814936, 2817684, 2820432, 2823180, 2825928, 2828676, 2831424, 2834172, 2836919, 2839667, 2842415, 2845163, 2847911, 2850659, 2853407, 2856155, 2858903, 2861651, 2864399, 2867147, 2869895, 2872643, 2875391, 2878138, 2880886, 2883634, 2886382, 2889130, 2891878, 2894626, 2897374, 2900122, 2902870, 2905618, 2908366, 2911114, 2913862, 2916610, 2919358, 2922105, 2924853, 2927601, 2930349, 2933097, 2935845, 2938593, 2941341, 2944089, 2946837, 2949585, 2952333, 2955081, 2957829, 2960577, 2963325, 2966073, 2968820, 2971568, 2974316, 2977064, 2979812, 2982560, 2985308, 2988056, 2990804, 2993552, 2996300, 2999048, 3001796, 3004544, 3007292, 3010040, 3012788, 3015536, 3018284, 3021031, 3023779, 3026527, 3029275, 3032023, 3034771, 3037519, 3040267, 3043015, 3045763, 3048511, 3051259, 3054007, 3056755, 3059503, 3062251, 3064999, 3067747, 3070495, 3073242, 3075990, 3078738, 3081486, 3084234, 3086982, 3089730, 3092478, 3095226, 3097974, 3100722, 3103470, 3106218, 3108966, 3111714, 3114462, 3117210, 3119958, 3122706, 3125454, 3128202, 3130949, 3133697, 3136445, 3139193, 3141941, 3144689, 3147437, 3150185, 3152933, 3155681, 3158429, 3161177, 3163925, 3166673, 3169421, 3172169, 3174917, 3177665, 3180413, 3183161, 3185909, 3188657, 3191405, 3194153, 3196900, 3199648, 3202396, 3205144, 3207892, 3210640, 3213388, 3216136, 3218884, 3221632, 3224380, 3227128, 3229876, 3232624, 3235372, 3238120, 3240868, 3243616, 3246364, 3249112, 3251860, 3254608, 3257356, 3260104, 3262852, 3265600, 3268347, 3271095, 3273843, 3276591, 3279339, 3282087, 3284835, 3287583, 3290331, 3293079, 3295827, 3298575, 3301323, 3304071, 3306819, 3309567, 3312315, 3315063, 3317811, 3320559, 3323307, 3326055, 3328803, 3331551, 3334299, 3337047, 3339795, 3342543, 3345291, 3348039, 3350787, 3353534, 3356282, 3359030, 3361778, 3364526, 3367274, 3370022, 3372770, 3375518, 3378266, 3381014, 3383762, 3386510, 3389258, 3392006, 3394754, 3397502, 3400250, 3402998, 3405746, 3408494, 3411242, 3413990, 3416738, 3419486, 3422234, 3424982, 3427730, 3430478, 3433226, 3435974, 3438722, 3441470, 3444218, 3446966, 3449714, 3452462, 3455210, 3457958, 3460706, 3463453, 3466201, 3468949, 3471697, 3474445, 3477193, 3479941, 3482689, 3485437, 3488185, 3490933, 3493681, 3496429, 3499177, 3501925, 3504673, 3507421, 3510169, 3512917, 3515665, 3518413, 3521161, 3523909, 3526657, 3529405, 3532153, 3534901, 3537649, 3540397, 3543145, 3545893, 3548641, 3551389, 3554137, 3556885, 3559633, 3562381, 3565129, 3567877, 3570625, 3573373, 3576121, 3578869, 3581617, 3584365, 3587113, 3589861, 3592609, 3595357, 3598105, 3600853, 3603601, 3606349, 3609097, 3611845, 3614593, 3617341, 3620089, 3622837, 3625585, 3628333, 3631081, 3633829, 3636577, 3639325, 3642072, 3644820, 3647568, 3650316, 3653064, 3655812, 3658560, 3661308, 3664056, 3666804, 3669552, 3672300, 3675048, 3677796, 3680544, 3683292, 3686040, 3688788, 3691536, 3694284, 3697032, 3699780, 3702528, 3705276, 3708024, 3710772, 3713520, 3716268, 3719016, 3721764, 3724512, 3727260, 3730008, 3732756, 3735504, 3738252, 3741000, 3743748, 3746496, 3749244, 3751992, 3754740, 3757488, 3760236, 3762984, 3765732, 3768480, 3771228, 3773976, 3776724, 3779472, 3782220, 3784968, 3787716, 3790464, 3793212, 3795960, 3798708, 3801456, 3804204, 3806952, 3809700, 3812448, 3815196, 3817944, 3820692, 3823440, 3826188, 3828936, 3831684, 3834432, 3837180, 3839928, 3842676, 3845424, 3848172, 3850920, 3853668, 3856416, 3859164, 3861912, 3864660, 3867408, 3870156, 3872904, 3875652, 3878400, 3881148, 3883896, 3886644, 3889392, 3892140, 3894888, 3897636, 3900384, 3903132, 3905880, 3908628, 3911377, 3914125, 3916873, 3919621, 3922369, 3925117, 3927865, 3930613, 3933361, 3936109, 3938857, 3941605, 3944353, 3947101, 3949849, 3952597, 3955345, 3958093, 3960841, 3963589, 3966337, 3969085, 3971833, 3974581, 3977329, 3980077, 3982825, 3985573, 3988321, 3991069, 3993817, 3996565, 3999313, 4002061, 4004809, 4007557, 4010305, 4013053, 4015801, 4018549, 4021297, 4024045, 4026793, 4029541, 4032289, 4035037, 4037785, 4040533, 4043281, 4046029, 4048777, 4051525, 4054273, 4057021, 4059769, 4062517, 4065265, 4068013, 4070761, 4073509, 4076257, 4079005, 4081753, 4084501, 4087249, 4089998, 4092746, 4095494, 4098242, 4100990, 4103738, 4106486, 4109234, 4111982, 4114730, 4117478, 4120226, 4122974, 4125722, 4128470, 4131218, 4133966, 4136714, 4139462, 4142210, 4144958, 4147706, 4150454 };
//2017-11-26 const float fy490elat[] = { 19.9675f, 19.9274f, 19.8873f, 19.8472f, 19.8071f, 19.7670f, 19.7276f, 19.6876f, 19.6475f, 19.6075f, 19.5676f, 19.5276f, 19.4876f, 19.4477f, 19.4078f, 19.3685f, 19.3286f, 19.2887f, 19.2488f, 19.2090f, 19.1691f, 19.1293f, 19.0895f, 19.0497f, 19.0100f, 18.9708f, 18.9311f, 18.8913f, 18.8516f, 18.8119f, 18.7722f, 18.7326f, 18.6929f, 18.6533f, 18.6142f, 18.5746f, 18.5350f, 18.4954f, 18.4559f, 18.4163f, 18.3768f, 18.3372f, 18.2977f, 18.2582f, 18.2193f, 18.1798f, 18.1404f, 18.1009f, 18.0615f, 18.0221f, 17.9827f, 17.9433f, 17.9039f, 17.8646f, 17.8252f, 17.7864f, 17.7471f, 17.7078f, 17.6685f, 17.6292f, 17.5900f, 17.5507f, 17.5115f, 17.4723f, 17.4330f, 17.3944f, 17.3552f, 17.3160f, 17.2769f, 17.2377f, 17.1986f, 17.1595f, 17.1204f, 17.0813f, 17.0422f, 17.0031f, 16.9646f, 16.9255f, 16.8865f, 16.8475f, 16.8085f, 16.7695f, 16.7305f, 16.6915f, 16.6526f, 16.6136f, 16.5747f, 16.5363f, 16.4974f, 16.4585f, 16.4196f, 16.3807f, 16.3419f, 16.3030f, 16.2642f, 16.2254f, 16.1866f, 16.1478f, 16.1095f, 16.0707f, 16.0319f, 15.9932f, 15.9545f, 15.9157f, 15.8770f, 15.8383f, 15.7996f, 15.7609f, 15.7223f, 15.6836f, 15.6454f, 15.6068f, 15.5682f, 15.5296f, 15.4910f, 15.4524f, 15.4138f, 15.3752f, 15.3367f, 15.2981f, 15.2596f, 15.2211f, 15.1830f, 15.1445f, 15.1060f, 15.0676f, 15.0291f, 14.9906f, 14.9522f, 14.9138f, 14.8753f, 14.8369f, 14.7985f, 14.7601f, 14.7217f, 14.6838f, 14.6455f, 14.6071f, 14.5688f, 14.5304f, 14.4921f, 14.4538f, 14.4155f, 14.3772f, 14.3390f, 14.3007f, 14.2624f, 14.2242f, 14.1864f, 14.1482f, 14.1099f, 14.0717f, 14.0335f, 13.9954f, 13.9572f, 13.9190f, 13.8809f, 13.8427f, 13.8046f, 13.7665f, 13.7283f, 13.6906f, 13.6526f, 13.6145f, 13.5764f, 13.5383f, 13.5003f, 13.4622f, 13.4242f, 13.3861f, 13.3481f, 13.3101f, 13.2721f, 13.2341f, 13.1961f, 13.1586f, 13.1206f, 13.0826f, 13.0447f, 13.0067f, 12.9688f, 12.9309f, 12.8930f, 12.8551f, 12.8172f, 12.7793f, 12.7414f, 12.7036f, 12.6657f, 12.6279f, 12.5904f, 12.5526f, 12.5148f, 12.4769f, 12.4391f, 12.4013f, 12.3636f, 12.3258f, 12.2880f, 12.2502f, 12.2125f, 12.1748f, 12.1370f, 12.0993f, 12.0616f, 12.0242f, 11.9865f, 11.9488f, 11.9111f, 11.8735f, 11.8358f, 11.7981f, 11.7605f, 11.7228f, 11.6852f, 11.6476f, 11.6099f, 11.5723f, 11.5347f, 11.4971f, 11.4595f, 11.4223f, 11.3847f, 11.3472f, 11.3096f, 11.2721f, 11.2345f, 11.1970f, 11.1595f, 11.1219f, 11.0844f, 11.0469f, 11.0094f, 10.9719f, 10.9345f, 10.8970f, 10.8595f, 10.8221f, 10.7849f, 10.7475f, 10.7100f, 10.6726f, 10.6352f, 10.5978f, 10.5604f, 10.5230f, 10.4856f, 10.4482f, 10.4108f, 10.3734f, 10.3361f, 10.2987f, 10.2613f, 10.2240f, 10.1867f, 10.1493f, 10.1120f, 10.0750f, 10.0377f, 10.0004f, 9.9631f, 9.9258f, 9.8885f, 9.8512f, 9.8140f, 9.7767f, 9.7394f, 9.7022f, 9.6649f, 9.6277f, 9.5905f, 9.5533f, 9.5160f, 9.4788f, 9.4416f, 9.4044f, 9.3675f, 9.3303f, 9.2931f, 9.2560f, 9.2188f, 9.1816f, 9.1445f, 9.1073f, 9.0702f, 9.0330f, 8.9959f, 8.9588f, 8.9217f, 8.8846f, 8.8474f, 8.8103f, 8.7733f, 8.7362f, 8.6991f, 8.6620f, 8.6249f, 8.5881f, 8.5511f, 8.5140f, 8.4769f, 8.4399f, 8.4029f, 8.3658f, 8.3288f, 8.2918f, 8.2548f, 8.2178f, 8.1807f, 8.1437f, 8.1068f, 8.0698f, 8.0328f, 7.9958f, 7.9588f, 7.9219f, 7.8849f, 7.8479f, 7.8110f, 7.7740f, 7.7371f, 7.7004f, 7.6635f, 7.6265f, 7.5896f, 7.5527f, 7.5158f, 7.4789f, 7.4420f, 7.4051f, 7.3682f, 7.3313f, 7.2944f, 7.2575f, 7.2207f, 7.1838f, 7.1469f, 7.1101f, 7.0732f, 7.0364f, 6.9995f, 6.9627f, 6.9258f, 6.8890f, 6.8522f, 6.8154f, 6.7785f, 6.7419f, 6.7051f, 6.6683f, 6.6315f, 6.5947f, 6.5579f, 6.5211f, 6.4843f, 6.4476f, 6.4108f, 6.3740f, 6.3373f, 6.3005f, 6.2637f, 6.2270f, 6.1902f, 6.1535f, 6.1167f, 6.0800f, 6.0433f, 6.0065f, 5.9698f, 5.9331f, 5.8964f, 5.8597f, 5.8229f, 5.7862f, 5.7495f, 5.7128f, 5.6761f, 5.6394f, 5.6029f, 5.5662f, 5.5296f, 5.4929f, 5.4562f, 5.4195f, 5.3829f, 5.3462f, 5.3095f, 5.2729f, 5.2362f, 5.1996f, 5.1629f, 5.1263f, 5.0897f, 5.0530f, 5.0164f, 4.9798f, 4.9431f, 4.9065f, 4.8699f, 4.8333f, 4.7966f, 4.7600f, 4.7234f, 4.6868f, 4.6502f, 4.6136f, 4.5770f, 4.5404f, 4.5038f, 4.4673f, 4.4307f, 4.3941f, 4.3575f, 4.3209f, 4.2844f, 4.2478f, 4.2112f, 4.1747f, 4.1382f, 4.1017f, 4.0651f, 4.0286f, 3.9920f, 3.9555f, 3.9189f, 3.8824f, 3.8458f, 3.8093f, 3.7727f, 3.7362f, 3.6997f, 3.6631f, 3.6266f, 3.5901f, 3.5536f, 3.5171f, 3.4805f, 3.4440f, 3.4075f, 3.3710f, 3.3345f, 3.2980f, 3.2615f, 3.2250f, 3.1885f, 3.1520f, 3.1155f, 3.0790f, 3.0425f, 3.0060f, 2.9695f, 2.9330f, 2.8965f, 2.8601f, 2.8236f, 2.7871f, 2.7506f, 2.7141f, 2.6777f, 2.6412f, 2.6047f, 2.5683f, 2.5318f, 2.4953f, 2.4589f, 2.4224f, 2.3859f, 2.3495f, 2.3130f, 2.2766f, 2.2401f, 2.2037f, 2.1672f, 2.1308f, 2.0943f, 2.0579f, 2.0214f, 1.9850f, 1.9485f, 1.9121f, 1.8756f, 1.8392f, 1.8028f, 1.7664f, 1.7299f, 1.6935f, 1.6571f, 1.6206f, 1.5842f, 1.5478f, 1.5113f, 1.4749f, 1.4385f, 1.4020f, 1.3656f, 1.3292f, 1.2928f, 1.2563f, 1.2199f, 1.1835f, 1.1471f, 1.1106f, 1.0742f, 1.0378f, 1.0014f, 0.9650f, 0.9285f, 0.8921f, 0.8557f, 0.8193f, 0.7829f, 0.7465f, 0.7100f, 0.6736f, 0.6372f, 0.6008f, 0.5644f, 0.5280f, 0.4916f, 0.4551f, 0.4187f, 0.3823f, 0.3459f, 0.3095f, 0.2731f, 0.2367f, 0.2003f, 0.1638f, 0.1274f, 0.0910f, 0.0546f, 0.0182f, -0.0182f, -0.0546f, -0.0910f, -0.1274f, -0.1638f, -0.2003f, -0.2367f, -0.2731f, -0.3095f, -0.3459f, -0.3823f, -0.4187f, -0.4551f, -0.4916f, -0.5280f, -0.5644f, -0.6008f, -0.6372f, -0.6736f, -0.7100f, -0.7465f, -0.7829f, -0.8193f, -0.8557f, -0.8921f, -0.9285f, -0.9650f, -1.0014f, -1.0378f, -1.0742f, -1.1106f, -1.1471f, -1.1835f, -1.2199f, -1.2563f, -1.2928f, -1.3292f, -1.3656f, -1.4020f, -1.4385f, -1.4749f, -1.5113f, -1.5478f, -1.5842f, -1.6206f, -1.6571f, -1.6935f, -1.7299f, -1.7664f, -1.8028f, -1.8392f, -1.8756f, -1.9121f, -1.9485f, -1.9850f, -2.0214f, -2.0579f, -2.0943f, -2.1308f, -2.1672f, -2.2037f, -2.2401f, -2.2766f, -2.3130f, -2.3495f, -2.3859f, -2.4224f, -2.4589f, -2.4953f, -2.5318f, -2.5683f, -2.6047f, -2.6412f, -2.6777f, -2.7141f, -2.7506f, -2.7871f, -2.8236f, -2.8601f, -2.8965f, -2.9330f, -2.9695f, -3.0060f, -3.0425f, -3.0790f, -3.1155f, -3.1520f, -3.1885f, -3.2250f, -3.2615f, -3.2980f, -3.3345f, -3.3710f, -3.4075f, -3.4440f, -3.4805f, -3.5171f, -3.5536f, -3.5901f, -3.6266f, -3.6631f, -3.6997f, -3.7362f, -3.7727f, -3.8093f, -3.8458f, -3.8824f, -3.9189f, -3.9555f, -3.9920f, -4.0286f, -4.0651f, -4.1017f, -4.1382f, -4.1747f, -4.2112f, -4.2478f, -4.2844f, -4.3209f, -4.3575f, -4.3941f, -4.4307f, -4.4673f, -4.5038f, -4.5404f, -4.5770f, -4.6136f, -4.6502f, -4.6868f, -4.7234f, -4.7600f, -4.7966f, -4.8333f, -4.8699f, -4.9065f, -4.9431f, -4.9798f };
const int wind_latarr_num = 11;
const float windLatArr[] = { 20.0,17.5,15.0,12.5,10,7.5,5.0,2.5,0.0,-2.5,-5.0 };

//2017-11-26
bool loadMengjialaPixelIndicesAndLats(string txtfile, vector<int>& pixVec, vector<float>& latVec)
{
	//逐行分析
	cout << "loading region pixels and lats for " << txtfile << endl;
	std::ifstream infs(txtfile.c_str());
	std::string line;
	while (std::getline(infs, line))
	{
		std::istringstream iss(line);
		int pix = 0;
		float lat = 0.f;
		if (!(iss >> pix >> lat)) 
		{ 
			//bad values
		}
		else {
			pixVec.push_back(pix);
			latVec.push_back(lat);
		}
	}
	infs.close();
	cout << "loading finished. all count " << pixVec.size() << endl;
	return true;
}
//2017-11-26 end


bool isvalidFy4SstDailyFile(string& filename)
{
	// FY4A-_AGRI--_N_DISK_1047E_L2-_LPW-_MULT_NOM_20171006_TPW_combination_day.tif
	size_t pos0 = filename.find("FY4A-_AGRI--_N_DISK_");//2017-11-26 FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_ maybe 9995E
	size_t pos1 = filename.find("_SST_combination_day.tif");
	size_t pos2 = filename.find("_day.tif");
	size_t pos3 = filename.find("_L2-_SST-_MULT_NOM_");//2017-11-26 
	if (pos0 != string::npos && pos1 != string::npos  && pos2 == filename.length() - 8 && pos3 != string::npos )//bugfixed 2017-11-10  //2017-11-26 
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool isvalidWindDailyFile(string& filename)
{
	size_t pos0 = filename.find("ncepwind.");
	size_t pos2 = filename.find("xyuv.txt");
	if (pos0 != string::npos && pos2 == filename.length() - 8)// 
	{
		return true;
	}
	else
	{
		return false;
	}
}



string getSstYmd(string& filename)
{
	return filename.substr(44, 8);
}
string getWindYmd(string& filename)
{
	//ncepwind.20171006.850.xyuv
	return filename.substr(9, 8);
}

/*
int processOneFile(
	string& waterfilepath,
	string& windfilepath,
	string& outdir,
	string& outpngpath,
	string& wgs84Program,
	string& lonfile,
	string& latfile,
	string& gdalWarp,
	string& txtProgram,
	string& plotTemplate,
	string& plotProgram,
	string& dbProgram,
	int ymdloc,
	string& host,
	string& user,
	string& pwd,
	string& db,
	string& tb,
	string& pid
	)
{
	return 0;
}
*/

void extractSst90EData(string& fy4sstfilepath, vector<int>& pixVec , vector<float>& latVec, string& sst90efilepath)//2017-11-26
{
	GDALDataset* fy4sst = (GDALDataset*)GDALOpen(fy4sstfilepath.c_str(), GDALAccess::GA_ReadOnly);
	int xsize = fy4sst->GetRasterXSize();
	int ysize = fy4sst->GetRasterYSize();
	float* buffer = new float[xsize * ysize];
	fy4sst->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Read, 0, 0, xsize, ysize,
		buffer, xsize, ysize, GDALDataType::GDT_Float32, 0, 0, 0);
	ofstream ofs(sst90efilepath);
	int num = pixVec.size();//2017-11-26
	ofs << "#lat sst for mengjiala 90E" << endl;
	for (int i = 0; i < num; ++i)
	{
		float sst = buffer[pixVec[i]];//2017-11-26
		float lat = latVec[i];//2017-11-26
		if (sst > -10 && sst < 60)
		{
			ofs << lat << "\t" << sst << endl;
		}
		else
		{
			ofs << lat << "\t NaN"  << endl;
		}
		
	}
	ofs.close();
	delete[] buffer;
	GDALClose(fy4sst);
}

void extractWind90EData(string& windfile, string& outfile)
{
	ifstream ifs(windfile.c_str());
	ofstream ofs(outfile.c_str());
	ofs << "#lat windu windv for mengjiala 90E" << endl;
	std::string line;
	getline(ifs, line);//first line is no used.
	while ( getline(ifs, line))
	{
		std::istringstream iss(line);
		if (line.length() > 1)
		{
			float lon, lat, wu, wv;
			iss >> lon >> lat >> wu >> wv;
			if (lon > 89.9f && lon < 90.1f && lat > -6.0f && lat < 21.0f)
			{
				ofs << lat << "\t" << wu << "\t" << wv << endl;
			}
		}
	}
	ifs.close();
	ofs.close();
}

void str2tm(string ymd, struct tm * t)
{
	string month = ymd.substr(4, 2);
	string year = ymd.substr(0, 4);
	string day = ymd.substr(6, 2);

	memset(t, 0, sizeof(struct tm));
	t->tm_mday = stoi(day);
	t->tm_mon = stoi(month) - 1;
	t->tm_year = stoi(year) - 1900;
}

int getYmdByEndDateAndDayCount(string ymd1, int nday)
{
	struct tm time1 ;
	str2tm(ymd1, &time1);
	time_t t1 = mktime(&time1);
	time_t t0 = t1 - nday * 60 * 60 * 24;
	struct tm * time0 = localtime(&t0);
	int day0 = time0->tm_mday;
	int mon0 = time0->tm_mon + 1;
	int year0 = time0->tm_year + 1900;
	return year0 * 10000 + mon0 * 100 + day0;
}

void appendfile2ofs(string& filepath, ofstream& ofs , string& ymd)
{
	ifstream ifs(filepath.c_str());
	std::string line;
	getline(ifs, line);//first line is no used.
	while (getline(ifs, line))
	{
		if (line.length() > 1)
		{
			ofs << ymd << "\t" <<line << endl;
		}
	}
	ifs.close();
}

void appendSSTNan2ofs( ofstream& ofs, string& ymd)
{
	for (int i = 0; i < fy4_90e_num ; ++i)
	{
		ofs << ymd << "\t" << fy490elat[i] << "\t NaN" << endl;
	}
}
/*
void appendWindNan2ofs( ofstream& ofs, string& ymd)
{
	
		for (int i = 0; i < wind_latarr_num ; ++i)
	{
		ofs << ymd << "\t" << windLatArr[i] << "\t   \t  " << endl;
	}
	

}
*/


string getY_m_dByYmds(string ymd)
{
	return ymd.substr(0, 4) + "-" + ymd.substr(4, 2) + "-" + ymd.substr(6, 2);
}

string getY_m_dByYmdi(int ymd)
{
	return getY_m_dByYmds(wft_int2str(ymd));
}


int main(int argc , char** argv )
{
	cout << "A program to monitor mengjiala wan sst and wind.2017-11-13." << endl;
	cout << "v1.0 2017-11-13" << endl;
	cout << "v1.1 do days before if today is done." << endl;
	if (argc == 1)
	{
		cout << "mengjiala_monitor startup.txt" << endl;
		cout << "******** startup.txt sample ********" << endl;
		cout << "#dailyssttifdir" << endl;
		cout << "D:/sst/" << endl;
		cout << "#90e_sst_txtdir" << endl;
		cout << "D:/sst90e/" << endl;

		cout << "#windtxtdir" << endl;
		cout << "/windtxt/" << endl;
		cout << "#90e_wind_txtdir" << endl;
		cout << "/wind90e/" << endl;


		cout << "#outdir" << endl;
		cout << "D:/mengjiala_output/" << endl;

		cout << "#daycount" << endl;
		cout << "30" << endl;
		cout << "#plot" << endl;
		cout << "gnuplot" << endl;
		cout << "#plottem" << endl;
		cout << "/extras/example.plot" << endl;

		cout << "#insertprogram" << endl;
		cout << "/root/ncc-fy4-project/produce_codes/insertdb/insertdb" << endl;
		cout << "#host" << endl;
		cout << "localhost" << endl;
		cout << "#user" << endl;
		cout << "htht" << endl;
		cout << "#pwd" << endl;
		cout << "htht000000" << endl;
		cout << "#db" << endl;
		cout << "qhzx_uus" << endl;
		cout << "#tb" << endl;
		cout << "tb_data_jifeng" << endl;
		cout << "#pid" << endl;
		cout << "99999" << endl;

		cout << "**** **** **** **** ****" << endl;
		exit(101);
	}

	/*
	string sstdir = "E:/testdata/fy4sst15min/fake_daily/";
	string mjlssttxtdir = "E:/testdata/fy4sst15min/mengjiala_sst_90e/";
	string winddir = "E:/testdata/ncep-wind/";
	string mjlwindtxtdir = "E:/testdata/fy4sst15min/mengjiala_wind_90e/";
	string mjloutdir = "E:/testdata/fy4sst15min/mengjiala-output/";
	string dayNumStr = "30";
	string plotProgram = "gnuplot";
	string plotTemplate = "E:/coding/fy4qhzx-project/extras/mengjiala-template-20171113.plot";
	*/
	string startupfile = string(argv[1]);
	string sstdir = wft_getValueFromExtraParamsFile(startupfile , "#dailyssttifdir" , true ) ;
	string mjlssttxtdir = wft_getValueFromExtraParamsFile(startupfile, "#90e_sst_txtdir", true);
	string winddir = wft_getValueFromExtraParamsFile(startupfile , "#windtxtdir" , true ) ;
	string mjlwindtxtdir = wft_getValueFromExtraParamsFile(startupfile, "#90e_wind_txtdir", true);
	string mjloutdir = wft_getValueFromExtraParamsFile(startupfile, "#outdir", true);
	string dayNumStr = wft_getValueFromExtraParamsFile(startupfile, "#daycount", true);
	string plotProgram = wft_getValueFromExtraParamsFile(startupfile, "#plot", true);
	string plotTemplate = wft_getValueFromExtraParamsFile(startupfile, "#plottem", true);

	/*
	string dbProgram = wft_getValueFromExtraParamsFile(startupfile, "#insertprogram", true);
	string host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startupfile, "#pid", true);
	*/

	string endYmdStr = wft_current_dateymd();
	GDALAllRegister();
	vector<string> allfiles;
	
	cout << " ********** step1 **********" << endl;

	//step1 check fy4-sst
	wft_get_allfiles(sstdir, allfiles);
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string fpath1 = allfiles[i];
		string fname1 = wft_base_name(fpath1);
		if (isvalidFy4SstDailyFile(fname1))
		{
			string ymd1 = getSstYmd(fname1);
			string sst90efilename = "fy4sst90e." + ymd1 + ".txt";
			string sst90efilepath = mjlssttxtdir + sst90efilename;
			if (wft_test_file_exists(sst90efilepath))
			{
				continue;
			}
			else {
				cout << "making " << sst90efilename << "... ..." <<endl;
				extractSst90EData(fpath1, sst90efilepath);
			}
		}
	}

	cout << " ********** step2 **********" << endl;

	//step2 check wind
	allfiles.clear();
	wft_get_allfiles(winddir, allfiles);
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string fpath1 = allfiles[i];
		string fname1 = wft_base_name(fpath1);
		if (isvalidWindDailyFile(fname1))
		{
			string ymd1 = getWindYmd(fname1);
			string wind90efilename = "wind90e." + ymd1 + ".txt";
			string wind90efilepath = mjlwindtxtdir + wind90efilename;
			if (wft_test_file_exists(wind90efilepath))
			{
				continue;
			}
			else {
				cout << "making " << wind90efilename << "... ..." << endl;
				extractWind90EData(fpath1, wind90efilepath);
			}
		}
	}

	cout << " ********** step3 combine days sst and wind **********" << endl;
	int endYmd = (int)atof(endYmdStr.c_str());
	int ndays = (int)atof(dayNumStr.c_str());
	int startYmd = getYmdByEndDateAndDayCount(endYmdStr, ndays);
	cout << "end day : " << endYmdStr << " before day num:" << ndays << " is date:" << startYmd << endl;
	allfiles.clear();

	string outpngname = string("mengjiala_sst_from_") + wft_int2str(startYmd) + "_to_" + endYmdStr + ".png";
	string outpngpath = mjloutdir + outpngname;

	if (wft_test_file_exists(outpngpath))
	{
		bool donext = false;
		cout << "Already has " << outpngpath << ", then out." << endl;
		wft_get_allfiles(mjlssttxtdir, allfiles);
		for (int i = 0; i < allfiles.size(); ++i)
		{
			string filepath1 = allfiles[i];
			string filename1 = wft_base_name(filepath1);
			int pos1 = filename1.find("fy4sst90e.");
			if (pos1 != string::npos)
			{
				endYmdStr = filename1.substr(10, 8);
				endYmd = (int)atof(endYmdStr.c_str());
				startYmd = getYmdByEndDateAndDayCount(endYmdStr, ndays);
				outpngname = string("mengjiala_sst_from_") + wft_int2str(startYmd) + "_to_" + endYmdStr + ".png";
				outpngpath = mjloutdir + outpngname;
				if (wft_test_file_exists(outpngpath) == false)
				{
					donext = true;
					break;
				}
			}
		}
		if (donext)
		{
			cout << "find : end day : " << endYmdStr << " before day num:" << ndays << " is date:" << startYmd << endl;
			cout << "making " << outpngpath << endl;
		}
		else {
			cout << "no more to do. out." << endl;
			return 0;
		}
	}

	int startYear = startYmd / 10000;
	int endYear = endYmd / 10000;
	string allwindfile = outpngpath + ".wind.txt";
	string allsstfile = outpngpath + ".sst.txt";
	ofstream windofs(allwindfile.c_str());
	ofstream sstofs(allsstfile.c_str());
	int nvalidday = 0;
	for (int iyear = startYear; iyear <= endYear; ++iyear)
	{
		for (int imon = 1; imon <= 12; ++imon)
		{
			for (int iday = 1; iday <= 31; ++iday)
			{
				int tymd = iyear * 10000 + imon * 100 + iday;
				if (tymd >= startYmd && tymd <= endYmd)
				{
					string sst90efile = string("fy4sst90e.") + wft_int2str(tymd) + ".txt";
					string wind90efile = string("wind90e.") + wft_int2str(tymd) + ".txt";
					string pathsst = mjlssttxtdir + sst90efile;
					string pathwind = mjlwindtxtdir + wind90efile;
					string tymds = wft_int2str(tymd);
					string xymd = tymds.substr(0, 4) + "-" + tymds.substr(4, 2) + "-" + tymds.substr(6, 2);
					int hasboth = 0;
					if (wft_test_file_exists(pathsst) )
					{			
						++hasboth;
						appendfile2ofs(pathsst, sstofs, xymd);
						sstofs << endl;						
					}
					else {
						appendSSTNan2ofs(sstofs, xymd);
						sstofs << endl;
					}
					if (wft_test_file_exists(pathwind))
					{
						++hasboth;
						appendfile2ofs(pathwind, windofs, xymd);
						windofs << endl;
					}
					else {
						//appendWindNan2ofs(windofs, xymd);
						//windofs << endl;
					}
					if (hasboth == 2)
					{
						++nvalidday;
					}
				}
			}
		}
	}
	windofs.close();
	sstofs.close();
	cout << "it found has both sst and wind ymd num:" << nvalidday  << endl;

	string startY_m_d = getY_m_dByYmdi(startYmd);
	string endY_m_d = getY_m_dByYmdi(endYmd);

	if (wft_test_file_exists(allwindfile) && wft_test_file_exists(allsstfile) )
	{
		
		int pos0 = 19;
		int pos1 = 31;
		string ymd0 = outpngname.substr(pos0, 8);
		string ymd1 = outpngname.substr(pos1, 8);
		vector<string> varvector, repvector;
		varvector.push_back("{{{INFILE1}}}");
		varvector.push_back("{{{INFILE2}}}");
		varvector.push_back("{{{OUTFILE}}}");
		varvector.push_back("{{{TITLE1}}}");
		varvector.push_back("{{{TITLE2}}}");
		varvector.push_back("{{{DATE0}}}");
		varvector.push_back("{{{DATE1}}}");
		repvector.push_back(allsstfile);
		repvector.push_back(allwindfile);
		repvector.push_back(outpngpath);
		repvector.push_back(ymd0);
		repvector.push_back(ymd1);
		repvector.push_back(startY_m_d);
		repvector.push_back(endY_m_d);
		string temp_plotfile = outpngpath + ".plot";
		wft_create_file_by_template_with_replacement(temp_plotfile, plotTemplate, varvector, repvector);
		if (wft_test_file_exists(temp_plotfile))
		{
			string cmd6 = plotProgram + " " + temp_plotfile;
			int res6 = system(cmd6.c_str());
			cout << "plot result:" << res6 << endl;
			if (wft_test_file_exists(outpngpath))
			{
				//insert db.
				/*
				string cmd7 = dbProgram + " -host " + host + " -user " + user
					+ " -pwd " + pwd + " -db " + db + " -tb " + tb
					+ " -datapath " + waterfilepath
					+ " -dtloc " + wft_int2str(posYmd)
					+ " -dtlen 8 "
					+ " -thumb " + outpngpath
					+ " -pid " + pid;

				cout << cmd7 << endl;
				int res7 = system(cmd7.c_str());
				cout << "insertdb result:" << res7 << endl;
				*/
				//delete temp files.
				wft_remove_file(allwindfile);
				wft_remove_file(allsstfile);

				return 0;
			}
			else {
				cout << "*** Error : failed to make png file " << outpngpath << endl;
				return 10;
			}
		}
		else {
			cout << "*** Error : failed to make plot file " << temp_plotfile << endl;
			return 10;
		}
	}
	else
	{
		cout << "*** Error : failed to make txt file  " << allwindfile << " or/and "  << allsstfile << endl;
		return 10;
	}
	
	cout << "done." << endl;
    return 0;
}







