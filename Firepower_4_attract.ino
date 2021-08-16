/////////////////////////////
//                         //
//  Firepower Attract Mode //
//                         //
/////////////////////////////

// entry point from APC main = EnterAttractMode() which is a delegate for FP_AttractMode()



void FP_AttractMode() {                               // Attract Mode
  ACselectRelay = 0; // assign the number of the A/C select relay
  digitalWrite(VolumePin,HIGH);                       // set volume to zero
  RemoveAllBlinkingLamps();
  LampPattern = NoLamps;
  Switch_Pressed = FP_AttractMode_SW;
  Switch_Released = DummyProcess;
  AppByte2 = 0;
  LampReturn = FP_AttractMode_LampCycle;
  ActivateTimer(1000, 0, FP_AttractMode_LampCycle);
  FP_AttractMode_DisplayCycle(0);
  ejectHoles.ReleaseAll();
  through.ClearOutHole();
}

void FP_AttractMode_LampCycle(byte Event) {                // play multiple lamp pattern series
  UNUSED(Event);
  PatPointer = FP_AttractFlow[AppByte2].FlowPat;      // set the pointer to the current series
  FlowRepeat = FP_AttractFlow[AppByte2].Repeat;       // set the repetitions
  AppByte2++;                                         // increase counter
  if (!FP_AttractFlow[AppByte2].Repeat) {             // repetitions of next series = 0?
    AppByte2 = 0;}                                    // reset counter
  ShowLampPatterns(1);}                               // call the player

void FP_AttractMode_DisplayCycle(byte Step) {
//  FP_CheckForLockedBalls(0);
  switch (Step) {
  case 0:
    WriteUpper2("APC BASE CODE   ");
    ActivateTimer(50, 0, ScrollUpper);
    WriteLower2("                ");
    ActivateTimer(1000, 0, ScrollLower2);
    if (NoPlayers) {                                  // if there were no games before skip the next step
      Step++;}
    else {
      Step = 2;}
    break;
  case 1:
    WriteUpper2("                ");                  // erase display
    WriteLower2("                ");
    for (i=1; i<=NoPlayers; i++) {                    // display the points of all active players
      ShowNumber(8*i-1, Points[i]);}
    ActivateTimer(50, 0, ScrollUpper);
    ActivateTimer(900, 0, ScrollLower2);
    Step++;
    break;
  case 2:                                             // Show highscores
    WriteUpper2("1>              ");
    WriteLower2("2>              ");
    for (i=0; i<3; i++) {
      *(DisplayUpper2+8+2*i) = DispPattern1[(HallOfFame.Initials[i]-32)*2];
      *(DisplayUpper2+8+2*i+1) = DispPattern1[(HallOfFame.Initials[i]-32)*2+1];
      *(DisplayLower2+8+2*i) = DispPattern2[(HallOfFame.Initials[3+i]-32)*2];
      *(DisplayLower2+8+2*i+1) = DispPattern2[(HallOfFame.Initials[3+i]-32)*2+1];}
    ShowNumber(15, HallOfFame.Scores[0]);
    ShowNumber(31, HallOfFame.Scores[1]);
    ActivateTimer(50, 0, ScrollUpper);
    ActivateTimer(900, 0, ScrollLower2);
    Step++;
    break;
  case 3:
    WriteUpper2("3>              ");
    WriteLower2("4>              ");
    for (i=0; i<3; i++) {
      *(DisplayUpper2+8+2*i) = DispPattern1[(HallOfFame.Initials[6+i]-32)*2];
      *(DisplayUpper2+8+2*i+1) = DispPattern1[(HallOfFame.Initials[6+i]-32)*2+1];
      *(DisplayLower2+8+2*i) = DispPattern2[(HallOfFame.Initials[9+i]-32)*2];
      *(DisplayLower2+8+2*i+1) = DispPattern2[(HallOfFame.Initials[9+i]-32)*2+1];}
    ShowNumber(15, HallOfFame.Scores[2]);
    ShowNumber(31, HallOfFame.Scores[3]);
    ActivateTimer(50, 0, ScrollUpper);
    ActivateTimer(900, 0, ScrollLower2);
    Step = 0;}
  ActivateTimer(4000, Step, FP_AttractMode_DisplayCycle);}

void FP_AttractMode_SW(byte Button) {                  // Attract Mode switch behaviour
  switch (Button) {
  case FP_SW_HIGH_SCORE_RESET:                                             // high score reset
    digitalWrite(Blanking, LOW);                      // invoke the blanking
    break;
  //case 20:                                            // outhole
  //  ActivateTimer(200, 0, FP_CheckForLockedBalls);    // check again in 200ms
  //  break;
  case FP_SW_SERVICE_MODE:                                            // Service Mode
    BlinkScore(0);                                    // stop score blinking
    ShowLampPatterns(0);                              // stop lamp animations
    KillAllTimers();
    BallWatchdogTimer = 0;
    CheckReleaseTimer = 0;
    LampPattern = NoLamps;                            // Turn off all lamps
    ReleaseAllSolenoids();
    if (APC_settings[DebugMode]) {                    // deactivate serial interface in debug mode
      Serial.end();
      }
    if (!QuerySwitch(73)) {                           // Up/Down switch pressed?
      WriteUpper("  TEST  MODE    ");
      WriteLower("                ");
      AppByte = 0;
      ActivateTimer(1000, 0, FP_TestMode);}
    else {
      Settings_Enter();}
    break;
  case FP_SW_START_BUTTON:                            // start game
    ShowLampPatterns(0);                            // stop lamp animations
    KillAllTimers();
    for (i=0; i< 8; i++) { LampColumns[i] = 0; }
    LampPattern = LampColumns;
    if (APC_settings[Volume]) // system set to digital volume control ?
    {                     
      analogWrite(VolumePin,255-APC_settings[Volume]); // adjust PWM to volume setting
    }
    else 
    {
      digitalWrite(VolumePin,HIGH); // turn off the digital volume control
    }  
    FP_StartGame();
    break;
  case FP_SW_OUTHOLE:
    ActivateSolenoid(0,FP_SOL_OUTHOLEKICKER);
  }
}
