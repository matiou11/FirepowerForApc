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

// global variables used by Firepower
bool FP_BallSaverKickOn;
bool FP_Mode_Multiball;
bool FP_SpinnerLit;
bool FP_ExtraBallLit;
bool FP_ExtraBallValidated;
bool FP_InLaneLeftLit;
bool FP_InLaneRightLit;
bool FP_FireLit;
bool FP_PowerLit;
// global flags used by Firepower
bool FP_flagIgnorePlungerLaneSwitch;
// Objects needed by Firepower
APC_RolloverLanes fireLanes(FP_NB_FIRE_LANES, FP_SW_FIRE_ROLLOVERS, FP_LP_FIRE_LANES);
APC_Through through(FP_NB_INSTALLED_BALLS, FP_SW_THROUGH, FP_SW_PLUNGER_LANE, FP_SW_OUTHOLE, FP_SOL_SHOOTERLANEFEEDER, FP_SOL_OUTHOLEKICKER);
APC_Eject_Holes ejectHoles(FP_NB_EJECT_HOLES, FP_SW_EJECT_HOLES, FP_LP_EJECT_HOLES, FP_SOL_EJECT_HOLES);
APC_Bumpers bumpers(FP_NB_BUMPERS, FP_SW_BUMPERS, FP_LP_BUMPERS, FP_SOL_BUMPERS, 0);
APC_Flippers flippers(FP_NB_FLIPPERS, FP_SOL_FLIPPER_ENABLE);
APC_Slings slings(FP_NB_SLINGS, FP_SW_SLINGS, FP_SOL_SLINGS);
APC_Standups standUps123(3, FP_SW_123_TARGETS, FP_LP_123_TARGETS, true);
APC_Standups standUps456(3, FP_SW_456_TARGETS, FP_LP_456_TARGETS, true);
APC_Standups standUpsPower(3, FP_SW_POWER_TARGETS, FP_LP_POWER_TARGETS, false);
APC_Sys6Displays displays;
FP_ScoreManager scoreManager; // should be array of 4, one for each player

void FP_init() 
{
	//if (APC_settings[DebugMode]) Serial.begin(115200); // activate serial interface in debug mode
  //Serial.begin(115200);
  GameDefinition = FP_GameDefinition; // read the game specific settings and highscores
}                

void FP_StartGame()  // start a game, called from Attract mode when start switch is pressed
{
    NoPlayers = 1;
    Ball = 1;
    Player = 1;
    ExBalls = 0;
    InLock = 0;
    
    FP_BallSaverKickOn = false;
    FP_Mode_Multiball = false;
    FP_SpinnerLit = false;
    FP_ExtraBallLit = false;
    FP_ExtraBallValidated = false;
    FP_flagIgnorePlungerLaneSwitch = false;
    FP_InLaneLeftLit = false;
    FP_InLaneRightLit = false;
    FP_FireLit = false;
    FP_PowerLit = false;
    
    displays.resetScoreDisplays();
    displays.UpdateCredit(Ball);
    flippers.Enable(true);
    slings.Enable(true);
    bumpers.Enable(true);
    fireLanes.Enable(true);
    standUps123.Enable(true);
    standUps456.Enable(true);
    standUpsPower.Enable(true);
    ejectHoles.Enable(true);
    scoreManager.Enable(true);
    FP_IncrementScore(0, true); // game starts with bonus = 1000
    through.ReleaseOneBall();
    Switch_Pressed = FP_Game_SwitchPressed;
    Switch_Released = FP_Game_SwitchReleased;
}


void FP_Game_SwitchReleased(byte switchNumber) 
{
  switch(switchNumber)
  {
    case FP_SW_PLUNGER_LANE:
      if (!FP_flagIgnorePlungerLaneSwitch) PlaySound(50, "0_6b.snd");
      break;
  }
}

