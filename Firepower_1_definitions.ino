// description of the Firepower pinball

const byte FP_INSTALLED_BALLS = 3;                     // number of balls installed in the game
const byte FP_NB_FIRE_LANES = 4;
const byte FP_NB_EJECT_HOLES = 3;

//////////////
// Switches //
//////////////
constexpr byte FP_SW_PLUMB_BOB_TILT = 1;
constexpr byte FP_SW_BALL_ROLL_TILT = 2;
constexpr byte FP_SW_SLAM_TILT = 7;
constexpr byte FP_SW_PLAYFIELD_TILT = 47;

constexpr byte FP_SW_START_BUTTON = 3;
constexpr byte FP_SW_OUTHOLE = 9;                      // number of the outhole switch
constexpr byte FP_SW_THROUGH[3] = {51,57,58};    // ball through switches from the plunger lane to the outhole
constexpr byte FP_SW_LANE_CHANGE_SWITCH = 45;
constexpr byte FP_SW_PLUNGER_LANE = 46;
constexpr byte FP_SW_FIRE_ROLLOVERS[FP_NB_FIRE_LANES] = {32,33,34,35};
constexpr byte FP_SW_123456_TARGETS[6] = {17,18,19,21,22,23};
constexpr byte FP_SW_HIGH_SCORE_RESET = 8;
constexpr byte FP_SW_SERVICE_MODE = 72;
constexpr byte FP_SW_EJECT_HOLES[FP_NB_EJECT_HOLES] = {13,30,36}; // left right/lower right/upper

///////////////
//   Lamps   //
///////////////
constexpr byte FP_LP_FIRE = 3;
constexpr byte FP_LP_POWER = 4;
constexpr byte FP_LP_123456_TARGETS[6] = {26,27,28,29,30,31};
constexpr byte FP_LP_FIRE_LANES[4] = {5,6,7,8};
constexpr byte FP_LP_EJECT_HOLES[FP_NB_EJECT_HOLES] = {33,34,35}; // left right/lower right/upper

///////////////
// Solenoids //
///////////////
constexpr byte FP_SOL_OUTHOLEKICKER = 1;                      // solenoid number of the outhole kicker
constexpr byte FP_SOL_SHOOTERLANEFEEDER = 8;                  // solenoid number of the shooter lane feeder
constexpr byte FP_SOL_FLIPPER_LEFT_ENABLE = 23;
constexpr byte FP_SOL_FLIPPER_RIGHT_ENABLE = 24;
constexpr byte FP_SOL_EJECT_HOLES[FP_NB_EJECT_HOLES] = {4,5,6}; // left right/lower right/upper

unsigned int FP_SolTimes[32] = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,0,0,100,100,100,100,100,100,100,100}; // Activation times for solenoids

const byte FP_defaults[64] = {0,0,0,0,0,0,0,0,        // game default settings
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0};

const struct SettingTopic FP_setList[3] = {
    {"RESTOREDEFAULT",RestoreDefaults,0,0,0},
    {"  EXIT SETTNGS",ExitSettings,0,0,0},
    {"",NULL,0,0,0}};
