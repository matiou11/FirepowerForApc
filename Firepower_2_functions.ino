// Firepower functions

void FP_AddPlayer() {
  if ((NoPlayers < 4) && (Ball == 1)) {               // if actual number of players < 4
    NoPlayers++;                                      // add a player
    Points[NoPlayers] = 0;                            // delete the points of the new player
    ShowPoints(NoPlayers);}}                          // and show them

void FP_CheckForLockedBalls(byte Event) {             // check if balls are locked and release them
  UNUSED(Event);
  if (QuerySwitch(FP_SW_OUTHOLE)) {                     // for the outhole
    ActA_BankSol(FP_SOL_OUTHOLEKICKER);}
}                                                     // add the locks of your game here

void FP_NewBall(byte Balls) {                         // release ball (Event = expected balls on ramp)
  ShowAllPoints(0);
  if (APC_settings[DisplayType] < 2) {                // credit display present?
    *(DisplayUpper+16) = LeftCredit[32 + 2 * Ball];}  // show current ball in left credit
  BlinkScore(1);                                      // start score blinking
  Switch_Released = FP_CheckShooterLaneSwitch;
  if (!QuerySwitch(FP_SW_PLUNGER_LANE)) {
    ActA_BankSol(FP_SOL_SHOOTERLANEFEEDER);               // release ball
    Switch_Pressed = FP_BallReleaseCheck;             // set switch check to enter game
    CheckReleaseTimer = ActivateTimer(5000, Balls-1, FP_CheckReleasedBall);} // start release watchdog
  else {
    Switch_Pressed = FP_ResetBallWatchdog;}}

void FP_CheckShooterLaneSwitch(byte Switch) {
  if (Switch == FP_SW_PLUNGER_LANE) { // shooter lane switch released?
    Switch_Released = DummyProcess;
    if (!BallWatchdogTimer) {
      BallWatchdogTimer = ActivateTimer(30000, 0, FP_SearchBall);}}}

void FP_BallReleaseCheck(byte Switch) {               // handle switches during ball release
  if (Switch > 15) {                                  // edit this to be true only for playfield switches
    if (CheckReleaseTimer) {
      KillTimer(CheckReleaseTimer);
      CheckReleaseTimer = 0;}                         // stop watchdog
    Switch_Pressed = FP_ResetBallWatchdog;
    if (Switch == FP_SW_PLUNGER_LANE) { // ball is in the shooter lane
      Switch_Released = FP_CheckShooterLaneSwitch;}   // set mode to register when ball is shot
    else {
      if (!BallWatchdogTimer) {
        BallWatchdogTimer = ActivateTimer(30000, 0, FP_SearchBall);}}} // set switch mode to game
  FP_GameMain(Switch);}                               // process current switch

void FP_ResetBallWatchdog(byte Switch) {              // handle switches during ball release
  if (Switch > 15) {                                  // edit this to be true only for playfield switches
    if (BallWatchdogTimer) {
      KillTimer(BallWatchdogTimer);}                  // stop watchdog
    BallWatchdogTimer = ActivateTimer(30000, 0, FP_SearchBall);}
  FP_GameMain(Switch);}                               // process current switch