void FP_Game_SwitchPressed(byte switchNumber) 
{                        
  switch (switchNumber)                // game switch events
  {
    case FP_SW_BUMPERS[0]:
    case FP_SW_BUMPERS[1]:
    case FP_SW_BUMPERS[2]:
    case FP_SW_BUMPERS[3]:
    {
      byte bumperIndex = bumpers.SwitchNumberToBumperIndex(switchNumber);
      bool ret = bumpers.Bump(bumperIndex); // ret = true when lit and false when turned off
      if (ret) FP_IncrementScore(10000, false); else FP_IncrementScore(100, false);
      break;
    }
    case FP_SW_SLINGS[0]:
    case FP_SW_SLINGS[1]:
    {
      byte slingIndex = slings.SwitchNumberToSlingIndex(switchNumber);
      slings.Sling(slingIndex);
      FP_IncrementScore(10, true);
      break;      
    }
    case FP_SW_LEFT_OUTSIDE_ROLLOVER:
      if (FP_BallSaverKickOn)
      {
        FP_Left_KickBack(0); // ActivateTimer(50, 0, FP_Left_KickBack);
        FP_BallSaverKickOn = false;
        TurnOffLamp(FP_LP_BALL_SAVER_KICK_ON);
      }
      break;
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
      ActivateTimer(1000, 0, FP_BallDrained);
      ActivateTimer(2000, 0, FP_BallDrained); // try again later, just in case
      break;
    case FP_SW_FIRE_ROLLOVERS[0]:
    case FP_SW_FIRE_ROLLOVERS[1]:
    case FP_SW_FIRE_ROLLOVERS[2]:
    case FP_SW_FIRE_ROLLOVERS[3]:
    {
      byte laneIndex = fireLanes.SwitchNumberToLaneIndex(switchNumber);
      byte ret = fireLanes.RollOverActivated(laneIndex);
      FP_IncrementScore(1000, ret != 0); // bonus only if first time through
      if ( ret == 2 ) FP_FireActivated(0);
      break;
    }
    case FP_SW_LANE_CHANGE:
      fireLanes.LaneChange();
      break;
    case FP_SW_EJECT_HOLES[0]:
    case FP_SW_EJECT_HOLES[1]:
    case FP_SW_EJECT_HOLES[2]:
    {
      byte holeIndex = ejectHoles.SwitchNumberToHoleIndex(switchNumber);
      byte ret = ejectHoles.BallInHole(holeIndex); // 0 hole was full, 1 ball not accepted, 2 ball kept
      if (ret == 1) FP_IncrementScore(1000, false);
      else if (ret == 2) {FP_IncrementScore(10000, false); ActivateTimer(500, 0, FP_BallLocked);}
      break;
    }
    case FP_SW_123_TARGETS[0]:
    case FP_SW_123_TARGETS[1]:
    case FP_SW_123_TARGETS[2]:
    {
      byte standUpIndex = standUps123.SwitchNumberToStandUpIndex(switchNumber);
      bool ret = standUps123.Hit(standUpIndex);
      FP_IncrementScore(1000, ret); // bonus only if first time hit
      if (standUps123.AllHit()) ActivateTimer(200, 0, FP_StandUpNumbersAllHit);
      break;
    }
    case FP_SW_456_TARGETS[0]:
    case FP_SW_456_TARGETS[1]:
    case FP_SW_456_TARGETS[2]:
    {
      byte standUpIndex = standUps456.SwitchNumberToStandUpIndex(switchNumber);
      bool ret = standUps456.Hit(standUpIndex);
      FP_IncrementScore(1000, ret); // bonus only if first time hit
      if (standUps456.AllHit()) ActivateTimer(200, 0, FP_StandUpNumbersAllHit);
      break;
    }
    case FP_SW_POWER_TARGETS[0]:
    case FP_SW_POWER_TARGETS[1]:
    case FP_SW_POWER_TARGETS[2]:
    {
      byte standUpIndex = standUpsPower.SwitchNumberToStandUpIndex(switchNumber);
      bool ret = standUpsPower.Hit(standUpIndex);
      FP_IncrementScore(1000, false);
      if (standUpsPower.AllHit()) ActivateTimer(200, 0, FP_PowerActivated);
      if (FP_ExtraBallLit && switchNumber == FP_SW_POWER_TARGETS[1] && ret) 
      {
        FP_ExtraBallValidated = true;
        TurnOnLamp(FP_LP_SHOOT_AGAIN_PF);
        FP_ExtraBallLit = false;
        TurnOffLamp(FP_LP_EXTRABALL_WHEN_LIT);
      }
      break;
    }
    case FP_SW_SPINNER:
      if (FP_SpinnerLit) FP_IncrementScore(1000, false); else FP_IncrementScore(100, false);
      break;
    case FP_SW_TOP_TARGET:
      FP_IncrementScore(1000, true);
      break;
    case FP_SW_STAR_ROLLOVER_LEFT:
    case FP_SW_STAR_ROLLOVER_RIGHT:
      FP_IncrementScore(1000, false);
      break;
    case FP_SW_INLANE_LEFT:
      FP_IncrementScore(1000, true);
      if (FP_InLaneLeftLit) { FP_IncrementScore(1000, true); FP_IncrementScore(1000, true); }
      break;
    case FP_SW_INLANE_RIGHT:
      FP_IncrementScore(1000, true);
      if (FP_InLaneRightLit) { FP_IncrementScore(1000, true); FP_IncrementScore(1000, true); }
      break;
    case FP_SW_BUMPER_SIDES[0]:
    case FP_SW_BUMPER_SIDES[1]:
    case FP_SW_BUMPER_SIDES[2]:
    case FP_SW_BUMPER_SIDES[3]:
    case FP_SW_BUMPER_SIDES[4]:
    case FP_SW_BUMPER_SIDES[5]:
    case FP_SW_BUMPER_SIDES[6]:
      FP_BumperSidesHit(0);
      ActivateTimer(50, 0, FP_BumperSidesHit);
      ActivateTimer(100, 0, FP_BumperSidesHit);
      ActivateTimer(150, 0, FP_BumperSidesHit);
      ActivateTimer(200, 0, FP_BumperSidesHit);
      break;
    
  }
}

