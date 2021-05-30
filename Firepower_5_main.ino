////////////////////////////
//                        //
//   Firepower Main code  //
//                        //
////////////////////////////

// entry point in FP_StartGame, coming from FP_AttractMode_SW

struct GameDef FP_GameDefinition = {
    FP_setList,                                       // GameSettingsList
    (byte*)FP_defaults,                               // GameDefaultsPointer
    "FP_SET.BIN",                                     // GameSettingsFileName
    "FP_SCORE.BIN",                                   // HighScoresFileName
    FP_AttractMode,                                   // AttractMode
    FP_SolTimes};                                     // Default activation times of solenoids

void FP_init() 
{
	if (APC_settings[DebugMode]) Serial.begin(115200); // activate serial interface in debug mode
  GameDefinition = FP_GameDefinition; // read the game specific settings and highscores
}                

void FP_StartGame()  // start a game, called from Attract mode when start switch is pressed
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
    Ball = 1;
    Player = 1;
    ExBalls = 0;
    Bonus = 1;
    BonusMultiplier = 1;
    InLock = 0;
    Multiballs = 1;
    for (i=1; i < 5; i++) {
      LockedBalls[i] = 0;
      Points[i] = 0;}
    ActivateSolenoid(0, FP_SOL_FLIPPER_LEFT_ENABLE);                        // enable flipper fingers
    ActivateSolenoid(0, FP_SOL_FLIPPER_RIGHT_ENABLE);
    fireLanes.ResetLanes();
    through.ReleaseOneBall();
    Switch_Pressed = FP_GameMain;
    Switch_Released = DummyProcess;
}

void FP_GameMain(byte switchNumber) 
{                        
  switch (switchNumber)                // game switch events
  {
    case FP_SW_PLUMB_BOB_TILT:
    case FP_SW_BALL_ROLL_TILT:
    case FP_SW_SLAM_TILT:
    case FP_SW_PLAYFIELD_TILT:
      break;
    case FP_SW_START_BUTTON:
      FP_AddPlayer();
      break;
    case FP_SW_OUTHOLE:
      ActivateTimer(500, 0, FP_ClearOutHole);
      ActivateTimer(500, 0, FP_BallDrained);
      break;
    case FP_SW_FIRE_ROLLOVERS[0]:
    case FP_SW_FIRE_ROLLOVERS[1]:
    case FP_SW_FIRE_ROLLOVERS[2]:
    case FP_SW_FIRE_ROLLOVERS[3]:
      fireLanes.RollOverActivated(switchNumber);
      if (fireLanes.AllLanesOn()) ActivateTimer(200, 0, FP_Fire);
      break;
    case FP_SW_LANE_CHANGE_SWITCH:
      fireLanes.LaneChange();
      break;
    case FP_SW_EJECT_HOLES[0]:
    case FP_SW_EJECT_HOLES[1]:
    case FP_SW_EJECT_HOLES[2]:
      bool ballWasKept = ejectHoles.BallInHole(switchNumber);
      if (ballWasKept) ActivateTimer(500, 0, FP_BallLocked);
      break;
  }
}

void FP_BallDrained(byte event)
{
  UNUSED(event);
  through.ReleaseOneBall();
}

void FP_Fire(byte event)
{
  UNUSED(event);
  TurnOnLamp(FP_LP_FIRE);
}

void FP_BallLocked(byte event)
{
  
}

void FP_AddPlayer()
{

}
