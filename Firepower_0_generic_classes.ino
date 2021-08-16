// Max number of items per object
// Could be avoided by dynamically creating arrays inside of objects, but I prefer avoiding this in "embedded" software
constexpr byte MAX_NB_STANDUP_TARGETS_PER_BANK = 3;
constexpr byte MAX_NB_BUMPERS = 4;
constexpr byte MAX_NB_EJECT_HOLES = 3;
constexpr byte MAX_NB_LANES = 4;
constexpr byte MAX_NB_BALLS = 3;
constexpr byte MAX_NB_FLIPPERS = 2;
constexpr byte MAX_NB_SLINGS = 2;
constexpr byte MAX_NB_LAMPS_IN_BANKS = 12;

// Lamp Tools, to be integrated in apc ?
void ToggleLampIfNeeded(byte lampIndex, bool wantedState)
{
  if ( QueryLamp(lampIndex) != wantedState) ToggleLamp(lampIndex);
}
void ToggleLamp(byte lampIndex)
{
  if ( QueryLamp(lampIndex) ) TurnOffLamp(lampIndex); else TurnOnLamp(lampIndex);
}
void BlinkLampBriefly(byte lampIndex, int blinkPeriodMs, int blinkDurationMs)
{
  TurnOffLamp(lampIndex);
  AddBlinkLamp(lampIndex, blinkPeriodMs);
  ActivateTimer(blinkDurationMs, lampIndex, RemoveBlinkLamp);
}
void RemoveAllBlinkingLamps()
{
  for (int index = 1; index <= 64; index++) RemoveBlinkLamp(index);
}

 // Debug tools
class APC_Debug
{
  private:
    bool enabled;
  public:
    void enable(bool enabled)
    {
      this->enabled = enabled;
      if (enabled) Serial.begin(115200);
    }
    void write(String msg, int integer)
    {
      if (enabled)
      {
        Serial.print(msg);
        Serial.println(integer);
      }
    }
    void write(String msg)
    {
      if (enabled)
      {
        Serial.println(msg);
      }
    }
    void write(String name, String msg, int integer)
    {
      if (enabled)
      {
        Serial.print(name);
        Serial.print(msg);
        Serial.println(integer);
      }
    }
    void write(String name, String msg)
    {
      if (enabled)
      {
        Serial.print(name);
        Serial.println(msg);
      }
    }
};

APC_Debug debug;

// APC_RolloverLanes
// Class to materialize roll over lanes ("F" "I" "R" "E" on Firepower)
//   byte nbLanes: number of lanes
//   const byte *switches: array with switch numbers (one per lane)
//   const byte *lamps: array of lamps (one per lane)
class APC_RolloverLanes
{
  private:
    byte nbLanes;
    bool laneStatus[MAX_NB_LANES];
    byte switches[MAX_NB_LANES];
    byte lamps[MAX_NB_LANES];
    bool enabled;
    void blinkAndReset()
    {
      for (int i = 0; i < nbLanes; i++)
      {
        laneStatus[i] = false;
        BlinkLampBriefly(lamps[i],100,1000);
      }
    }
  public:
    APC_RolloverLanes(byte nbLanes, const byte *switches, const byte *lamps)
    {
      this->nbLanes = nbLanes;
      for (int i = 0; i < nbLanes; i++) { this->switches[i] = switches[i]; this->lamps[i] = lamps[i]; }
      enabled = false;
    }
    void Enable(bool enable)
    {
      enabled = enable;
      Reset();
    }
    void Reset()
    {
      for ( int i = 0 ; i < nbLanes ; i++ ) laneStatus[i] = false;
      UpdateLights();
    }
    byte RollOverActivated(byte laneIndex) // function to call when a rollover switch is activated. 
    // returns 0: lane was activated already. 1: lane got activated // 2: all lanes are now activated
    {
      if (!laneStatus[laneIndex])
      {
        laneStatus[laneIndex] = true;
        UpdateLights();
        if (AllLanesOn())
        {
          blinkAndReset();
          return 2;
        }
        else
        {
          return 1;
        }
      }
      return 0; // the lane was activated already
    }
    bool AllLanesOn()
    {
      bool allLanesOn = true;
      for ( int i = 0 ; i < nbLanes ; i++ )
        if (laneStatus[i] == false) allLanesOn = false;
      return (allLanesOn && enabled);
    }
    void LaneChange() // shift all values to the right
    {
      bool last = laneStatus[nbLanes - 1];
      for ( int i = nbLanes - 1 ; i > 0  ; i-- ) laneStatus[i] = laneStatus[i-1];
      laneStatus[0] = last;
      UpdateLights();
    }
    void UpdateLights()
    {
      for ( int i = 0 ; i < nbLanes ; i++ )
        if (laneStatus[i] && enabled) TurnOnLamp(lamps[i]);
        else TurnOffLamp(lamps[i]);
    }
    byte SwitchNumberToLaneIndex(byte switchNumber)
    {
      for ( int i = 0 ; i < nbLanes ; i++ ) if (switches[i] == switchNumber) return i;
      return 0;
    }
};