void FP_BallDrained(byte event)
{
  UNUSED(event);
  // TODO: avoid quick reentry into this function (wasting a ball) 
  if (through.NbBallsIn() + ejectHoles.NbBallsLocked() == FP_NB_INSTALLED_BALLS) // are all balls accounted for ?
  {
    // next ball except if extra ball was validated
    if (!FP_ExtraBallValidated) Ball++;
    TurnOffLamp(FP_LP_SHOOT_AGAIN_PF);
    FP_ExtraBallValidated = false;
    FP_Mode_Multiball = false;
    displays.UpdateCredit(Ball);
    through.ReleaseOneBall();
  }
}

void FP_FireActivated(byte event)
{
  UNUSED(event);
  FP_FireLit = true;
  TurnOnLamp(FP_LP_FIRE);
  bool ret = scoreManager.IncrementBonusMultiplier(); // true if succesful, false if already maxed out
  if (!ret)
  {
    FP_ExtraBallLit = true;
    TurnOnLamp(FP_LP_EXTRABALL_WHEN_LIT);
    standUpsPower.Reset();
  }
  if (FP_PowerLit) FP_FirePowerActivated(0);
}

void FP_PowerActivated(byte event)
{
  UNUSED(event);
  FP_PowerLit = true;
  TurnOnLamp(FP_LP_POWER);
  standUpsPower.Reset();
  FP_InLaneLeftLit = true;
  TurnOnLamp(FP_LP_INLANE_LEFT);
  FP_InLaneRightLit = true;
  TurnOnLamp(FP_LP_INLANE_RIGHT);
  if (FP_FireLit) FP_FirePowerActivated(0);
}

void FP_FirePowerActivated(byte event)
{
  // blink and reset
  TurnOffLamp(FP_LP_POWER);
  AddBlinkLamp(FP_LP_POWER, 100);
  ActivateTimer(500, FP_LP_POWER, FP_RemoveBlink);
  TurnOffLamp(FP_LP_FIRE);
  AddBlinkLamp(FP_LP_FIRE, 100);
  ActivateTimer(500, FP_LP_FIRE, FP_RemoveBlink);
  scoreManager.CollectAndIncrementFirepowerBonus();
  displays.UpdatePlayerScore(1,scoreManager.Score());
}

void FP_StandUpNumbersAllHit(byte event)
{
  UNUSED(event);
  FP_BallSaverKickOn = true;
  TurnOnLamp(FP_LP_BALL_SAVER_KICK_ON);
  if (standUps123.AllHit() && standUps456.AllHit())
  {
    bumpers.LightMode(-1); // increase bumper light mode
    FP_SpinnerLit = true;
    TurnOnLamp(FP_LP_SPINNER_LIT);
    ejectHoles.ActivateHole(255); // activate randomly one hole
    ejectHoles.ActivateHole(255); // activate randomly a second hole
    standUps123.Reset();
    standUps456.Reset();
  }
}

void FP_BallLocked(byte event)
{
  if (ejectHoles.NbBallsLocked() < FP_NB_INSTALLED_BALLS)
  {
    FP_ThroughReleaseOneBall(0);
  }
  else
  {
    ActivateTimer(500, 0, FP_MultiballStart);
  }
}

void FP_MultiballStart(byte event)
{
  FP_Mode_Multiball = true;
  // release balls!
  ActivateTimer(1000, 2, FP_ReleaseLock);
  ActivateTimer(3000, 1, FP_ReleaseLock);
  ActivateTimer(5000, 0, FP_ReleaseLock);
  ejectHoles.Reset();
}

void FP_BumperSidesHit(byte event)
{
  FP_IncrementScore(10,false);
}

void FP_IncrementScore(int points, bool bonus)
{
  scoreManager.IncrementScore(points);
  displays.UpdatePlayerScore(1,scoreManager.Score());
  if (bonus) scoreManager.Add1000ToBonus();
}

void FP_UpdateAllPlayfieldLamps()
{
 
}

  
void FP_AddPlayer()
{

}

void FP_Left_KickBack(byte event)
{
  ActivateSolenoid(0, FP_SOL_LEFT_BALL_SAVER_KICKER);
}

// Interfaces for timers 

void FP_ClearOutHole(byte event)
{
    UNUSED(event);
    through.ClearOutHole();
}

void FP_ReleaseLock(byte index)
{
    ejectHoles.ReleaseLock(index);
}

void FP_RemoveBlink(byte lampIndex)
{
  RemoveBlinkLamp(lampIndex);
}

void FP_Reactivate_Flag(byte flagIndex)
{
  FP_flagIgnorePlungerLaneSwitch = false;
}

void FP_ThroughReleaseOneBall(byte event)
{
    UNUSED(event);
    FP_flagIgnorePlungerLaneSwitch = true; // to avoid launch sound
    through.ReleaseOneBall();
    ActivateTimer(350, 0, FP_Reactivate_Flag);
}
