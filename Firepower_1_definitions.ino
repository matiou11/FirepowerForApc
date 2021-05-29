// description of the Firepower pinball

const byte FP_INSTALLED_BALLS = 3;                     // number of balls installed in the game
const byte FP_NB_FIRE_LANES = 4;

//////////////
// Switches //
//////////////
const byte FP_SW_PLUMB_BOB_TILT = 1;
const byte FP_SW_BALL_ROLL_TILT = 2;
const byte FP_SW_SLAM_TILT = 7;
const byte FP_SW_PLAYFIELD_TILT = 47;

const byte FP_SW_START_BUTTON = 3;
const byte FP_SW_OUTHOLE = 9;                      // number of the outhole switch
const byte FP_SW_THROUGH[3] = {51,57,58};    // ball through switches from the plunger lane to the outhole
const byte FP_SW_LANE_CHANGE_SWITCH = 45;
const byte FP_SW_PLUNGER_LANE = 46;
const byte FP_SW_FIRE_ROLLOVERS[FP_NB_FIRE_LANES] = {32,33,34,35};
const byte FP_SW_123456_TARGETS[6] = {17,18,19,21,22,23};
const byte FP_SW_HIGH_SCORE_RESET = 8;
const byte FP_SW_SERVICE_MODE = 72;

///////////////
//   Lamps   //
///////////////
const byte FP_LP_123456_TARGETS[6] = {26,27,28,29,30,31};
const byte FP_LP_FIRE_LANES[4] = {5,6,7,8};

///////////////
// Solenoids //
///////////////
const byte FP_SOL_OUTHOLEKICKER = 1;                      // solenoid number of the outhole kicker
const byte FP_SOL_SHOOTERLANEFEEDER = 8;                  // solenoid number of the shooter lane feeder
const byte FP_SOL_FLIPPER_LEFT_ENABLE = 23;
const byte FP_SOL_FLIPPER_RIGHT_ENABLE = 24;


const byte FP_SearchCoils[15] = {1,4,6,8,13,15,16,17,18,19,20,21,22,14,0}; // coils to fire when the ball watchdog timer runs out - has to end with a zero
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