// APC_Through
// Class to materialize through, but also outhole and shooter lane feeder
//   byte nbBalls: number of balls in the through
//   const byte *throughSwitches: array with through switch numbers
//   byte plungerLaneSwitch: switch number for the plunger lane
//   byte outHoleSwitch: switch number for the outhole
//   byte shooterLaneFeederSol: solenoid number for the plunger lane feeder
//   byte outHoleKickerSol: solenoid number for the outhole kicker
class APC_Through
{
  private:
    byte nbBalls;
    byte throughSwitches[MAX_NB_BALLS];
    byte plungerLaneSwitch;
    byte outHoleSwitch;
    byte shooterLaneFeederSol;
    byte outHoleKickerSol;
  public:
  APC_Through(byte nbBalls, const byte *throughSwitches, byte plungerLaneSwitch, byte outHoleSwitch, byte shooterLaneFeederSol, byte outHoleKickerSol)
  {
    this->nbBalls = nbBalls;
    for (int i = 0; i < nbBalls; i++) this->throughSwitches[i] = throughSwitches[i];
    this->plungerLaneSwitch = plungerLaneSwitch;
    this->outHoleSwitch = outHoleSwitch;
    this->shooterLaneFeederSol = shooterLaneFeederSol;
    this->outHoleKickerSol = outHoleKickerSol;
  }
  byte NbBallsIn()
  {
    byte nb = 0;
    for ( int i = 0 ; i < nbBalls ; i++ ) if (QuerySwitch(throughSwitches[i])) nb++;
    return nb;
  }
  void ReleaseOneBall()
  {
    // release a ball in the plunger lane if it's not occupied already
    if (!QuerySwitch(plungerLaneSwitch)) ActivateSolenoid(0,shooterLaneFeederSol);
  }
  bool OutHoleBlocked()
  {
    return QuerySwitch(outHoleSwitch);
  }
  void ClearOutHole()
  {
    if (OutHoleBlocked()) ActivateSolenoid(0,outHoleKickerSol);
  }
};

