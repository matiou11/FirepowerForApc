////////////////////////////
//                        //
//   Firepower Main code  //
//                        //
////////////////////////////

// entry point from APC main = EnterAttractMode() which is a delegate for FP_AttractMode()

struct GameDef FP_GameDefinition = {
    FP_setList,                                       // GameSettingsList
    (byte*)FP_defaults,                               // GameDefaultsPointer
    "FP_SET.BIN",                                     // GameSettingsFileName
    "FP_SCORE.BIN",                                   // HighScoresFileName
    FP_AttractMode,                                   // AttractMode
    FP_SolTimes};                                     // Default activation times of solenoids

FP_FireLanes fireLanes;

void FP_init() 
{
	if (APC_settings[DebugMode]) Serial.begin(115200); // activate serial interface in debug mode
  GameDefinition = FP_GameDefinition; // read the game specific settings and highscores
}                

void FP_StartGame()  // start a game, called from Attract mode when start switch is pressed
{
  if (FP_CountBallsInTrunk() == FP_INSTALLED_BALLS || (FP_CountBallsInTrunk() == game_settings[BCset_InstalledBalls]-1 && QuerySwitch(game_settings[BCset_PlungerLaneSwitch])))  // Ball missing?
  { 
    Switch_Pressed = DummyProcess;                  // Switches do nothing
    ShowLampPatterns(0);                            // stop lamp animations
    KillAllTimers();
    if (APC_settings[Volume]) {                     // system set to digital volume control?
      analogWrite(VolumePin,255-APC_settings[Volume]);} // adjust PWM to volume setting
    else {
      digitalWrite(VolumePin,HIGH);}                // turn off the digital volume control
    for (i=0; i< 8; i++) {                          // turn off all lamps
      LampColumns[i] = 0;}
    LampPattern = LampColumns;
    NoPlayers = 0;
    WriteUpper("                ");
    WriteLower("                ");
    Ball = 1;
    FP_AddPlayer();
    Player = 1;
    ExBalls = 0;
    Bonus = 1;
    BonusMultiplier = 1;
    InLock = 0;
    Multiballs = 1;
    for (i=1; i < 5; i++) {
      LockedBalls[i] = 0;
      Points[i] = 0;}
    fireLanes.ResetLanes();
    FP_NewBall(game_settings[BCset_InstalledBalls]); // release a new ball (3 expected balls in the trunk)
    ActivateSolenoid(0, FP_SOL_FLIPPER_LEFT_ENABLE);                        // enable flipper fingers
    ActivateSolenoid(0, FP_SOL_FLIPPER_RIGHT_ENABLE);
  }
}

void FP_GameMain(byte Event) {                        // game switch events
  switch (Event) {
  case FP_SW_PLUMB_BOB_TILT:                                             // plumb bolt tilt
  case FP_SW_BALL_ROLL_TILT:                                             // ball roll tilt
  case FP_SW_SLAM_TILT:                                             // slam tilt
    break;
  case FP_SW_PLAYFIELD_TILT:                                            // playfield tilt
    WriteUpper(" TILT  WARNING  ");
    ActivateTimer(3000, 0, ShowAllPoints);
    break;
  case FP_SW_START_BUTTON:                                             // credit button
    FP_AddPlayer();
    break;
  default:
  	if (Event == game_settings[BCset_OutholeSwitch]) {
      ActivateTimer(200, 0, FP_ClearOuthole);}        // check again in 200ms
  }}

void FP_BallEnd(byte Event) {
  byte BallsInTrunk = FP_CountBallsInTrunk();
  if ((BallsInTrunk == 5)||(BallsInTrunk < game_settings[BCset_InstalledBalls]+1-Multiballs-InLock)) {
    InLock = 0;
//    if (Multiballs == 1) {
//      for (i=0; i<3; i++) {                           // Count your locked balls here
//        if (Switch[41+i]) {
//          InLock++;}}}
    WriteLower(" BALL   ERROR   ");
    if (QuerySwitch(game_settings[BCset_OutholeSwitch])) { // ball still in outhole?
      ActA_BankSol(game_settings[BCset_OutholeKicker]); // make the coil a bit stronger
      ActivateTimer(2000, Event, FP_BallEnd);}        // and come back in 2s
    else {
      if (Event < 11) {                               // have I been here already?
        Event++;
        ActivateTimer(1000, Event, FP_BallEnd);}      // if not try again in 1s
      else {                                          // ball may be still in outhole
        BlockOuthole = false;
        Event = 0;
        FP_ClearOuthole(0);}}}
  else {
    switch (Multiballs) {
    case 3:                                           // goto 2 ball multiball
      Multiballs = 2;
      if (BallsInTrunk != 1) {                        // not 1 ball in trunk
        ActivateTimer(1000, 0, FP_BallEnd);}          // check again later
      else {
        BlockOuthole = false;}                        // remove outhole block
      break;
    case 2:                                           // end multiball
      Multiballs = 1;
      if (BallsInTrunk == game_settings[BCset_InstalledBalls]) { // all balls in trunk?
        ActivateTimer(1000, 0, FP_BallEnd);}
      else {
        BlockOuthole = false;}                        // remove outhole block
      break;
    case 1:                                           // end of ball
      if (BallsInTrunk + InLock != game_settings[BCset_InstalledBalls]) {
        WriteUpper(" COUNT  ERROR   ");
        InLock = 0;
//        for (i=0; i<3; i++) {                       // check how many balls are on the ball ramp
//          if (Switch[41+i]) {
//            InLock++;}}
        ActivateTimer(1000, 0, FP_BallEnd);}
      else {
        LockedBalls[Player] = 0;
        BlinkScore(0);                                // stop score blinking
        FP_BallEnd2(BallsInTrunk);                    // add bonus count here and start BallEnd2 afterwards
      }}}}

void FP_BallEnd2(byte Balls) {
  if (BallWatchdogTimer) {
    KillTimer(BallWatchdogTimer);
    BallWatchdogTimer = 0;}
  if (ExBalls) {                                      // Player has extra balls
    ExBalls--;
    ActivateTimer(1000, AppByte, FP_NewBall);
    BlockOuthole = false;}                            // remove outhole block
  else {                                              // Player has no extra balls
    if ((Points[Player] > HallOfFame.Scores[3]) && (Ball == APC_settings[NofBalls])) { // last ball & high score?
      Switch_Pressed = DummyProcess;                  // Switches do nothing
      FP_CheckHighScore(Player);}
    else {
      FP_BallEnd3(Balls);}}}

void FP_BallEnd3(byte Balls) {
  BlockOuthole = false;                               // remove outhole block
  if (Player < NoPlayers) {                           // last player?
    Player++;
    ActivateTimer(1000, Balls, FP_NewBall);}
  else {
    if (Ball < APC_settings[NofBalls]) {
      Player = 1;
      Ball++;
      ActivateTimer(1000, Balls, FP_NewBall);}
    else {                                            // game end
      ReleaseSolenoid(FP_SOL_FLIPPER_LEFT_ENABLE);                            // disable flipper fingers
      ReleaseSolenoid(FP_SOL_FLIPPER_RIGHT_ENABLE);
      FP_CheckForLockedBalls(0);
      GameDefinition.AttractMode();}}}