void FP_SearchBall(byte Counter) // ball watchdog timer has run out
{                    
  BallWatchdogTimer = 0;
  if (QuerySwitch(FP_SW_OUTHOLE)) 
  {
    BlockOuthole = false;
    ActivateTimer(1000, 0, FP_ClearOuthole);
  }
  else 
  {
    if (QuerySwitch(FP_SW_PLUNGER_LANE)) // if ball is waiting to be launched
    {
      BallWatchdogTimer = ActivateTimer(30000, 0, FP_SearchBall);  // restart watchdog
    }
    else 
    {                                            // if ball is really missing
      byte c = FP_CountBallsInTrunk();                // recount all balls
      if (c == FP_INSTALLED_BALLS) // found all balls in trunk?
      {
        if (BlockOuthole) 
        {                           // is the outhole blocked
          FP_BallEnd(0);                             // then it was probably a ball loss gone wrong
        }
        else 
        {
          ActivateTimer(1000, FP_INSTALLED_BALLS, FP_NewBall); // otherwise try it with a new ball
        }
      }
      else 
      {
        byte c2 = 0;                                  // counted balls in lock
                  // count balls in lock here with 5 being a warning when the switch states don't add up
        if (c == 5)                                  // balls have not settled yet
        {
          WriteUpper("  LOCK  STUCK   ");
          BallWatchdogTimer = ActivateTimer(1000, 0, FP_SearchBall); // and try again in 1s
        }
        else 
        {
          if (c2 > InLock)                          // more locked balls found than expected?
          {
            FP_HandleLock(0);                         // lock them
            BallWatchdogTimer = ActivateTimer(30000, 0, FP_SearchBall);
          }
          else 
          {
            WriteUpper("  BALL  SEARCH  ");
            ActivateSolenoid(0, FP_SearchCoils[Counter]); // fire coil to get ball free
            Counter++;
            if (!FP_SearchCoils[Counter])            // all coils fired?
            {
              Counter = 0;                           // start again
            }
            BallWatchdogTimer = ActivateTimer(1000, Counter, FP_SearchBall); // come again in 1s if no switch is activated
          }
        }
      }
    }
  }
}

byte FP_CountBallsInTrunk() {
  byte Balls = 0;
  for (i=0; i < FP_INSTALLED_BALLS; i++) { // check how many balls are on the ball ramp
    if (QuerySwitch(FP_SW_THROUGH[i])) {
      if (Balls < i) {
        return 5;}                                    // send warning
      Balls++;}}
  return Balls;}

void FP_CheckReleasedBall(byte Balls) {               // ball release watchdog
  CheckReleaseTimer = 0;
  BlinkScore(0);                                      // stop blinking to show messages
  WriteUpper("WAITINGFORBALL  ");                     // indicate a problem
  WriteLower("                ");
  if (Balls == 10) {                                  // indicating a previous trunk error
    WriteUpper("                ");
    WriteLower("                ");
    ShowAllPoints(0);
    BlinkScore(1);
    ActA_BankSol(FP_SOL_SHOOTERLANEFEEDER);}
  byte c = FP_CountBallsInTrunk();
  if (c == Balls) {                                   // expected number of balls in trunk
    WriteUpper("  BALL MISSING  ");
    if (QuerySwitch(FP_SW_OUTHOLE)) { // outhole switch still active?
      ActA_BankSol(FP_SOL_OUTHOLEKICKER);}}  // shove the ball into the trunk
  else {                                              //
    if (c == 5) {                                     // balls not settled
      WriteLower(" TRUNK  ERROR   ");
      Balls = 10;}
    else {
      if ((c > Balls) || !c) {                        // more balls in trunk than expected or none at all
        WriteUpper("                ");
        WriteLower("                ");
        ShowAllPoints(0);
        BlinkScore(1);
        ActA_BankSol(FP_SOL_OUTHOLEKICKER);}}} // release again
  CheckReleaseTimer = ActivateTimer(5000, Balls, FP_CheckReleasedBall);}

void FP_ClearOuthole(byte Event) {
  UNUSED(Event);
  if (QuerySwitch(game_settings[BCset_OutholeSwitch])) { // outhole switch still active?
    if (!BlockOuthole && !C_BankActive) {             // outhole blocked?
      BlockOuthole = true;                            // block outhole until this ball has been processed
      ActivateSolenoid(30, game_settings[BCset_OutholeKicker]); // put ball in trunk
      ActivateTimer(2000, 0, FP_BallEnd);}
    else {
      ActivateTimer(2000, 0, FP_ClearOuthole);}}}     // come back in 2s if outhole is blocked

void FP_HandleLock(byte Balls) {
      // do something with your lock
}


void FP_ResetHighScores(bool change) {                // delete the high scores file
  if (change) {                                       // if the start button has been pressed
    if (SDfound) {
      SD.remove(GameDefinition.HighScoresFileName);
      struct HighScores HScBuffer;                    // create new high score table
      HallOfFame = HScBuffer;                         // copy it to the hall of fame
      WriteLower(" SCORES DONE    ");}
    else {
      WriteLower(" SCORES NO SD   ");}}
  else {
    WriteLower(" SCORES         ");}}

void FP_CheckHighScore(byte Player) {

}