// APC_Eject_Holes
// Class to materialize eject holes
//   byte nbHoles
//   const byte *switches
//   const byte *lamps
//   const byte *solenoids
class APC_Eject_Holes
{
  private:
    byte nbHoles;
    bool enabled;
    bool holeActivated[MAX_NB_EJECT_HOLES];
    bool holeFull[MAX_NB_EJECT_HOLES];
    byte switches[MAX_NB_EJECT_HOLES];
    byte lamps[MAX_NB_EJECT_HOLES];
    byte solenoids[MAX_NB_EJECT_HOLES];
  public:
  APC_Eject_Holes(byte nbHoles, const byte *switches, const byte *lamps, const byte *solenoids)
  {
    this->nbHoles = nbHoles;
    enabled = false;
    for (int i = 0; i < nbHoles; i++) { this->switches[i] = switches[i]; this->solenoids[i] = solenoids[i]; this->lamps[i] = lamps[i];}
  }
  void Reset()
  {
    for ( int i = 0 ; i < nbHoles ; i++ )
    {
      holeActivated[i] = false;
      holeFull[i] = false;
    }
    UpdateLights();
  }
  void Enable(bool enable)
  {
    enabled = enable;
    Reset();
  }
  byte NbBallsLocked()
  {
    byte nb = 0;
    for ( int i = 0 ; i < nbHoles ; i++ ) if (holeFull[i] && enabled) nb++;
    return nb;
  }
  void ReleaseLock(byte holeIndex)
  {
    if (QuerySwitch(switches[holeIndex])) { ActivateSolenoid(0, solenoids[holeIndex]); holeFull[holeIndex] = false; }
  }
  void ReleaseAll()
  {
    for ( int i = 0 ; i < nbHoles ; i++ ) ReleaseLock(i);
  }
  void ActivateHole(byte holeIndex)
  {
    if (holeIndex == 255) // activate a random hole
    {
      // replace with random
      if (!holeActivated[0]) holeActivated[0] = true;
      else if (!holeActivated[1]) holeActivated[1] = true;
      else if (!holeActivated[2]) holeActivated[2] = true;
    }
    else
    {
      holeActivated[holeIndex] = true;
    }
    UpdateLights();
  }
  byte BallInHole(byte holeIndex) // return 2 if kept, return 1 if rejected, return 0 if hole was busy
  {
    if (!holeFull[holeIndex]) // is hole taken already ? if yes, do nothing
    {
      if (holeActivated[holeIndex]) // accept ball ?
      {
        holeFull[holeIndex] = true;
        UpdateLights();
        return 2;
      }
      else
      {
        // reject ball
        ActivateTimer(500, holeIndex, FP_ReleaseLock);
        return 1;
      }
    }
    return 0;
  }
  byte SwitchNumberToHoleIndex(byte SwitchNumber)
  {
    for ( int i = 0 ; i < nbHoles ; i++ ) if (switches[i] == SwitchNumber) return i;
    return 0;
  }
  void UpdateLights()
  {
    for ( int i = 0 ; i < nbHoles ; i++ )
    {
      if (holeFull[i] && enabled)
      {
        RemoveBlinkLamp(lamps[i]);
        TurnOnLamp(lamps[i]);
      }
      else if (holeActivated[i] && enabled)
      {
        AddBlinkLamp(lamps[i], 250);
      }
      else
      {
        RemoveBlinkLamp(lamps[i]);
        TurnOffLamp(lamps[i]);
      }
    }
  }
};

// APC_Bumpers
// Class to materialize bumpers
//   byte nbBumpers
//   const byte *switches
//   const byte *lamps
//   const byte *solenoids
class APC_Bumpers
{
  private:
    bool enabled;
    byte nbBumpers;
    byte switches[MAX_NB_BUMPERS];
    byte lamps[MAX_NB_BUMPERS];
    byte solenoids[MAX_NB_BUMPERS];
    bool isLit[MAX_NB_BUMPERS];
    byte initialLightMode;
    byte lightMode; // 0=off, 1=half on, 2=all on 
    bool bumpCycle; // alternates true/false at each bump
    void initializeLights()
    {
      for ( int i = 0 ; i < nbBumpers ; i++ )
      {
        if (lightMode == 2) isLit[i] = true;
        else if (lightMode == 1 && i%2 == 0)  isLit[i] = true;
        else isLit[i] = false;
      }
      UpdateLights();
    }
  public:
  APC_Bumpers(byte nbBumpers, const byte *switches, const byte *lamps, const byte *solenoids, byte initialLightMode)
  {
    enabled = false;
    this->nbBumpers = nbBumpers;
    this->initialLightMode = initialLightMode;
    for (int i = 0; i < nbBumpers; i++) { this->switches[i] = switches[i]; this->solenoids[i] = solenoids[i]; this->lamps[i] = lamps[i]; isLit[i] = false;}
    Reset();
  }
  void LightMode(byte wantedLightMode) // 0=off, 1=half on, 2=all on
  {
    if (wantedLightMode < 2) lightMode = wantedLightMode; 
    initializeLights();
  }
  void UpgradeLightMode()
  {
    if (lightMode < 2) lightMode++;
    initializeLights();
  }
  bool Bump(byte bumperIndex) // returns true when lit and false when turned off
  {
    bool wasLit = isLit[bumperIndex];
    if (enabled)
    {
      ActivateSolenoid(0, solenoids[bumperIndex]);
      if (lightMode == 1) // alternate lights
      {
        for ( int i = 0 ; i < nbBumpers ; i++ ) isLit[i] = !isLit[i];
        UpdateLights();
      }
    }
    return wasLit;
  }
  byte SwitchNumberToBumperIndex(byte SwitchNumber)
  {
    for ( int i = 0 ; i < nbBumpers ; i++ ) if (switches[i] == SwitchNumber) return i;
    return 0;
  }
  void Reset()
  {
    lightMode = initialLightMode;
    initializeLights();
  }
  void Enable(bool enable)
  {
    enabled = enable;
  }
  void UpdateLights()
  {
    if (enabled)
    {
      for ( int i = 0 ; i < nbBumpers ; i++ ) if (isLit[i]) TurnOnLamp(lamps[i]); else TurnOffLamp(lamps[i]);
    }
  }
};

