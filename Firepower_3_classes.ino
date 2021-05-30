// Firepower classes

// Fire lanes "F" "I" "R" "E"
class FP_FireLanes
{
  private:
    bool laneStatus[FP_NB_FIRE_LANES];
  public:
    FP_FireLanes()
    {
      ResetLanes();
    }
    void ResetLanes()
    {
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ ) laneStatus[i] = false;
      UpdateLights();
    }
    void RollOverActivated( byte switchNumber )
    {     
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ )
        if (FP_SW_FIRE_ROLLOVERS[i] == switchNumber) laneStatus[i] = true;
      UpdateLights();
    }
    bool AllLanesOn()
    {
      bool allLanesOn = true;
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ )
        if (laneStatus[i] == false) allLanesOn = false;
      return allLanesOn;
    }
    void LaneChange() // shift all values to the right
    {
      bool last = laneStatus[FP_NB_FIRE_LANES - 1];
      for ( int i = FP_NB_FIRE_LANES - 1 ; i > 0  ; i-- ) laneStatus[i] = laneStatus[i-1];
      laneStatus[0] = last;
      UpdateLights();
    }
    void UpdateLights()
    {
      for ( int i = 0 ; i < FP_NB_FIRE_LANES ; i++ )
        if (laneStatus[i]) TurnOnLamp(FP_LP_FIRE_LANES[i]);
        else TurnOffLamp(FP_LP_FIRE_LANES[i]);
    }
};

class FP_Through
{
  public:
  FP_Through()
  {
  }
  byte NbBallsIn()
  {
    byte nb = 0;
    for ( int i = 0 ; i < FP_INSTALLED_BALLS ; i++ )
      if (QuerySwitch(FP_SW_THROUGH[i])) nb++;
    return nb;
  }
  void ReleaseOneBall()
  {
    // release a ball in the plunger lane if it's not occupied already
    if (!QuerySwitch(FP_SW_PLUNGER_LANE)) ActivateSolenoid(0,FP_SOL_SHOOTERLANEFEEDER);
  }
  bool OutHoleBlocked()
  {
    return QuerySwitch(FP_SW_OUTHOLE);
  }
  void ClearOutHole()
  {
    if (OutHoleBlocked()) ActivateSolenoid(0,FP_SOL_OUTHOLEKICKER);
  }
};

class FP_Eject_Holes
{
  private:
    bool holeReady[FP_NB_EJECT_HOLES];
    bool holeFull[FP_NB_EJECT_HOLES];
  public:
  FP_Eject_Holes()
  {
  }
  void ResetHoles()
  {
    for ( int i = 0 ; i < FP_NB_EJECT_HOLES ; i++ )
    {
      holeReady[i] = false;
      holeFull[i] = false;
    }
    UpdateLights();
  }
  byte NbBallsLocked()
  {
    byte nb = 0;
    for ( int i = 0 ; i < FP_NB_EJECT_HOLES ; i++ )
      if (QuerySwitch(FP_SW_EJECT_HOLES[i])) nb++;
    return nb;
  }
  void ReleaseLock(byte index)
  {
    if (QuerySwitch(FP_SW_EJECT_HOLES[index])) ActivateSolenoid(0, FP_SOL_EJECT_HOLES[index]);
  }
  void ReleaseAll()
  {
    for ( int i = 0 ; i < FP_NB_EJECT_HOLES ; i++ ) ReleaseLock(i);
  }
  void HoleIsReady(byte index)
  {
    holeReady[index] = true;
    UpdateLights();
  }
  bool BallInHole(byte switchIndex)
  {
    byte index = SwithIndexToHoleIndex(switchIndex);
    if (holeReady[index])
    {
      // accept ball
      holeFull[index] = true;
      UpdateLights();
    }
    else
    {
      // reject ball
      ActivateTimer(500, FP_SOL_EJECT_HOLES[index], FP_ReleaseLock);
    }
    return holeFull[index]; // return true if ball was kept
  }
  byte SwithIndexToHoleIndex(byte SwitchIndex)
  {
    for ( int i = 0 ; i < FP_NB_EJECT_HOLES ; i++ )
      if (FP_SW_EJECT_HOLES[i] == SwitchIndex) return i;
    return 0;
  }
  void UpdateLights()
  {
    
  }
};

FP_FireLanes fireLanes;
FP_Through through;
FP_Eject_Holes ejectHoles;


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