// APC_Slings
// Class to materialize slings (very similar to bumper, but no lamp)
//   byte nbSlings
//   const byte *switches
//   const byte *solenoids
class APC_Slings
{
  private:
    bool enabled;
    byte nbSlings;
    byte switches[MAX_NB_SLINGS];
    byte solenoids[MAX_NB_SLINGS];
  public:
  APC_Slings(byte nbSlings, const byte *switches, const byte *solenoids)
  {
    enabled = false;
    this->nbSlings = nbSlings;
    for (int i = 0; i < nbSlings; i++) { this->switches[i] = switches[i]; this->solenoids[i] = solenoids[i];}    
  }
  void Sling(byte slingIndex)
  {
    if (enabled)
    {
      ActivateSolenoid(0, solenoids[slingIndex]);
    }
  }
  byte SwitchNumberToSlingIndex(byte SwitchNumber)
  {
    for ( int i = 0 ; i < nbSlings ; i++ ) if (switches[i] == SwitchNumber) return i;
    return 0;
  }
  void Enable(bool enable)
  {
    enabled = enable;
  }
};

// APC_Flippers
// Class to materialize flippers
//   byte nbFlippers
//   const byte *solenoids
class APC_Flippers
{
  private:
    byte nbFlippers;
    byte solenoids[MAX_NB_FLIPPERS];
  public:
  APC_Flippers(byte nbFlippers, const byte *solenoids)
  {
    this->nbFlippers = nbFlippers;
    for (int i = 0; i < nbFlippers; i++) { this->solenoids[i] = solenoids[i];}
  }
  void Enable(bool enable)
  {
    for (int i = 0; i < nbFlippers; i++)
    {
      if (enable) ActivateSolenoid(0, solenoids[i]); else ReleaseSolenoid(solenoids[i]);
    }
  }
};

// APC_Standups
// Class to materialize stand up targets
//   byte nbTargets
//   const byte *switches
//   const byte *lamps
//   bool blinking: should targets non hit yet blink ?
class APC_Standups
{
  private:
    String name;
    byte nbTargets;
    byte switches[MAX_NB_STANDUP_TARGETS_PER_BANK];
    byte lamps[MAX_NB_STANDUP_TARGETS_PER_BANK];
    bool blinking;
    bool enabled;
    bool hit[MAX_NB_STANDUP_TARGETS_PER_BANK];
  public:
    APC_Standups(String name, byte nbTargets, const byte *switches, const byte *lamps, bool blinking)
    {
      this->name = name;
      this->nbTargets = nbTargets;
      for (int i = 0; i < nbTargets; i++) { this->switches[i] = switches[i]; this->lamps[i] = lamps[i]; }
      this->blinking = blinking;
      enabled = false;
    }
    void Enable(bool enable)
    {
      debug.write(name, "enabling stand ups");
      enabled = enable;
      Reset();
    }
    bool Hit(byte standUpIndex) // returns true if target was not hit before
    {
      bool ret = hit[standUpIndex];
      hit[standUpIndex] = enabled;
      UpdateLamps();
      return !ret;
    }
    bool AllHit()
    {
      bool allHit = true;
      for ( int i = 0 ; i < nbTargets ; i++ ) if (hit[i] == false) allHit = false;
      return (allHit && enabled);
    }
    byte SwitchNumberToStandUpIndex(byte switchNumber)
    {
      for (int i = 0; i < nbTargets; i++) if (switches[i] == switchNumber) return i;
      return 0;
    }
    void UpdateLamps()
    {
      debug.write(name, "updating lamps stand ups");
      for (int i = 0; i < nbTargets; i++)
      {
        if (hit[i] && enabled) { RemoveBlinkLamp(lamps[i]); TurnOnLamp(lamps[i]); }
        else if (blinking && enabled) { TurnOnLamp(lamps[i]); AddBlinkLamp(lamps[i], 250); debug.write(name, "set blink", i);}
        else { RemoveBlinkLamp(lamps[i]); TurnOffLamp(lamps[i]); debug.write(name, "remove blink", i);}
      }
    }
    void Reset()
    {
      debug.write(name, "resetting stand ups");
      for (int i = 0; i < nbTargets; i++) hit[i] = false;
      UpdateLamps();
    }
};

// APC_Displays
// Class to materialize 7 segment displays of system 6
class APC_Sys6Displays
{
  private:
    char num_char[10]; // buffer string
    const byte DISPLAY_SIZE_PLAYER = 6;
    const byte DISPLAY_SIZE_CREDIT = 4;
    void updateDisplay(byte displayIndex, int numberToDisplay)
    {
      if (displayIndex == 0) // credit
      {      
        sprintf(num_char, "%4d", numberToDisplay);
      }
      else
      {
        sprintf(num_char, "%6d", numberToDisplay);
      }
      WritePlayerDisplay((char*)num_char, displayIndex);
    }
  public:
  void UpdatePlayerScore(byte player, int number)
  {
    if (player >= 1 && player <= 4)
    {
      updateDisplay(player, number);
    }
  }
  void resetScoreDisplays()
  {
    for (int i = 1; i <= 4; i++) UpdatePlayerScore(i, 0);
  }
  void UpdateCredit(int number)
  {
    updateDisplay(0, number);
  }
};

// APC_LampBank
// Class to materialize banks of lamps (series of lamps on playfield)
//   byte nbLamps
//   const byte *lamps
class APC_LampBank
{
  private:
    byte lamps[MAX_NB_LAMPS_IN_BANKS];
    bool lampStatus[MAX_NB_LAMPS_IN_BANKS];
    byte nbLamps;
    bool enabled;
  public:
    APC_LampBank(byte nbLamps, const byte *lamps)
    {
      enabled = false;
      for (int i = 0; i < nbLamps; i++) this->lamps[i] = lamps[i];
      this->nbLamps = nbLamps;
      Reset();
    }
    void Enable(bool enable)
    {
      enabled = enable;
    }
    void TurnOneOn()
    {
      for (int i = 0; i < nbLamps; i++) if (lampStatus[i] == false) { lampStatus[i] = true; break; }
      UpdateLights();
    }
    void TurnOneOff()
    {
      for (int i = nbLamps - 1; i >= 0; i--) if (lampStatus[i] == true) { lampStatus[i] = false; break; }
      UpdateLights();
    }    
    void TurnSomeLampsOn(byte nbOn) // turn on the nbOn first lamps
    {
      for (int i = 0; i < nbLamps; i++) lampStatus[i] = (i < nbOn);
      UpdateLights();
    }
    void Reset()
    {
      for (int i = 0; i < nbLamps; i++) lampStatus[i] = false;
      UpdateLights();
    }
    void UpdateLights()
    {
      if (enabled)
      {
        for (int i = 0; i < nbLamps; i++) if (lampStatus[i]) TurnOnLamp(lamps[i]); else TurnOffLamp(lamps[i]);
      }
    }
};
